
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
#ifndef MUDDLED_COMMAND_H
#define MUDDLED_COMMAND_H

#include "character.h"
#include "lookup.h"

typedef struct Command Command;

typedef void DoFun(const char *, Character *, const char *);

struct Command
{
    const char *name;
    short level;
    DoFun *dofun;
    int category;
    position_t position;
};

extern int max_command;
void command_interpret(Character *, const char *);
void cmd_syntax(Character *, const char *, ...)
__attribute__ ((format(printf, 2, 3)));

extern const Command cmd_table[];

#define DOFUN(funname) void do_##funname(const char *do_name, Character *ch, const char *argument)

extern const Lookup command_types[];

enum
{
    COMMAND_INFO = (1 << 0),
    COMMAND_SETTINGS = (1 << 1),
    COMMAND_ADMIN = (1 << 2),
    COMMAND_MANIP = (1 << 3),
    COMMAND_CONN = (1 << 4),
    COMMAND_COMM = (1 << 5),
    COMMAND_COMBAT = (1 << 6)
};

DoFun do_quit;
DoFun do_logout;
DoFun do_commands;
DoFun do_shutdown;
DoFun do_reboot;
DoFun do_save;
DoFun do_look;
DoFun do_north;
DoFun do_east;
DoFun do_south;
DoFun do_west;
DoFun do_up;
DoFun do_down;
DoFun do_edit;
DoFun do_color;
DoFun do_time;
DoFun do_title;
DoFun do_who;
DoFun do_score;
DoFun do_autotick;
DoFun do_brief;
DoFun do_say;
DoFun do_timezone;
DoFun do_hints;
DoFun do_logoff;
DoFun do_delete;
DoFun do_kill;
DoFun do_map;
DoFun do_explored;
DoFun do_kick;
DoFun do_cast;
DoFun do_automap;
DoFun do_prompt;
DoFun do_admin;
DoFun do_chat;
DoFun do_autologin;
DoFun do_get;
DoFun do_inventory;
DoFun do_equipment;
DoFun do_drop;
DoFun do_wear;
DoFun do_note;
DoFun do_forum;
DoFun do_subscribe;
DoFun do_think;
DoFun do_whisper;
DoFun do_shout;
DoFun do_force;
DoFun do_affects;
DoFun do_resists;
DoFun do_sockets;
DoFun do_goto;
DoFun do_socials;
DoFun do_import;
DoFun do_db;
DoFun do_help;
DoFun do_areas;

#endif              /* // #ifndef COMMAND_H */
