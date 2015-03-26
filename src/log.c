
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
#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "engine.h"
#include "log.h"
#include "flag.h"
#include "util.h"

extern void vsyslog(int priority, const char *message, va_list args);

void log_info(const char *format, ...)
{
    va_list args;

    va_start(args, format);

    vsyslog(LOG_INFO, format, args);

    va_end(args);
}

void log_error(const char *format, ...)
{
    va_list args;

    va_start(args, format);

    vsyslog(LOG_ERR, format, args);

    va_end(args);
}

void log_debug(const char *format, ...)
{
    va_list args;

    va_start(args, format);

    vsyslog(LOG_DEBUG, format, args);

    va_end(args);
}

void log_warn(const char *format, ...)
{
    va_list args;

    va_start(args, format);

    vsyslog(LOG_NOTICE, format, args);

    va_end(args);
}

void log_data(const char *format, ...)
{
    char buf[BUFSIZ + 1] = {0};
    va_list args;

    va_start(args, format);

    printf("DATA: ");

    if (sql_errcode() != SQL_OK)
    {
        vsnprintf(buf, BUFSIZ, format, args);

        snprintf(buf, BUFSIZ, "%s (%s)", buf,  sql_errmsg());

        syslog(LOG_ALERT, "%s", buf);
    }
    else
    {
        vsyslog(LOG_DEBUG, format, args);
    }
    va_end(args);
}

void log_trace(const char *format, ...)
{
    va_list args;

    va_start(args, format);

    printf("TRACE: ");
    vprintf(format, args);
    printf("\n");

    va_end(args);
}

void log_bug(const char *format, ...)
{
    va_list args;

    va_start(args, format);

    vsyslog(LOG_DEBUG, format, args);

    va_end(args);
}

