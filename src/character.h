
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

#ifndef MUDDLED_CHARACTER_H
#define MUDDLED_CHARACTER_H

#include "cdecl.h"
#include "flag.h"
#include "affect.h"
#include "player.h"
#include "nonplayer.h"
#include "race.h"
#include "fight.h"
#include "room.h"

extern const Lookup sex_table[];
extern const Lookup position_table[];
extern Character *first_character;

BEGIN_DECL
Character *new_char();
void destroy_char(Character *);
short scrwidth(const Character *);
short scrheight(const Character *);
int load_char_column(Character *, sql_stmt *, const char *, int);
int save_character(Character *, const Lookup *);
int delete_character(Character *);
int load_char_objs(Character *);
int save_char_objs(Character *);
bool is_immortal(const Character *);
bool is_implementor(const Character *);
void extract_char(Character *, bool);
int load_char_affects(Character *);
int save_char_affects(Character *);
bool is_playing(const Character *);
bool is_player(const Character *);
Character *get_char_world(Character *, const char *);
END_DECL

#define MAX_ALIGN   2500

#endif              /* // #ifndef CHARACTER_H */
