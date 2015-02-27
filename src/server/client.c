
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

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>
#include "../engine.h"
#include "../class.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <fcntl.h>
#ifdef HAVE_LIBCRYPT
#include <crypt.h>
#endif
#include <errno.h>
#include "../connection.h"
#include "../log.h"
#include "../character.h"
#include "../string.h"
#include "../nonplayer.h"
#include "command.h"
#include "../player.h"
#include "telnet.h"
#include "../room.h"
#include "../account.h"
#include "../color.h"
#include "../race.h"
#include "../macro.h"
#include "../util.h"
#include "../lookup.h"
#include "../help.h"
#include "../channel.h"
#include "client.h"
#include "olc.h"
#include "server.h"
#include "websocket.h"

#ifndef FNDELAY
#define FNDELAY O_NONBLOCK
#endif

void show_string(Client *, const char *);

Client *first_client = 0;

bool client_has_color(const Client *conn)
{
    if (conn->account)
    {
        return !is_set(conn->account->flags, PLR_COLOR_OFF);
    }
    else
    {
        return !nullstr(conn->termType);
    }
}

static void write_to_client(Client *conn, const char *arg)
{

    size_t length = strlen(arg);

    /*
     * Initial \n\r if needed.
     */
    if (conn->outtop == 0 && !conn->fCommand)
    {

        conn->outbuf[0] = '\n';

        conn->outbuf[1] = '\r';

        conn->outtop = 2;

    }
    /*
     * Expand the buffer as needed.
     */
    while (conn->outtop + length >= conn->outsize)
    {

        char *outbuf;

        if (conn->outsize >= 32000)
        {

            log_warn("Buffer overflow. Closing %s.", getip(conn));

            close_client(conn);

            return;

        }
        outbuf = (char *)alloc_mem(2, conn->outsize);

        strncpy(outbuf, conn->outbuf, conn->outtop);

        free_mem(conn->outbuf);

        conn->outbuf = outbuf;

        conn->outsize *= 2;

    }
    /*
     * Copy.
     */
    strncpy(conn->outbuf + conn->outtop, arg, length);

    conn->outtop += length;

}

static void vwrite_to_client(Client *conn, const char *fmt, va_list args)
{

    char buf[OUT_SIZ * 2];

    vsnprintf(buf, sizeof(buf), fmt, args);

    write_to_client(conn, buf);

}

static void writef_line_to_client(Client *conn, const char *fmt, ...)
{

    va_list args;

    va_start(args, fmt);

    vwrite_to_client(conn, fmt, args);

    va_end(args);

    write_to_client(conn, "\n\r");

}

static void write_line_to_client(Client *conn, const char *arg)
{

    write_to_client(conn, arg);

    write_to_client(conn, "\n\r");

}

static void writef_to_client(Client *conn, const char *fmt, ...)
{

    va_list args;

    va_start(args, fmt);

    vwrite_to_client(conn, fmt, args);

    va_end(args);

}

static void page_to_client(Client *conn, const char *txt)
{

    if (txt == NULL || !*txt || conn == 0)
        return;

    if (conn->scrHeight == 0)
    {

        write(conn, txt);

        return;

    }
    conn->showstr_head = malloc(strlen(txt) + 1);

    strcpy(conn->showstr_head, txt);

    conn->showstr_point = conn->showstr_head;

    show_string(conn, "");

}

static void vtitle_to_client(Client *conn, const char *title, va_list args)
{

    writeln(conn, valign_string(ALIGN_CENTER, conn->scrWidth, "~W~!B", client_has_color(conn) ? 0 : "-", title, args));

}

static void titlef_to_client(Client *conn, const char *title, ...)
{

    va_list args;

    va_start(args, title);

    vtitle_to_client(conn, title, args);

    va_end(args);

}

static void title_to_client(Client *conn, const char *title)
{

    writeln(conn, align_string(ALIGN_CENTER, conn->scrWidth, "~W~!B", client_has_color(conn) ? 0 : "-", title));

}

static bool write_to_socket(int desc, char *txt, size_t length)
{

    size_t iStart;

    int nWrite;

    size_t nBlock;

    if (length <= 0)
        length = strlen(txt);

    for (iStart = 0; iStart < length; iStart += nWrite)
    {

        nBlock = UMIN(length - iStart, 4096);

        if ((nWrite = send(desc, txt + iStart, nBlock, 0)) < 0)
        {

            log_error("write_to_socket");

            return false;

        }
    }

    return true;

}

