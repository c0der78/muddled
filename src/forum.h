
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

#ifndef MUDDLED_FORUM_H
#define MUDDLED_FORUM_H

#include "typedef.h"

extern Forum *forum_table;
extern int max_forum;

enum
{

	FORUM_CHANGED = (1 << 0),

	FORUM_NOUNSUB = (1 << 1)
};

#define FORUM_ERROR -1

BEGIN_DECL

Note *new_note();
void destroy_note(Note *);
int lookup_forum_by_name(const char *);
int lookup_forum_by_id(identifier_t);
void finish_note(Forum *, Note *);
bool is_note_to(Character *, Note *);
void show_note_to_char(Character *, Note *, int);
void next_forum(Character *);
Note *find_note(Character *, Forum *, int);
int unread_notes(Character *, Forum *);
void check_notes(Forum *);
bool is_subscribed(Account *, int);
void set_subscribed(Account *, int);
void remove_subscribed(Account *, int);
void note_check(Character *);
void show_forum(Character *, bool);
void save_forums();
int load_forums();
Note *last_note(Character *, Forum *);
int delete_note(Forum *, Note *);

END_DECL

#endif              /* // #ifndef FORUM_H */
