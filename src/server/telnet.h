
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

#ifndef TELNET_H
#define TELNET_H

#define IAC 255

#define DONT    254
#define DO  253
#define WONT    252
#define WILL    251
#define SB  250

#define SE      240

#define SEND    1
#define IS      0

#define TELOPT_ECHO 1
#define TELOPT_TTYPE    24
#define TELOPT_NAWS 31
#define TELOPT_TSPEED   32

#include <string.h>
#include "client.h"

ssize_t test_telopts(const Client *);

ssize_t send_telopt(const Client *, unsigned char, unsigned char);

void process_telnet(Client *, unsigned char *, size_t);

void set_cursor(Client *, int, int);

void restore_cursor(Client *);

void clear_screen(Client *);

#endif              /* // #ifndef TELNET_H */
