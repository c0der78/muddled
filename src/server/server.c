
/******************************************************************************
 *                                       _     _ _          _                 *
 *                   _ __ ___  _   _  __| | __| | | ___  __| |                *
 *                  | '_ ` _ \| | | |/ _` |/ _` | |/ _ \/ _` |                *
 *                  | | | | | | |_| | (_| | (_| | |  __/ (_| |                *
 *                  |_| |_| |_|\__,_|\__,_|\__,_|_|\___|\__,_|                *
 *                                                                            *
 *         (C) 2010 by Ryan Jennings <c0der78@gmail.com> www.arg3.com         *
 *                 Many thanks to creators of muds before me.                 *
 *                                                                            *
 *        In order to use any part of this Mud, you must comply with the      *
 *     license in 'license.txt'.  In particular, you may not remove either    *
 *                        of these copyright notices.                         *
 *                                                                            *
 *       Much time and thought has gone into this software and you are        *
 *     benefitting.  I hope that you share your changes too.  What goes       *
 *                            around, comes around.                           *
 ******************************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include "../engine.h"
#include "../macro.h"
#include "../log.h"
#include "../macro.h"
#include "../character.h"
#include "../str.h"
#include "../player.h"
#include "../account.h"
#include "../util.h"
#include "telnet.h"
#include "config.h"
#include "client.h"
#include "update.h"
#include "websocket.h"

short server_port = 4000;
int server_socket = -1;
bool server_rebooting = false;
const char *exec_path = 0;
const char *reboot_file = 0;

time_t startup_time = 0;
time_t last_reboot = 0;

extern char *mktemp(char *);

void initialize_server()
{
    if (!server_rebooting)
    {
        static struct sockaddr_in sa_zero;
        struct sockaddr_in sa;
        int x = 1;

        if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            log_error("init_socket: socket");
            exit(-1);
        }
        if (setsockopt
                (server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&x,
                 sizeof(x)) < 0)
        {
            log_error("Init_socket: SO_REUSEADDR");
            close(server_socket);
            exit(-1);
        }
#if defined SO_DONTLINGER && !defined SYSV
        {
            struct linger ld;

            ld.l_onoff = 1;
            ld.l_linger = 1000;

            if (setsockopt
                    (server_socket, SOL_SOCKET, SO_DONTLINGER,
                     (char *)&ld, sizeof(ld)) < 0)
            {
                log_error("init_socket: SO_DONTLINGER");
                close(server_socket);
                exit(-1);
            }
        }
#endif

        sa = sa_zero;
        sa.sin_family = AF_INET;
        sa.sin_port = htons(server_port);
        sa.sin_addr.s_addr = INADDR_ANY;

        if (bind(server_socket, (struct sockaddr *)&sa, sizeof(sa)) < 0)
        {
            log_error("init socket: bind");
            close(server_socket);
            exit(-1);
        }
        if (listen(server_socket, 5) < 0)
        {
            log_error("init socket: listen");
            close(server_socket);
            exit(-1);
        }

    }
    else
    {
        char buf[ARG_SIZ];

        FILE *fp = engine_open_file(reboot_file, "r");

        last_reboot = time(0);

        if (fp == 0)
        {
            log_error("Could not open file %s for reading",
                      reboot_file);
        }
        else
        {
            log_trace("parsing reboot file");
            if (fgets(buf, sizeof(buf), fp) != NULL)
            {
                if (sscanf(buf, "%ld\n", &startup_time) != 1)
                {
                    log_error
                    ("reboot: could not get startup time");
                }
            }
            socklen_t size = sizeof(struct sockaddr);
            while (fgets(buf, sizeof(buf), fp) != NULL)
            {
                Client *conn = new_client();
                char name[100];
                char account[100];
                char term[100];
                int width, height;
                struct hostent *from;

                conn->account = new_account((Connection *) conn);

                if (sscanf (buf, "%d %d %d %[^'~']~ %[^'~']~ %s\n",
                            &conn->socket, &width, &height, account,
                            name, term) != 6)
                {
                    log_error("could not scan line for reboot");
                    destroy_client(conn);
                    continue;
                }
                conn->termType = str_dup(term);
                conn->scrWidth = width;
                conn->scrHeight = height;

                if (getpeername(conn->socket, (struct sockaddr *)&conn->addr, &size) < 0)
                {
                    log_error("getpeername");
                    conn->host = str_dup("(unknown)");
                }
                else
                {
                    from = gethostbyaddr((char *)&conn->addr.sin_addr, sizeof(conn->addr.sin_addr), AF_INET);
                    conn->host = str_dup(from ? from->h_name : getip(conn));
                    log_info("new client: %s", conn->host);
                }

                if (!load_account(conn->account, account))
                {
                    log_error("could not reload account %s", account);
                    destroy_client(conn);
                    continue;
                }
                for (AccountPlayer *ch = conn->account->players; ch; ch = ch->next)
                {
                    if (!str_cmp(ch->name, name))
                    {
                        log_info("reloaded %s", name);
                        conn->account->playing = load_player_by_id((Connection *) conn, ch->charId);
                        break;
                    }
                }

                if (conn->account->playing == 0)
                {
                    log_error("could not reload player %s", name);
                    destroy_client(conn);
                }
                else
                {
                    LINK(first_client, conn, next);
                    test_telopts(conn);

                    writeln(conn, "Reality warps briefly and you feel a change in the world.");

                    set_playing(conn);
                }

            }

            unlink(reboot_file);
        }
    }

    log_info("listening on port %d", server_port);

    websocket_context = create_websocket(websocket_port);
}

void game_loop(int control)
{
    static struct timeval null_time;
    struct timeval last_time;

    gettimeofday(&last_time, NULL);
    current_time = (time_t) last_time.tv_sec;

    /*
     * Main loop
     */
    while (server_socket != -1)
    {
        fd_set in_set;
        fd_set out_set;
        fd_set exc_set;
        Client *c, *c_next;
        int maxdesc;

        /*
         * Poll all active descriptors.
         */
        FD_ZERO(&in_set);
        FD_ZERO(&out_set);
        FD_ZERO(&exc_set);
        FD_SET(control, &in_set);
        maxdesc = control;
        for (c = first_client; c; c = c->next)
        {
            maxdesc = UMAX(maxdesc, c->socket);
            FD_SET(c->socket, &in_set);
            FD_SET(c->socket, &out_set);
            FD_SET(c->socket, &exc_set);
        }

        if (select(maxdesc + 1, &in_set, &out_set, &exc_set, &null_time) < 0)
        {
            log_error("select: poll");
            exit(1);
        }
        /*
         * New connection?
         */
        if (FD_ISSET(control, &in_set))
        {
            initialize_client(control);
        }
        /*
         * Kick out the freaky folks.
         */
        for (c = first_client; c; c = c_next)
        {
            c_next = c->next;
            if (FD_ISSET(c->socket, &exc_set) || c->handler == 0)
            {
                FD_CLR(c->socket, &in_set);
                FD_CLR(c->socket, &out_set);
                if (client_is_playing(c))
                    save_player(c->account->playing);
                c->outtop = 0;
                close_client(c);
            }
        }

        /*
         * Process input.
         */
        for (c = first_client; c; c = c_next)
        {
            c_next = c->next;

            c->fCommand = false;

            if (FD_ISSET(c->socket, &in_set))
            {
                if (!c->readln(c))
                {
                    FD_CLR(c->socket, &out_set);
                    if (client_is_playing(c))
                        save_player(c->account->playing);
                    c->outtop = 0;
                    close_client(c);
                    continue;
                }
            }
            /*
             * if (d->character != NULL && d->character->daze > 0)
             * --d->character->daze;
             *
             * if ( d->character != NULL && d->character->wait > 0 ) {
             * --d->character->wait; continue; }
             */

            if (parse_input_buffer(c) && c->handler)
            {
                c->fCommand = true;
                // stop_idling(c->character);

                (*c->handler) (c, c->incomm);

                c->incomm[0] = 0;
            }
        }

        /*
         * Autonomous game motion.
         */
        update_handler();

        libwebsocket_service(websocket_context, 1);

        /*
         * Output.
         */
        for (c = first_client; c; c = c_next)
        {
            c_next = c->next;

            if ((c->fCommand || c->outtop > 0) && FD_ISSET(c->socket, &out_set))
            {
                if (!process_output(c, true))
                {
                    if (client_is_playing(c))
                        save_player(c->account->playing);
                    c->outtop = 0;
                    close_client(c);
                }
            }
        }

        /*
         * Synchronize to a clock.
         * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
         * Careful here of signed versus unsigned arithmetic.
         */
        {
            struct timeval now_time;
            suseconds_t secDelta;
            suseconds_t usecDelta;

            gettimeofday(&now_time, NULL);
            usecDelta =
                last_time.tv_usec - now_time.tv_usec +
                1000000 / PULSE_PER_SECOND;
            secDelta =
                (suseconds_t) (last_time.tv_sec - now_time.tv_sec);

            while (usecDelta < 0)
            {
                usecDelta += 1000000;
                secDelta -= 1;
            }

            while (usecDelta >= 1000000)
            {
                usecDelta -= 1000000;
                secDelta += 1;
            }

            if (secDelta > 0 || (secDelta == 0 && usecDelta > 0))
            {
                struct timeval stall_time;

                stall_time.tv_usec = usecDelta;
                stall_time.tv_sec = secDelta;
                if (select(0, NULL, NULL, NULL, &stall_time) <
                        0)
                {
                    perror("Game_loop: select: stall");
                    exit(1);
                }
            }
        }

        gettimeofday(&last_time, NULL);
        current_time = (time_t) last_time.tv_sec;
    }
}