Client *new_client()
{

    Client *conn = (Client *) alloc_mem(1, sizeof(Client));

    memset(&conn->addr, 0, sizeof(conn->addr));

    conn->socket = -1;

    conn->websocket = NULL;

    memset(conn->lastCommand, 0, sizeof(conn->lastCommand));

    conn->handler = client_get_account_name;

    conn->writeln = write_line_to_client;

    conn->write = write_to_client;

    conn->writelnf = writef_line_to_client;

    conn->writef = writef_to_client;

    conn->readln = read_line_from_client;

    conn->vwrite = vwrite_to_client;

    conn->title = title_to_client;

    conn->titlef = titlef_to_client;

    conn->vtitle = vtitle_to_client;

    conn->page = page_to_client;

    conn->is_playing = client_is_playing;

    conn->termType = str_empty;

    conn->host = str_empty;

    conn->scrHeight = 24;

    conn->scrWidth = 80;

    conn->outsize = 2000;

    conn->outbuf = alloc_mem(conn->outsize, sizeof(char));

    return conn;

}

void destroy_client(Client *conn)
{

    free_str(conn->termType);

    free_str(conn->host);

    if (conn->socket != -1)
        close(conn->socket);

    if (conn->account != 0)
        destroy_account(conn->account);

    if (conn->showstr_head != 0)
        free_mem(conn->showstr_head);

    if (conn->outsize > 0)
        free_mem(conn->outbuf);

    free_mem(conn);

}

void initialize_client(int control)
{

    static socklen_t size = sizeof(struct sockaddr);

    struct hostent *from;

    Client *c = new_client();

    getsockname(control, (struct sockaddr *)&c->addr, &size);

    if ((c->socket =
                accept(control, (struct sockaddr *)&c->addr, &size)) < 0)
    {

        log_error("accept");

        destroy_client(c);

        return;

    }
    if (fcntl(c->socket, F_SETFL, FNDELAY) == -1)
    {

        log_error("fcntl: FNDELAY");

        return;

    }
    if (getpeername(c->socket, (struct sockaddr *)&c->addr, &size) < 0)
    {

        log_error("getpeername");

        c->host = str_dup("(unknown)");

    }
    else
    {

        from =
            gethostbyaddr((char *)&c->addr.sin_addr,
                          sizeof(c->addr.sin_addr), AF_INET);

        c->host = str_dup(from ? from->h_name : getip(c));

        log_info("new client: %s", c->host);

    }

    LINK(first_client, c, next);

    test_telopts(c);

    if (greeting == 0)
    {

        writelnf(c, "Welcome to %s!", engine_info.name);

        writeln(c, "");

    }
    else
    {

        writeln(c, greeting);

    }

    write(c, "Login: ");

}

const char *getip(const Client *conn)
{

    return inet_ntoa(conn->addr.sin_addr);

}

