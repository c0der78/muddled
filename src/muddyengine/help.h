
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

#ifndef HELP_H
#define HELP_H

typedef struct help Help;

typedef enum
{
    HELP_UNKNOWN,
    HELP_INFO,
    HELP_COMMUNICATION,
} help_category;

#include <muddyengine/lookup.h>
#include <muddyengine/engine.h>

struct help
{
    identifier_t id;
    const char *keywords;
    const char *syntax;
    const char *text;
    Help *related;
    help_category category;
    Help *next;
    Help *next_related;
};

Help *new_help();
void destroy_help(Help *);
int load_helps();
Help *help_find(const char *);
Help *help_match(const char *);
int save_help(Help *);
const char *help_related_string(Help *);
extern Help *first_help;
extern const char *greeting;

extern const Lookup help_categories[];
#endif				/* // #ifndef HELP_H */
