
/******************************************************************************
 *         __  __           _     _         ____  _       _                   *
 *        |  \/  |_   _  __| | __| |_   _  |  _ \| | __ _(_)_ __  ___         *
 *        | |\/| | | | |/ _` |/ _` | | | | | |_) | |/ _` | | '_ \/ __|        *
 *        | |  | | |_| | (_| | (_| | |_| | |  __/| | (_| | | | | \__ \        *
 *        |_|  |_|\__,_|\__,_|\__,_|\__, | |_|   |_|\__,_|_|_| |_|___/        *
 *                                  |___/                                     *
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
#ifndef LOG_H
#define LOG_H


void log_info(const char *, ...) __attribute__((format(printf, 1, 2)));

void log_error(const char *, ...) __attribute__((format(printf, 1, 2)));

void log_debug(const char *, ...) __attribute__((format(printf, 1, 2)));

void log_warn(const char *, ...) __attribute__((format(printf, 1, 2)));

void log_data(const char *, ...) __attribute__((format(printf, 1, 2)));

void log_trace(const char *, ...) __attribute__((format(printf, 1, 2)));

void log_bug(const char *, ...) __attribute__((format(printf, 1, 2)));


#endif              /* // #ifndef LOG_H */