bool parse_input_buffer(Client *conn)
{

    int i, j, k;

    bool got_n, got_r;

    /*
     * Hold horses if pending command already.
     */
    if (conn->incomm[0] != '\0')
        return true;

    /*
     * Look for at least one new line.
     */
    for (i = 0; conn->inbuf[i] != '\n' && conn->inbuf[i] != '\r'; i++)
    {

        if (conn->inbuf[i] == IAC)
        {
            // Telnet IAC
            process_telnet(conn, conn->inbuf, i);

        }
        if (conn->inbuf[i] == '\0')
            return false;

    }

    for (i = 0, k = 0; conn->inbuf[i] != '\n' && conn->inbuf[i] != '\r';
            i++)
    {

        if (k >= sizeof(conn->inbuf) - 4)
        {

            writeln(conn, "Line too long.");

            for (; conn->inbuf[i] != 0; i++)
            {

                if (conn->inbuf[i] == '\n'
                        || conn->inbuf[i] == '\r')
                    break;

            }

            conn->inbuf[i] = '\n';

            conn->inbuf[i + 1] = 0;

            break;

        }
        if (conn->inbuf[i] == '\b' && k > 0)
            --k;

#define DISABLE_EXTENDED_ASCII_CHARS
#ifdef DISABLE_EXTENDED_ASCII_CHARS
        else
        {

            if (isascii(conn->inbuf[i]) && isprint(conn->inbuf[i]))
                conn->incomm[k++] = conn->inbuf[i];

        }

#else               /* */
        else
        {

            unsigned char c = conn->inbuf[i];

            if (c > 0x1F && c != 0x7F && c != 0xFF)
            {

                conn->incomm[k++] = conn->inbuf[i];

            }
        }

#endif
    }

    /*
     * if (k == 0) conn->incomm[k++] = ' ';
     */
     if(k < ARG_SIZ)
        conn->incomm[k] = 0;
    else
        conn->incomm[ARG_SIZ-1] = 0;

    if (k > 1 || conn->incomm[0] == '!')
    {

        if (conn->incomm[0] != '!'
                && strcasecmp(conn->incomm, conn->lastCommand))
        {

            conn->repeat = 0;

        }
        else
        {

            if (++conn->repeat == 25)
            {

                log_warn("%s input spamming!", getip(conn));

            }
            else if (conn->repeat == 35)
            {

                log_warn("%s still input spamming!",
                         getip(conn));

            }
            else if (conn->repeat >= 45)
            {

                conn->repeat = 0;

                writeln(conn, "\n\r*** I WARNED YOU!!! ***");

                strcpy(conn->incomm, "quit");

            }
        }

    }
    if (conn->incomm[0] == '!')
    {

        strcpy(conn->incomm, conn->lastCommand);

    }
    else
    {

        strcpy(conn->lastCommand, conn->incomm);

    }

    got_n = got_r = false;

    for (; conn->inbuf[i] == '\r' || conn->inbuf[i] == '\n'; i++)
    {

        if (conn->inbuf[i] == '\r' && got_r++)
            break;

        else if (conn->inbuf[i] == '\n' && got_n++)
            break;

    }

    for (j = 0; (conn->inbuf[j] = conn->inbuf[i + j]) != 0; j++) ;

    return true;

}

bool read_line_from_client(Client *conn)
{

    size_t iStart = 0;

    /*
     * Hold horses if pending command already.
     */
    if (conn->incomm[0] != 0)
        return false;

    if (iStart >= sizeof(conn->inbuf) - 10)
    {

        log_error("input overflow for %s", getip(conn));

        writeln(conn, "\n\r*** PUT A LID ON IT!!! ***\n\r");

        return false;

    }
    for (;;)
    {

        ssize_t nRead;

        if (sizeof(conn->inbuf) - 10 - iStart == 0)
            break;

        nRead =
            recv(conn->socket, (conn->inbuf + iStart),
                 (sizeof(conn->inbuf) - 10 - iStart), 0);

        if (nRead > 0)
        {

            iStart += nRead;

            if (conn->inbuf[iStart - 1] == '\n'
                    || conn->inbuf[iStart - 1] == '\r')
                break;

        }
        else if (nRead == 0)
        {

            log_warn("%s disconnected", getip(conn));

            return false;

        }
        else if (errno == EWOULDBLOCK)
        {

            break;

        }
        else
        {

            return false;

        }

    }

    conn->inbuf[iStart] = 0;

    conn->inbuf[iStart + 1] = 0;

    return true;

}

void pager_prompt(Client *conn)
{

    int shown_lines = 0;

    int total_lines = 0;

    for (const char *ptr = conn->showstr_head;
            *ptr && ptr != conn->showstr_point; ptr++)
        if (*ptr == '\n')
            shown_lines++;

    total_lines = shown_lines;

    for (const char *ptr = conn->showstr_point; *ptr; ptr++)
        if (*ptr == '\n')
            total_lines++;

    writef(conn,
           "\n\r(%d%%) (H)elp, (R)efresh page, (P)revious page, or hit ENTER: ",
           100 * shown_lines / total_lines);

}

