
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

#ifndef EXIT_H
#define EXIT_H

typedef struct exit Exit;

typedef enum
{
    DIR_NORTH,
    DIR_EAST,
    DIR_SOUTH,
    DIR_WEST,
    DIR_UP,
    DIR_DOWN,
    MAX_DIR
} direction_t;

#include "room.h"

struct exit
{
    Exit *next;
    identifier_t id;
    union
    {
        Room *room;
        identifier_t id;
    } to;
    Room *fromRoom;
    Flag *flags;
    Flag *status;
    identifier_t key;
};

enum
{
    EXIT_ISDOOR,
    EXIT_CLOSED,
    EXIT_LOCKED
};

Exit *new_exit();
void destroy_exit(Exit *);
int load_exits(Room *);
int save_exit(Exit *, direction_t);
void finalize_exits();
extern const Lookup direction_table[];
extern const Lookup exit_flags[];
#endif              /* // #ifndef EXIT_H */
