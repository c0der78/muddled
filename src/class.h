
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

#ifndef MUDDLED_CLASS_H
#define MUDDLED_CLASS_H

typedef struct clazz Class;

#include "character.h"
#include  <stdbool.h>

struct clazz
{
    identifier_t id;
    const char *name;
    const char *description;
    bool fMana;
    short thac0;
    short thac32;
};

extern Class *class_table;
extern int max_class;
Class *new_class();
void destroy_class(Class *);

int load_classes();
int class_count(const Character *);
bool is_valid_class(int);
int class_lookup(const char *);
int get_class_by_id(identifier_t);
bool has_class(Character *, int);
const char *class_short(Character *);
const char *class_who(Character *);

#endif              /* // #ifndef CLASS_H */