void bust_a_prompt(Client *conn)
{

    char buf[OUT_SIZ] = { 0 };

    char buf2[BUF_SIZ] = { 0 };

    char *pbuf = buf;

    char *tmp;

    Character *ch;

    const char *pstr;

    if (!conn->account || !conn->account->playing
            || !conn->account->playing->pc)
    {

        write(conn, "> ");

        return;

    }
    ch = conn->account->playing;

    if (ch->fighting)
    {

        pstr = ch->pc->battlePrompt;

    }
    else
    {

        pstr = ch->pc->prompt;

    }

    while (*pstr)
    {

        if (*pstr != '%')
        {

            *pbuf++ = *pstr++;

            continue;

        }
        pstr++;

        switch (*pstr)
        {

        default:

            tmp = "";

            break;

        case 'E':

            if (ch->fighting == 0)
            {

                tmp = "No Enemy";

            }
            else
            {

                sprintf(buf2, "%d%%",
                        (int)percent(ch->fighting->hit,
                                     ch->fighting->maxHit));

                tmp = buf2;

            }

            break;

        case '%':

            tmp = "%";

            break;

        case 'm':

            sprintf(buf2, "%ld", ch->mana);

            tmp = buf2;

            break;

        case 'M':

            sprintf(buf2, "%ld", ch->maxMana);

            tmp = buf2;

            break;

        case 'h':

            sprintf(buf2, "%ld", ch->hit);

            tmp = buf2;

            break;

        case 'H':

            sprintf(buf2, "%ld", ch->maxHit);

            tmp = buf2;

            break;

        case 'v':

            sprintf(buf2, "%ld", ch->move);

            tmp = buf2;

            break;

        case 'V':

            sprintf(buf2, "%ld", ch->maxMove);

            tmp = buf2;

            break;

        case 'n':

            tmp = "\n\r";

            break;

        }

        while ((*pbuf = *tmp) != 0)
            pbuf++, tmp++;

        pstr++;

    }

    *pbuf++ = ' ';

    write(conn, buf);

}

bool parse_color_codes(Client *conn)
{

    color_t color;

    bool colorOn = client_has_color(conn);

    register char *result;

    register const char *pstr, *tmp;

    char output[OUT_SIZ];

    bool success = true;

    bool written = false;

    bool need_to_finish_color = false;

    result = output;

    for (pstr = conn->outbuf;
            *pstr && (pstr - conn->outbuf) < conn->outtop; pstr++)
    {

        if ((result - output) >= OUT_SIZ - 32)
        {

            *result++ = 0;

            if (conn->websocket)
                written = write_to_websocket(conn->websocket, output, result - output);
            else
                written =  write_to_socket(conn->socket, output, result - output);

            if (!(success = written))
                break;  /* problems... */

            memset(output, 0, sizeof(output));

            result = output;    /* increment counter */

        }
        if (*pstr != COLOR_CODE)
        {

            *result++ = *pstr;

            continue;

        }
        pstr = convert_color_code(pstr, colorOn ? &color : 0);

        if (*pstr == COLOR_CODE)
        {

            *result++ = COLOR_CODE;

        }
        else if (colorOn)
        {

            if (conn->websocket)
            {
                if (need_to_finish_color)
                {
                    tmp = finish_html_color();

                    while (*tmp && (result - output) < OUT_SIZ)
                    {
                        *result++ = *tmp++;
                    }
                }
                tmp = make_html_color(&color);

                need_to_finish_color = true;
            }
            else
            {
                tmp = make_terminal_color(&color);
            }

            while (*tmp != 0 && (result - output) < OUT_SIZ)
            {
                *result++ = *tmp++;
            }
        }
    }

    if (conn->websocket && need_to_finish_color)
    {
        tmp = finish_html_color();

        while (*tmp && (result - output) < OUT_SIZ)
        {
            *result++ = *tmp++;
        }
    }
    *result = 0;

    if (conn->websocket)
        written = write_to_websocket(conn->websocket, output, result - output);
    else
        written = write_to_socket(conn->socket, output, result - output);

    success = (success && written);

    conn->outtop = 0;

    return success;

}

bool process_output(Client *conn, bool fPrompt)
{

    /*
     * Bust a prompt.
     * OLC changed
     */
    if (!is_set(engine_info.flags, ENGINE_OFF))
    {

        if (conn->showstr_point)
            pager_prompt(conn);

        else if (fPrompt && conn->editing)
            olc_prompt(conn);

        else if (fPrompt && client_is_playing(conn))
        {

            Character *ch;

            Character *victim;

            ch = conn->account->playing;

            /*
             * battle prompt
             */
            if ((victim = ch->fighting) != NULL
                    && can_see(ch, victim))
            {

                int percent;

                char wound[100];

                if (victim->maxHit > 0)
                    percent =
                        (int)(victim->hit * 100 /
                              victim->maxHit);

                else
                    percent = -1;

                if (percent >= 100)
                    strcpy(wound,
                           "is in excellent condition.");

                else if (percent >= 90)
                    strcpy(wound, "has a few scratches.");

                else if (percent >= 75)
                    strcpy(wound,
                           "has some small wounds and bruises.");

                else if (percent >= 50)
                    strcpy(wound,
                           "has quite a few wounds.");

                else if (percent >= 30)
                    strcpy(wound,
                           "has some big nasty wounds and scratches.");

                else if (percent >= 15)
                    strcpy(wound, "looks pretty hurt.");

                else if (percent >= 0)
                    strcpy(wound, "is in awful condition.");

                else
                    strcpy(wound, "is bleeding to death.");

                writelnf(conn, "~R%s %s~x", NAME(victim),
                         wound);

            }
            writeln(conn, "");

            bust_a_prompt(conn);

        }
    }
    /*
     * Short-circuit if nothing to write.
     */
    if (conn->outtop == 0)
        return true;

    return parse_color_codes(conn);

}

