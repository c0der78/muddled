
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
#ifndef MUDDLED_LOG_H
#define MUDDLED_LOG_H

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "cdecl.h"

BEGIN_DECL

void log_info(const char *, ...) __attribute__((format(printf, 1, 2)));

void log_error(const char *, ...) __attribute__((format(printf, 1, 2)));

void log_errno(int errornum);

void log_debug(const char *, ...) __attribute__((format(printf, 1, 2)));

void log_warn(const char *, ...) __attribute__((format(printf, 1, 2)));

void log_data(const char *, ...) __attribute__((format(printf, 1, 2)));

void log_trace(const char *, ...) __attribute__((format(printf, 1, 2)));

void log_bug(const char *, ...) __attribute__((format(printf, 1, 2)));

END_DECL

#endif /* // #ifndef LOG_H */
