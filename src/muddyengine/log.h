
/******************************************************************************
 *         __  __           _     _         ____  _       _                   *
 *        |  \/  |_   _  __| | __| |_   _  |  _ \| | __ _(_)_ __  ___         *
 *        | |\/| | | | |/ _` |/ _` | | | | | |_) | |/ _` | | '_ \/ __|        *
 *        | |  | | |_| | (_| | (_| | |_| | |  __/| | (_| | | | | \__ \        *
 *        |_|  |_|\__,_|\__,_|\__,_|\__, | |_|   |_|\__,_|_|_| |_|___/        *
 *                                  |___/                                     *
 *                                                                            *
 *    (C) 2010 by Ryan Jennings <c0der78@gmail.com> www.ryan-jennings.net     *
 *	           Many thanks to creators of muds before me.                 *
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

void log_fun(const char *, int, const char *, ...)
    __attribute__ ((format(printf, 3, 4)));

/*
 * void log_info(const char *, ...) __attribute__((format(printf, 1, 2)));
 * 
 * void log_error(const char *, ...) __attribute__((format(printf, 1,
 * 2)));
 * 
 * void log_debug(const char *, ...) __attribute__((format(printf, 1,
 * 2)));
 * 
 * void log_warn(const char *, ...) __attribute__((format(printf, 1, 2)));
 * 
 * void log_data(const char *, ...) __attribute__((format(printf, 1, 2)));
 * 
 * void log_trace(const char *, ...) __attribute__((format(printf, 1,
 * 2)));
 * 
 * void log_bug(const char *, ...) __attribute__((format(printf, 1, 2))); 
 */

#define log_info(msg, ...)	log_fun(__FUNCTION__, LOG_INFO, msg, ## __VA_ARGS__)
#define log_error(msg, ...)	log_fun(__FUNCTION__, LOG_ERR, msg, ## __VA_ARGS__)
#define log_debug(msg, ...)	log_fun(__FUNCTION__, LOG_DEBUG, msg, ## __VA_ARGS__)
#define log_warn(msg, ...)	log_fun(__FUNCTION__, LOG_WARN, msg, ## __VA_ARGS__)
#define log_data(msg, ...)	log_fun(__FUNCTION__, LOG_DATA, msg, ## __VA_ARGS__)
#define log_trace(msg, ...)	log_fun(__FUNCTION__, LOG_TRACE, msg, ## __VA_ARGS__)
#define log_bug(msg, ...)	log_fun(__FUNCTION__, LOG_BUG, msg, ## __VA_ARGS__)

enum {
	LOG_INFO,
	LOG_ERR,
	LOG_DEBUG,
	LOG_WARN,
	LOG_DATA,
	LOG_TRACE,
	LOG_BUG,
	LOG_FILE
};

#include <muddyengine/lookup.h>

extern const Lookup logging_flags[];
#endif				/* // #ifndef LOG_H */