void close_client(Client *conn)
{

    if (conn->outtop > 0)
        process_output(conn, false);

    UNLINK(first_client, Client, conn, next);

    destroy_client(conn);

}

void show_string(Client *conn, const char *input)
{

    char buffer[2 * OUT_SIZ];

    char buf[ARG_SIZ];

    register char *scan;

    register const char *chk;

    int lines = 0, toggle = 1;

    int show_lines;

    one_argument(input, buf);

    switch (UPPER(buf[0]))
    {

    case '\0':

    case 'C':

        lines = 0;

        break;

    case 'R':       /* refresh current page of text */

        lines = -1 - (conn->scrHeight - 2);

        break;

    case 'B':       /* scroll back a page of text */

        lines = -1 - (2 * (conn->scrHeight - 2));

        break;

    case '?':

    case 'H':       /* Show some help */

        writeln(conn, "Pager help:");

        writeln(conn, "C or Enter     next page");

        writeln(conn, "R              refresh this page");

        writeln(conn, "B              previous page");

        writeln(conn, "H or ?         help");

        writeln(conn, "Any other keys exit.");

        return;

    default:        /* otherwise, stop the text viewing */

        if (conn->showstr_head)
        {

            free_mem(conn->showstr_head);

            conn->showstr_head = 0;

        }
        conn->showstr_point = 0;

        return;

    }

    /*
     * do any backing up necessary
     */
    if (lines < 0)
    {

        for (chk = conn->showstr_point; chk != conn->showstr_head;
                chk--)
        {

            if ((*chk == '\n') || (*chk == '\r'))
            {

                toggle = -toggle;

                if (toggle < 0)
                    if (!(++lines))
                        break;

            }
        }

        conn->showstr_point = chk;

    }
    show_lines = conn->scrHeight - 2;

    lines = 0;

    toggle = 1;

    for (scan = buffer;; scan++, conn->showstr_point++)
    {

        if (((*scan = *conn->showstr_point) == '\n'
                || *scan == '\r') && (toggle = -toggle) < 0)
            lines++;

        else if (!*scan || (show_lines > 0 && lines >= show_lines))
        {

            *scan = '\0';

            write(conn, buffer);

            for (chk = conn->showstr_point; isspace((int)*chk); chk++)
            {

            }



            if (!*chk)
            {

                if (conn->showstr_head)
                {

                    free_mem(conn->showstr_head);

                    conn->showstr_head = 0;

                }
                conn->showstr_point = 0;

            }


            return;

        }
    }

    return;

}

void client_wait_to_quit(Client *conn, const char *argument)
{

    conn->handler = 0;

}

void client_command_parser(Client *conn, const char *argument)
{

    if (conn->showstr_point != 0)
    {

        show_string(conn, argument);

    }
    else
    {

        if (conn->editing != 0)
        {

            conn->editing->edit(conn, argument);

        }
        else
        {

            command_interpret(conn->account->playing, argument);

        }

    }

}

void client_display_account_menu(Client *conn)
{

    int count = 0;

    clear_screen(conn);

    set_cursor(conn, 1, 1);

    conn->title(conn, "Account Menu");

    writeln(conn, "");

    writeln(conn, "~Y(C)~Create new player");

    writeln(conn, "~Y(D)~Celete account");

    writeln(conn, "~Y(E)~Cxit game\n");

    if (conn->account->players != 0)
    {

        writeln(conn, "Players:");

        for (AccountPlayer *ch = conn->account->players; ch != 0;
                ch = ch->next)
        {

            writelnf(conn, " ~Y%2d) ~W%s ~C(Lvl %d)~x", ++count,
                     ch->name, ch->level);

        }

    }
    write(conn, "\n\r~CYour selection?~x ");

}

bool client_is_playing(Client *conn)
{

    return conn && conn->account && conn->account->playing
           && (conn->handler == client_command_parser);

}

