
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

#ifndef CHANNEL_H
#define CHANNEL_H

typedef struct channel Channel;

typedef enum
{
    channel_global,
    channel_admin
} channel_t;

typedef enum
{
    CHANNEL_NORMAL,
    CHANNEL_SOCIAL,
    CHANNEL_EMOTE,
    CHANNEL_THINK,
    CHANNEL_WHO
} channel_arg_t;

#include "character.h"

struct channel
{

    int *pgcn;

    int bit;

    const char *name;

    const char *description;

    const char *format;

    const char *color;

    channel_t type;

};

enum
{
    CHANNEL_CHAT,
    CHANNEL_ADMIN
};

extern int gcn_chat;

extern int gcn_admin;

extern const Channel channel_table[];

int interpret_channel(Character *, int gcn, const char *);

void initialize_channels();

const char *say_verb(const char *, Character *, Character *, int);

extern const Lookup channel_flags[];
void announce(Character *, info_t, const char *, ...)
__attribute__ ((format(printf, 3, 4)));

#endif              /* // #ifndef CHANNEL_H */
