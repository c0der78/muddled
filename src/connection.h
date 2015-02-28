
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

#ifndef MUDDLED_CONNECTION_H
#define MUDDLED_CONNECTION_H

#include <stdarg.h>
#include "typedef.h"

/*
 * To implement a connection, create a structure that has these fields in the same order,
 * adding additional fields afterwards.  Then you can cast your struct to a connection type.
 */
struct connection
{

    void (*handler) (Connection *, const char *);

    void (*writelnf) (Connection *, const char *, ...);

    void (*writeln) (Connection *, const char *);

    void (*writef) (Connection *, const char *, ...);

    void (*write) (Connection *, const char *);

    void (*page) (Connection *, const char *);

    void (*vwrite) (Connection *, const char *, va_list);

    void (*titlef) (Connection *, const char *, ...);

    void (*title) (Connection *, const char *);

    void (*vtitle) (Connection *, const char *, va_list);

    bool(*is_playing) (Connection *);

    short scrHeight;

    short scrWidth;

    const char *termType;

    const char *host;

    Account *account;

};

#endif              /* // #ifndef CONNECTION_H */