void set_playing(Client *conn)
{

    Character *ch = conn->account->playing;

    if (ch->inRoom == 0)
    {

        ch->inRoom = get_default_room();

    }
    conn->handler = client_command_parser;

    LINK(first_character, ch, next);

    LINK(first_player, ch, next_player);

    char_to_room(ch, ch->inRoom);

    if (!server_rebooting)
    {

        clear_screen(conn);

        set_cursor(conn, 1, 1);

        log_info("%s playing %s (lvl %d)", conn->account->login,
                 ch->name, ch->level);

        act(TO_ROOM, ch, 0, 0,
            "~?The nebula orion flickers and $n materializes from the fourth dimension...~x");

        writeln(conn,
                "~?You materialize from the fourth dimension...~x");

        do_look(str_empty, ch, str_empty);

    }
}

void client_display_sex_menu(Client *conn)
{

    clear_screen(conn);

    set_cursor(conn, 1, 1);

    conn->title(conn, "Available Sexes");

    int count = 0;

    for (const Lookup *t = sex_table; t->name != 0; t++)
    {

        writelnf(conn, "~Y%d) ~W%s", ++count, capitalize(t->name));

    }

    writeln(conn, "");

    write(conn, "~CWhat is your sex? ");

}

void client_confirm_delete_char(Client *conn, const char *argument)
{

    if (!argument || !*argument)
    {

        set_playing(conn);

        writeln(conn, "Delete canceled.");

        return;

    }
    if (strcmp
            (conn->account->password, crypt(argument, conn->account->login)))
    {

        set_playing(conn);

        writeln(conn, "~CPasswords don't match. Delete cancelled.");

        return;

    }
    delete_player(conn->account->playing);

    extract_char(conn->account->playing, true);

    client_display_account_menu(conn);

    conn->handler = client_account_menu;

}

void client_delete_char(Connection *conn)
{

    Client *cl = (Client *) conn;

    writelnf(conn, "~RAre you sure you want to delete '%s'?~x",
             cl->account->playing->name);

    write(conn,
          "~CEnter your account password to continue, or hit return to cancel: ~x");

    cl->handler = client_confirm_delete_char;

}

void client_display_class_menu(Client *conn)
{

    clear_screen(conn);

    set_cursor(conn, 1, 1);

    conn->title(conn, "Available Classes");

    for (int i = 0; i < max_class; i++)
    {

        writelnf(conn, "~W%12s~C: %s~x", class_table[i].name,
                 class_table[i].description);

    }
    writeln(conn, "");

    writeln(conn, "~CWhat is your class? ~x");

}

void client_get_class(Client *conn, const char *argument)
{

    if (!argument || !*argument)
    {

        write(conn,
              "~CPlease specify which class your character shall be: ~x");

        return;

    }
    int c = class_lookup(argument);

    if (c == -1)
    {

        writeln(conn, "~CThat is not a valid class.~x");

        return;

    }
    Character *ch = conn->account->playing;

    int pos = class_count(ch);

    ch->classes = (int *)realloc(ch->classes, (pos + 2) * sizeof(int));

    ch->classes[pos] = c;

    ch->classes[pos + 1] = -1;

    AccountPlayer *ap = new_account_player();

    set_playing(conn);

    ap->charId = ch->id;

    ap->name = str_dup(ch->name);

    ap->level = ch->level;

    LINK(conn->account->players, ap, next);

    announce(ch, INFO_JOIN, "$n has entered the realms.");

}

void client_display_race_menu(Client *conn)
{

    clear_screen(conn);

    set_cursor(conn, 1, 1);

    conn->title(conn, "Available Races");

    for (Race *race = first_race; race != 0; race = race->next)
    {

        writelnf(conn, "~W%12s~C: %s~x", race->name, race->description);

    }

    writeln(conn, "");

    write(conn, "~CWhat is your race? ~x");

}

void client_get_race(Client *conn, const char *argument)
{

    if (!argument || !*argument)
    {

        write(conn,
              "~CPlease specify which race your character shall be: ~x");

        return;

    }
    Race *race = race_lookup(argument);

    if (race == 0)
    {

        writeln(conn, "~CThat is not a valid race.~x");

        return;

    }
    conn->account->playing->race = race;

    memcpy(conn->account->playing->stats, race->stats,
           MAX_STAT * sizeof(int));

    client_display_class_menu(conn);

    conn->handler = client_get_class;

}

