/******************************************************************************
 *         __  __           _     _         ____  _       _                   *
 *        |  \/  |_   _  __| | __| |_   _  |  _ \| | __ _(_)_ __  ___         *
 *        | |\/| | | | |/ _` |/ _` | | | | | |_) | |/ _` | | '_ \/ __|        *
 *        | |  | | |_| | (_| | (_| | |_| | |  __/| | (_| | | | | \__ \        *
 *        |_|  |_|\__,_|\__,_|\__,_|\__, | |_|   |_|\__,_|_|_| |_|___/        *
 *                                  |___/                                     *
 *                                                                            *
 *         (C) 2010 by Ryan Jennings <c0der78@gmail.com> www.arg3.com         *
 *	               Many thanks to creators of muds before me.                 *
 *                                                                            *
 *        In order to use any part of this Mud, you must comply with the      *
 *     license in 'license.txt'.  In particular, you may not remove either    *
 *                        of these copyright notices.                         *
 *                                                                            *
 *       Much time and thought has gone into this software and you are        *
 *     benefitting.  I hope that you share your changes too.  What goes       *
 *                            around, comes around.                           *
 ******************************************************************************/
#include <unistd.h>
#include <fcntl.h>
#include "muddyplains/server.h"
#include <sys/wait.h>
#include <muddyengine/player.h>
#include <muddyengine/macro.h>
#include <muddyengine/db.h>
#include <muddyengine/script.h>
#include <muddyengine/log.h>
#include <muddyengine/account.h>
#include <muddyengine/help.h>
#include <muddyengine/engine.h>
#include <muddyengine/hint.h>
#include <muddyengine/race.h>
#include <muddyengine/skill.h>
#include <signal.h>
#include <muddyengine/social.h>
#include <muddyengine/class.h>
#include <muddyengine/area.h>
#include <muddyengine/forum.h>
#include "muddyplains/client.h"
#include <muddyengine/channel.h>

#include <sys/time.h>

void init_signals();
void parse_options(int, char **);

static void exit_engine()
{
    save_engine();

    save_forums();
}

int main(int argc, char *argv[])
{
    srand((unsigned int) (time(NULL) ^ getpid()));

    parse_options(argc, argv);

    init_signals();

    initialize_engine(Stringify(ROOT_DIR));

    initialize_channels();

    run_server();

    return 0;
}

void parse_options(int argc, char *argv[])
{
    int c;

    log_trace("parsing options");

    exec_path = argv[0];

    while ((c = getopt(argc, argv, "c:p:i:")) != -1)
    {
        switch (c)
        {
        case 'c':
            server_socket = atoi(optarg);
            server_rebooting = true;
            break;
        case 'p':
            server_port = atoi(optarg);
            break;
        case 'i':
            server_import(Stringify(ROOT_DIR), optarg);
            exit(EXIT_SUCCESS);
        default:
            break;
        }
    }
}

void handle_sig(int sig)
{
    Character *ch;
    struct sigaction default_action;
    int i;
    pid_t forkpid;
    int status;
    static volatile sig_atomic_t crashed = 0;

    waitpid(-1, &status, WNOHANG);
    if (crashed == 0)
    {
        crashed++;

        log_info("game has crashed (%d).", sig);

        for (Client * c_next, *c = first_client; c; c = c_next)
        {
            c_next = c->next;
            ch = c->account ? c->account->playing : 0;

            if (!ch)
            {
                close_client(c);
                continue;
            }
            save_player(ch);

            writelnf(c, "\007~R%s has crashed, please hold on.~x",
                     engine_info.name);
        }

        /*success - proceed with fork / copyover plan.Otherwise will go to
           // next section and crash with a full reboot to recover */
        forkpid = fork();
        if (forkpid > 0)
        {
            /*Parent process copyover and exit */
            waitpid(forkpid, &status, WNOHANG);
            reboot_server();
            exit(0);
        }
        else if (forkpid < 0)
        {
            exit(1);
        }
        /*Child process proceed to dump
           // Close all files ! */
        for (i = 255; i >= 0; i--)
            close(i);

        /*Dup / dev / null to STD {
           IN, OUT, ERR
           } */
        open("/dev/null", O_RDWR);
        dup(0);
        dup(0);

        default_action.sa_handler = SIG_DFL;
        sigaction(sig, &default_action, NULL);

        /*I run different scripts depending on my port */
        if (!fork())
        {
            exit(0);
        }
        else
            return;
        raise(sig);
    }
    else if (crashed == 1)
    {
        crashed++;

        for (Client * c_next, *c = first_client; c; c = c_next)
        {
            c_next = c->next;
            ch = c->account ? c->account->playing : 0;
            if (ch == NULL)
            {
                close_client(c);
                continue;
            }
            writeln(c,
                    "\007\007~R** Character not saved properly. Shutting down gracefully.. **~x");
            close_client(c);
            continue;
        }
        log_info("Unable to save characters.");
        default_action.sa_handler = SIG_DFL;
        sigaction(sig, &default_action, NULL);

        if (!fork())
        {
            kill(getppid(), sig);
            exit(1);
        }
        else
            return;
        raise(sig);
    }
    else if (crashed == 2)
    {
        crashed++;
        log_info("total game crash");
        default_action.sa_handler = SIG_DFL;
        sigaction(sig, &default_action, NULL);

        if (!fork())
        {
            kill(getppid(), sig);
            exit(1);
        }
        else
            return;
        raise(sig);
    }
    else if (crashed == 3)
    {
        default_action.sa_handler = SIG_DFL;
        sigaction(sig, &default_action, NULL);

        if (!fork())
        {
            kill(getppid(), sig);
            exit(1);
        }
        else
            return;
        raise(sig);
    }
}