void run_server()
{
    initialize_server();

    startup_time = time(0);

    game_loop(server_socket);
}

void stop_server()
{
    if (server_socket != -1)
    {
        log_info("shutting down server");
        close(server_socket);
        server_socket = -1;
    }

    char buf[ARG_SIZ + 1] = {0};

    sprintf(buf, "update engine set last_shutdown = '%s'", iso8601_format(current_time, buf, ARG_SIZ));

    sql_exec(buf);
}

void reboot_server()
{
    FILE *fp;

    char arg1[50];

    char arg2[50];

    char arg3[100];

    reboot_file = mktemp("reboot.XXXXXX");

    fp = engine_open_file(reboot_file, "w");

    if (fp == 0)
    {
        log_error("Could not open file %s", reboot_file);
        return;
    }
    fprintf(fp, "%ld\n", startup_time);

    for (Client *conn = first_client; conn; conn = conn->next)
    {

        if (client_is_playing(conn))
            save_player(conn->account->playing);

        fprintf(fp, "%d %d %d %s~ %s~ %s\n", conn->socket,
                conn->scrWidth, conn->scrHeight, conn->account->login,
                conn->account->playing->name, conn->termType);

        if (conn->outtop > 0)
            process_output(conn, false);

    }

    fclose(fp);

    sprintf(arg1, "-c%d", server_socket);

    sprintf(arg2, "-p%d", server_port);

    sprintf(arg3, "-f%s", reboot_file);

    execl(exec_path, exec_path, arg1, arg2, arg3, (char *)NULL);

    log_error("Could not reboot");
}