void client_get_char_sex(Client *conn, const char *argument)
{

    if (!argument || !*argument)
    {

        writeln(conn,
                "~CPlease specify which sex your character shall be:~x ");

        return;

    }
    if (is_number(argument))
    {

        int num = atoi(argument);

        int count = 0;

        const Lookup *t;

        for (t = sex_table; t->name != 0; t++)
        {

            if (++count == num)
                break;

        }

        if (t->name == 0)
        {

            writelnf(conn, "~CValid sexes are: ~W%s~x",
                     lookup_names(sex_table));

            return;

        }
        conn->account->playing->sex = t->value;

    }
    else
    {

        long sex = value_lookup(sex_table, argument);

        if (sex == -1)
        {

            writelnf(conn, "~CValid sexes are: ~W%s~x",
                     lookup_names(sex_table));

            return;

        }
        conn->account->playing->sex = (sex_t) sex;

    }

    client_display_race_menu(conn);

    conn->handler = client_get_race;

}

void newbize(Character *ch)
{

    set_bit(ch->pc->account->flags, PLR_HINTS);

    ch->level = 1;

}

void client_create_new_char(Client *conn, const char *argument)
{

    if (!argument || !*argument || strlen(argument) > 12
            || strlen(argument) < 3)
    {

        writeln(conn,
                "Please enter a name between 3 and 12 characters.");

        return;

    }
    for (const char *str = argument; *str != 0; str++)
    {

        if (!isalpha((int)*str))
        {

            writeln(conn,
                    "You can only use letters for your characters name.");

            return;

        }
    }

    conn->account->playing = new_char();

    conn->account->playing->pc = new_player((Connection *) conn);

    newbize(conn->account->playing);

    conn->account->playing->name = str_dup(capitalize(argument));

    client_display_sex_menu(conn);

    conn->handler = client_get_char_sex;

}

void client_confirm_account_delete(Client *conn, const char *argument)
{

    send_telopt(conn, WONT, TELOPT_ECHO);

    if (strcmp
            (conn->account->password, crypt(argument, conn->account->login)))
    {

        writeln(conn,
                "Incorrect password.  Account deletion cancelled.");

        conn->handler = client_account_menu;

        return;

    }
    writeln(conn, "Sorry to see you go! Safe travels!");

    delete_account(conn->account);

    destroy_account(conn->account);

    conn->handler = 0;

}

void client_account_menu(Client *conn, const char *argument)
{

    if (is_number(argument))
    {

        int count = 0, num = atoi(argument);

        AccountPlayer *ch;

        for (ch = conn->account->players; ch != 0; ch = ch->next)
        {

            if (++count == num)
            {

                break;

            }
        }

        if (ch != 0)
        {

            conn->account->playing =
                load_player_by_id((Connection *) conn, ch->charId);

            if (conn->account->playing == 0)
            {

                writeln(conn,
                        "There was a problem loading that character.");

            }
            else
            {

                writelnf(conn,
                         "The nebula orion flashes and %s materializes from the 4th dimension...",
                         ch->name);

                announce(conn->account->playing, INFO_JOIN,
                         "$n has entered the realms.");

                set_playing(conn);

                return;

            }

        }
        client_display_account_menu(conn);

        return;

    }
    switch (UPPER(argument[0]))
    {

    case 'C':

        write(conn, "~CWhat is your new characters name?~x ");

        conn->handler = client_create_new_char;

        break;

    case 'D':

        writeln(conn,
                "~CAre you sure you want to delete your account?~x");

        write(conn, "~CEnter your password to confirm:~x ");

        conn->handler = client_confirm_account_delete;

        send_telopt(conn, WILL, TELOPT_ECHO);

        break;

    case 'E':

        writeln(conn, "~GGood-bye!~x");

        conn->handler = 0;

        break;

    default:

        writeln(conn, "~CHuh? What is your selection?~x ");

        break;

    }

}

void update_login_count()
{

    engine_info.logins++;

    engine_info.total_logins++;

}

void client_display_timezones(Client *conn)
{

    writelnf(conn, "~C%-6s %-30s (%s)", "Name", "City/Zone Crosses",
             "Time~x");

    time_t current_time = time(0);

    writelnf(conn, "~C%s~x", fillstr("-", conn->scrWidth));

    for (int i = 0; timezones[i].name != 0; i++)
    {

        writelnf(conn, "%-6s %-30s (%s)", timezones[i].name,
                 timezones[i].zone, str_time(current_time, i, NULL));

    }
    writeln(conn, "");

    write(conn, "~CWhat is your timezone?~x ");

}

