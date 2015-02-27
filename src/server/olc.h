
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
#ifndef OLC_H
#define OLC_H

typedef struct OLC OLC;

typedef struct Editor Editor;

#include "client.h"
#include "../character.h"
#include "../room.h"
#include "../help.h"
#include "../object.h"
#include "../area.h"
#include "../skill.h"
#include "../social.h"

void room_editor(Client *, const char *);
void exit_editor(Client *, const char *);
void exits_editor(Client *, const char *);
void string_editor(Client *, const char *);
void help_editor(Client *, const char *);
void reset_editor(Client *, const char *);
void npc_editor(Client *, const char *);
void object_editor(Client *, const char *);
void area_editor(Client *, const char *);
void player_editor(Client *, const char *);
void skill_editor(Client *, const char *);
void engine_editor(Client *, const char *);
void social_editor(Client *, const char *);

int character_editor(Client *, Character *, const char *, const char *);

void room_editor_menu(Client *);
void exits_editor_menu(Client *);
void string_editor_menu(Client *);
void help_editor_menu(Client *);
void reset_editor_menu(Client *);
void npc_editor_menu(Client *);
void object_editor_menu(Client *);
void area_editor_menu(Client *);
void player_editor_menu(Client *);
void skill_editor_menu(Client *);
void engine_editor_menu(Client *);
void social_editor_menu(Client *);

void character_editor_menu(Client *, Character *);

void olc_prompt(Client *);
void string_editor_preview(Client *, const char *, const char *);

Editor *build_room_editor(Room *);
Editor *build_exits_editor(Exit **);
Editor *build_exit_editor(Exit *);
Editor *build_string_editor(const char **);
Editor *build_help_editor(Help *);
Editor *build_npc_editor(Character *);
Editor *build_object_editor(Object *);
Editor *build_area_editor(Area *);
Editor *build_player_editor(Character *);
Editor *build_skill_editor(Skill *);
Editor *build_engine_editor(Engine *);
Editor *build_social_editor(Social *);

void room_edit_list(Client *, Area *);
void object_edit_list(Client *, Area *);
void help_edit_list(Client *);
void npc_edit_list(Client *, Area *);
void area_edit_list(Client *);
void player_edit_list(Client *);
void skill_edit_list(Client *);
void social_edit_list(Client *);

struct Editor
{
	void (*edit) (Client *, const char *);
	void (*show) (Client *);
	void *data;
	Editor *next;
};

#define EDIT_CHANGED 1
#define EDIT_NOCHANGE 0
#define EDIT_END -1
int edit_text(Client *, const char **, const char *);
int edit_flag(const char *, Client *, Flag *, const char *, const Lookup *);
int edit_string(const char *, Client *, const char **, const char *);

Editor *new_editor();
void destroy_editor(Editor *);
void finish_editing(Client *);

void olc_syntax(Client *, const char *, ...)
__attribute__ ((format(printf, 2, 3)));

#endif              /* // #ifndef OLC_H */
