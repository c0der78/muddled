
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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <muddyengine/log.h>
#include <muddyengine/engine.h>
#include <muddyengine/flag.h>
#include <muddyengine/util.h>
#include <errno.h>
#include "config.h"

const Lookup logging_flags[] =
{
    {"info", LOG_INFO},
    {"error", LOG_ERR},
    {"debug", LOG_DEBUG},
    {"warning", LOG_WARN},
    {"data", LOG_DATA},
    {"trace", LOG_TRACE},
    {"bug", LOG_BUG},
    {"file", LOG_FILE},
    {0, 0}
};

void log_fun(const char *func, int level, const char *fmt, ...)
{

    char buf[ARG_SIZ];
    char date[BUF_SIZ];
    char log[BUF_SIZ];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);

    strcpy(date, str_time(-1, -1, "[%D %T]"));

    switch (level)
    {
    case LOG_INFO:
        sprintf(log, "%s INFO: %s\n", date, buf);
        break;
    case LOG_ERR:
        if (errno != 0)
        {
            sprintf(log, "%s ERROR [%s]: %s (%s)\n", date, func,
                    buf, strerror(errno));
        }
        else
        {
            sprintf(log, "%s ERROR [%s]: %s\n", date, func, buf);
        }
        break;
    case LOG_DEBUG:
        sprintf(log, "%s DEBUG [%s]: %s\n", date, func, buf);
        break;
    case LOG_WARN:
        sprintf(log, "%s WARN [%s]: %s\n", date, func, buf);
        break;
    case LOG_DATA:
        if (sql_errcode() != SQL_OK)
        {
            sprintf(log, "%s SQL [%s]: %s (%s)\n", date, func, buf,
                    sql_errmsg());
        }
        else
        {
            sprintf(log, "%s SQL [%s]: %s\n", date, func, buf);
        }
        break;
    case LOG_TRACE:
        sprintf(log, "%s TRACE [%s]: %s\n", date, func, buf);
        break;
    case LOG_BUG:
        sprintf(log, "%s BUG [%s]: %s\n", date, func, buf);
        break;
    }

    printf("%s", log);

    if (!engine_info.logging || !is_set(engine_info.logging, level))
        return;

    if (is_set(engine_info.logging, LOG_FILE))
    {
        char filename[ARG_SIZ];

        filename[0] = '\0';

        if (current_time == 0)
            current_time = time(0);

        strftime(filename, ARG_SIZ, LOG_DIR "/%d_%m_%Y.log",
                 gmtime(&current_time));

        FILE *fp = fopen(filename, "a+");

        if (fp != 0)
        {

            fputs(log, fp);

            fclose(fp);
        }
    }
}