void client_get_timezone(Client *conn, const char *argument)
{

    int zone = timezone_lookup(argument);

    if (zone == -1)
    {

        writeln(conn, "That is not a valid timezone.");

        return;

    }
    conn->account->timezone = zone;

    client_display_account_menu(conn);

    update_login_count();

    conn->handler = client_account_menu;

}

void client_create_account_email(Client *conn, const char *argument)
{

    if (!argument || !*argument || !is_valid_email(argument))
    {

        write(conn, "That is not a valid email address. Try again: ");

        return;

    }
    conn->account->email = str_dup(argument);

    client_display_timezones(conn);

    conn->handler = client_get_timezone;

}

void client_create_account_password(Client *, const char *);

void client_confirm_account_password(Client *conn, const char *argument)
{

    if (strcmp
            (conn->account->password, crypt(argument, conn->account->login)))
    {

        writeln(conn, "Passwords don't match.  Let's try again.");

        write(conn, "Please enter your new password: ");

        conn->handler = client_create_account_password;

        return;

    }
    send_telopt(conn, WONT, TELOPT_ECHO);

    writeln(conn, "");

    write(conn, "What is your email address? ");

    conn->handler = client_create_account_email;

}

void client_create_account_password(Client *conn, const char *argument)
{

    if (!argument || !*argument || strlen(argument) < 6)
    {

        writeln(conn,
                "Your password must be at least 6 symbols or more.");

        return;

    }
    free_str_dup(&conn->account->password,
                 crypt(argument, conn->account->login));

    write(conn, "Please re-enter your password: ");

    conn->handler = client_confirm_account_password;

}

void client_create_account(Client *conn, const char *argument)
{

    switch (UPPER(argument[0]))
    {

    case 'Y':

        writelnf(conn, "Please enter a password for %s: ",
                 conn->account->login);

        conn->handler = client_create_account_password;

        send_telopt(conn, WILL, TELOPT_ECHO);

        break;

    case 'N':

        write(conn, "Ok, what is your account login then? ");

        conn->handler = client_get_account_name;

        break;

    default:

        write(conn, "Please enter 'Y' or  'N': ");

        break;

    }

}

void client_get_account_password(Client *conn, const char *argument)
{

    if (strcmp
            (conn->account->password, crypt(argument, conn->account->login)))
    {

        if (conn->password_retries >= 2)
        {

            send_telopt(conn, WONT, TELOPT_ECHO);

            write(conn,
                  "Too many password attempts.  Disconnecting.");

            conn->handler = 0;

            return;

        }
        write(conn, "Incorrect password, try again: ");

        conn->password_retries++;

        return;

    }
    send_telopt(conn, WONT, TELOPT_ECHO);

    update_login_count();

    AccountPlayer *p;

    for (p = conn->account->players; p; p = p->next)
    {

        if (p->charId == conn->account->autologinId)
            break;

    }

    if (p == 0)
    {

        client_display_account_menu(conn);

        conn->handler = client_account_menu;

    }
    else
    {

        conn->account->playing =
            load_player_by_id((Connection *) conn, p->charId);

        if (conn->account->playing == 0)
        {

            writeln(conn,
                    "There was a problem loading your character.");

        }
        else
        {

            act(TO_ROOM, conn->account->playing, 0, 0,
                "~?The nebula orion flashes and $n materializes from the fourth dimension...~x");

            writeln(conn,
                    "~?You materialize from the fourth dimension...~x");

            set_playing(conn);

            announce(conn->account->playing, INFO_JOIN,
                     "$n has entered the realms.");

        }

    }

}

void client_get_account_name(Client *conn, const char *argument)
{

    if (!argument || !*argument)
    {

        write(conn, "Please enter your account login: ");

        return;

    }
    conn->account = new_account((Connection *) conn);

    if (!load_account(conn->account, argument))
    {

        conn->account->login = str_dup(argument);

        writeln(conn, "Hmmm, we have no record of that account.");

        writef(conn, "Would you like to create one for %s? (Y/N) ",
               argument);

        conn->handler = client_create_account;

    }
    else
    {

        write(conn, "Please enter your password: ");

        conn->handler = client_get_account_password;

        send_telopt(conn, WILL, TELOPT_ECHO);

    }

}