struct itimerval vtimer;
#define DEFAULT_VTIMER  (120)

void set_vtimer(long sec)
{
    vtimer.it_value.tv_sec = sec == -1 ? DEFAULT_VTIMER : sec;
    vtimer.it_value.tv_usec = 0;
    struct itimerval otimer;

    if (setitimer(ITIMER_REAL, &vtimer, &otimer) < 0)
    {
        perror("setitimer");
        exit(EXIT_FAILURE);
    }
}

void handle_alarm(int sig)
{
    static int safe_check = 0;
    char crash_message_a[] =
        "program has been unresponsive for the past 60 seconds.";
    char crash_message_b[] = "initiating reboot...";
    char crash_message_c[] = "failed to gracefully restart.";

    switch (safe_check)
    {
    case 0:
        safe_check = 1;
        log_bug("%s", crash_message_a);
        log_bug("%s", crash_message_b);
        break;

    case 1:
        safe_check = 2;
        log_info("%s", crash_message_a);
        log_info("%s", crash_message_b);
        log_info("%s", crash_message_c);
        break;

    case 2:
        break;
    }

    /* Reboot the MUD */

    set_vtimer(-1);

    /* Shouldnt return */
    handle_sig(sig);

    /* Last resort */
    exit(1);
}

const struct sig_type
{
    const char *name;
    int sig;
    void (*sigfun) (int);
    int flags;
} sig_table[] =
{
    {
        "SIGPIPE", SIGPIPE, SIG_IGN, 0
    }, {
        "SIGCHLD", SIGCHLD, SIG_IGN, 0
    }, {
        "SIGHUP", SIGHUP, SIG_IGN, 0
    }, {
        "SIGQUIT", SIGQUIT, handle_sig, 0
    }, {
        "SIGBUS", SIGBUS, handle_sig, SA_NODEFER
    }, {
        "SIGUSR1", SIGUSR1, handle_sig, SA_NODEFER
    }, {
        "SIGUSR2", SIGUSR2, handle_sig, SA_NODEFER
    }, {
        "SIGINT", SIGINT, exit, SA_NODEFER
    }, {
        "SIGILL", SIGILL, handle_sig, SA_NODEFER
    }, {
        "SIGFPE", SIGFPE, handle_sig, SA_NODEFER
    }, {
        "SIGSEGV", SIGSEGV, handle_sig, SA_NODEFER
    }, {
        "SIGTERM", SIGTERM, exit, SA_NODEFER
    }, {
        "SIGABRT", SIGABRT, handle_sig, SA_NODEFER
    }, {
        "SIGALRM", SIGALRM, handle_alarm, SA_NODEFER
    }, {
        NULL, -1, NULL, -1
    }
};

bool init_sig(const struct sig_type *tabl)
{
    struct sigaction sigact;

    sigact.sa_flags = tabl->flags;
    sigact.sa_handler = (void (*)(int)) tabl->sigfun;
    sigemptyset(&sigact.sa_mask);

    sigaction(tabl->sig, &sigact, NULL);

    if (tabl->sig == SIGVTALRM)
    {
        set_vtimer(-1);
    }
    return true;
}

void init_signals()
{
    for (int i = 0; sig_table[i].name != NULL; i++)
        init_sig(&sig_table[i]);

    atexit(close_lua);
    atexit(db_close);
    atexit(exit_engine);
}
