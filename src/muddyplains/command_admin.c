
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

#include "command.h"
#include <muddyengine/character.h>
#include <muddyengine/player.h>
#include <muddyengine/account.h>
#include <muddyengine/help.h>
#include <muddyengine/room.h>
#include <libgen.h>
#include "server.h"
#include "olc.h"
#include "client.h"
#include <muddyengine/object.h>
#include <muddyengine/social.h>
#include <muddyengine/area.h>
#include <muddyengine/string.h>
#include <muddyengine/skill.h>
#include <muddyengine/nonplayer.h>
#include <muddyengine/buffer.h>
#include <muddyengine/util.h>
#include "importer.h"

DOFUN(edit)
{

    char arg[100];

    Client *conn;

    if (!ch->pc || !ch->pc->conn)
        return;

    if (!argument || !*argument)
    {

        cmd_syntax(ch, do_name, "room", "help", "social", "npc",
                   "object", "area", "player", "skill", "engine", 0);

        /*
         * writeln(ch, "Syntax: %s room [?]", name); writeln(ch, " %s help
         * [?]", name); writeln(ch, " %s npc [?]", name); writeln(ch, " %s
         * object [?]", name); writeln(ch, " %s area [?]", name);
         * writeln(ch, " %s player [?]", name);
         */
        return;

    }
    conn = (Client *) ch->pc->conn;

    argument = one_argument(argument, arg);

    if (!str_prefix(arg, "room"))
    {

        if (!argument || !*argument)
        {

            conn->editing = build_room_editor(ch->inRoom);

            conn->editing->show(conn);

            return;

        }
        if (!str_prefix(argument, "create"))
        {

            Room *room = new_room();

            room->area = ch->inRoom->area;

            conn->editing = build_room_editor(room);

            conn->editing->show(conn);

            return;

        }
        if (!str_prefix(argument, "list"))
        {

            room_edit_list(conn, ch->inRoom->area);

            return;

        }
        if (is_number(argument))
        {

            int id = atoi(argument);

            Room *room = get_room_by_id(id);

            if (room == 0)
            {

                writeln(conn, "No such room!");

                return;

            }
            conn->editing = build_room_editor(room);

            conn->editing->show(conn);

            return;

        }
        cmd_syntax(ch, do_name,
                   "room		- edit the current room",
                   "room create	- create a room for the current area",
                   "room id#	- edit a specific room",
                   "room list	- lists rooms in area", 0);

        /*
         * writeln(conn, "Syntax: edit room - edit the current room");
         * writeln(conn, " edit room create - create a room for the
         * current area"); writeln(conn, " edit room id# - edit a specific
         * room"); writeln(conn, " edit room list - lists rooms in area");
         */
        return;

    }
    if (!str_prefix(arg, "help"))
    {

        if (!argument || !*argument || argument[0] == '?')
        {

            cmd_syntax(ch, do_name,
                       "help <keyword or id#> 	- edit a specific help",
                       "help create	 	- create a help",
                       "help list		- list helps", 0);

            return;

        }
        Help *help;

        if (!str_prefix(argument, "create"))
        {

            help = new_help();

            conn->editing = build_help_editor(help);

            conn->editing->show(conn);

            return;

        }
        if (!str_prefix(argument, "list"))
        {

            help_edit_list(conn);

        }
        help = help_find(argument);

        if (help == 0)
        {

            writeln(conn, "No such help.");

            return;

        }
        conn->editing = build_help_editor(help);

        conn->editing->show(conn);

        return;

    }
    if (!str_prefix(arg, "social"))
    {

        if (!argument || !*argument || argument[0] == '?')
        {

            cmd_syntax(ch, do_name,
                       "social <keyword or id#> 	- edit a specific social",
                       "social create	 	- create a social",
                       "social list		- list socials", 0);

            return;

        }
        Social *social;

        if (!str_prefix(argument, "create"))
        {

            social = new_social();

            conn->editing = build_social_editor(social);

            conn->editing->show(conn);

            return;

        }
        if (!str_prefix(argument, "list"))
        {

            social_edit_list(conn);

        }
        social = social_lookup(argument);

        if (social == 0)
        {

            writeln(conn, "No such social.");

            return;

        }
        conn->editing = build_social_editor(social);

        conn->editing->show(conn);

        return;

    }
    if (!str_prefix(arg, "skill"))
    {

        if (!argument || !*argument || argument[0] == '?')
        {

            cmd_syntax(ch, do_name,
                       "skill <name or id#> 	- edit a specific skill",
                       "skill create	 	- create a skill",
                       "skill list		- list skills", 0);

            return;

        }
        Skill *skill;

        if (!str_prefix(argument, "create"))
        {

            skill = new_skill();

            conn->editing = build_skill_editor(skill);

            conn->editing->show(conn);

            return;

        }
        if (!str_prefix(argument, "list"))
        {

            skill_edit_list(conn);

        }
        skill = skill_lookup(argument);

        if (skill == 0)
        {

            writeln(conn, "No such skill.");

            return;

        }
        conn->editing = build_skill_editor(skill);

        conn->editing->show(conn);

        return;

    }
    if (!str_prefix(arg, "player"))
    {

        if (!argument || !*argument || argument[0] == '?')
        {

            cmd_syntax(ch, do_name,
                       "player <keyword or id#> 	- edit a specific player",
                       "player list			- list players",
                       0);

            return;

        }
        Character *vch;

        if (!str_prefix(argument, "list"))
        {

            player_edit_list(conn);

            return;

        }
        vch = player_lookup(argument);

        if (vch == 0)
        {

            vch =
                load_player_by_name((Connection *) conn, argument);

            if (vch == 0)
            {

                writeln(conn, "No such player.");

                return;

            }
        }
        conn->editing = build_player_editor(vch);

        conn->editing->show(conn);

        return;

    }
    if (!str_prefix(arg, "npc"))
    {

        if (!argument || !*argument || argument[0] == '?')
        {

            cmd_syntax(ch, do_name,
                       "npc <keyword or id#> 	- edit a specific npc",
                       "npc create	 	- create an npc",
                       "npc list		- list npcs", 0);

            return;

        }
        Character *vch;

        if (!str_prefix(argument, "create"))
        {

            vch = new_char();

            vch->npc = new_npc();

            conn->editing = build_npc_editor(vch);

            conn->editing->show(conn);

            return;

        }
        if (!str_prefix(argument, "list"))
        {

            npc_edit_list(conn, ch->inRoom->area);

            return;

        }
        vch = npc_lookup(argument);

        if (vch == 0)
        {

            writeln(conn, "No such npc.");

            return;

        }
        conn->editing = build_npc_editor(vch);

        conn->editing->show(conn);

        return;

    }
    if (!str_prefix(arg, "object"))
    {

        if (!argument || !*argument || argument[0] == '?')
        {

            cmd_syntax(ch, do_name,
                       "object <keyword or id#> - edit a specific object",
                       "object create	 	 - create an object",
                       "object list		 - list objects", 0);

            return;

        }
        Object *obj;

        if (!str_prefix(argument, "create"))
        {

            obj = new_object();

            conn->editing = build_object_editor(obj);

            conn->editing->show(conn);

            return;

        }
        if (!str_prefix(argument, "list"))
        {

            object_edit_list(conn, ch->inRoom->area);

            return;

        }
        obj = object_lookup(argument);

        if (obj == 0)
        {

            writeln(conn, "No such object.");

            return;

        }
        conn->editing = build_object_editor(obj);

        conn->editing->show(conn);

        return;

    }
    if (!str_prefix(arg, "area"))
    {

        if (!argument || !*argument || argument[0] == '?')
        {

            cmd_syntax(ch, do_name,
                       "area <keyword or id#> 	- edit a specific area",
                       "area create	 	- create an area",
                       "area list		- list areas", 0);

            return;

        }
        Area *area;

        if (!str_prefix("create", argument))
        {

            area = new_area();

            conn->editing = build_area_editor(area);

            conn->editing->show(conn);

            return;

        }
        if (!str_prefix(argument, "list"))
        {

            area_edit_list(conn);

            return;

        }
        area = area_lookup(argument);

        if (area == 0)
        {

            writeln(conn, "No such area.");

            return;

        }
        conn->editing = build_area_editor(area);

        conn->editing->show(conn);

        return;

    }
    if (!str_prefix(arg, "engine"))
    {

        conn->editing = build_engine_editor(&engine_info);

        conn->editing->show(conn);

        return;

    }
    do_edit(do_name, ch, str_empty);

}

DOFUN(shutdown)
{

    if (!is_implementor(ch))
    {

        writeln(ch, "You do not have permission to do that.");

        return;

    }
    writeln(ch, "Shutting down server...");

    act_pos(TO_WORLD, POS_DEAD, ch, 0, 0, "$n has shutdown the server...");

    stop_server();

}

DOFUN(reboot)
{

    if (!is_implementor(ch))
    {

        writeln(ch, "You do not have permission to do that.");

        return;

    }
    writeln(ch, "Rebooting server...");

    act_pos(TO_WORLD, POS_DEAD, ch, 0, 0, "$n has rebooted the server...");

    reboot_server();

}

DOFUN(force)
{

    char arg[ARG_SIZ];

    extern int pulse_tick;

    if (nullstr(argument))
    {

        cmd_syntax(ch, do_name, "tick	- updates a game tick", 0);

        return;

    }
    argument = one_argument(argument, arg);

    if (!str_cmp(arg, "tick"))
    {

        pulse_tick = 0;

        writeln(ch, "Game tick forced.");

        return;

    }
    do_force(do_name, ch, "");

}

DOFUN(sockets)
{

    Buffer *buf = new_buf();

    for (Client * c = first_client; c; c = c->next)
    {

        writelnf(buf, "~W%s~x - ~C%s~x (~G%s~x) ~B%s~x", getip(c),
                 c->host, c->termType,
                 c->account->playing ? c->account->playing->
                 name : "Unknown");

    }

    ch->page(ch, buf_string(buf));

    destroy_buf(buf);

}

DOFUN(goto)
{

    Room *loc = find_location(ch, argument);

    if (loc == 0)
    {

        writeln(ch, "Unable to find that location.");

        return;

    }
    act(TO_ROOM, ch, 0, 0, "$n dissappears in a swirling mist.");

    char_from_room(ch);

    char_to_room(ch, loc);

    act(TO_ROOM, ch, 0, 0, "$n appears in a swirling mist.");

    do_look(str_empty, ch, str_empty);

}

DOFUN(import)
{

    if (nullstr(argument))
    {

        cmd_syntax(ch, do_name,
                   "rom <filename>	- import a rom area file",
                   "commit		    - commit an import", 0);

        return;

    }
    char arg[ARG_SIZ];

    argument = one_argument(argument, arg);

    if (!str_cmp(arg, "rom"))
    {

        if (nullstr(argument))
        {

            writeln(ch, "Please specify a file to import.");

            return;

        }
        FILE *fp = fopen(argument, "r");

        if (fp == 0)
        {

            writelnf(ch, "Could not open %s for reading.",
                     argument);

            return;

        }
        if (!str_suffix(".lst", argument))
        {

            if (!import_rom_area_list
                    (dirname((char *)argument), fp))
                writeln(ch, "Could not import areas.");

            else
            {

                writeln(ch, "Areas imported.");

            }

        }
        else
        {

            if (!import_rom_file(fp))
                writeln(ch, "Could not import file.");

            else
            {

                writeln(ch, "File imported.");

            }

        }

        return;

    }
    if (!str_cmp(arg, "commit"))
    {
        import_commit(true);
        writeln(ch, "Import committed.");
        return;
    }
    do_import(do_name, ch, str_empty);

}

DOFUN(db)
{

    if (nullstr(argument))
    {

        cmd_syntax(ch, do_name,
                   "save [type]	- saves to the database", 0);

        return;

    }
    char arg[ARG_SIZ];

    argument = one_argument(argument, arg);

    if (!str_cmp(arg, "save"))
    {

        if (nullstr(argument))
        {

            writeln(ch, "Valid types are: socials areas");

            return;

        }
        if (!str_prefix(argument, "socials"))
        {

            save_socials();

            writeln(ch, "Socials saved.");

            return;

        }
        else if (!str_prefix(argument, "areas"))
        {

            for (Area * area = first_area; area; area = area->next)
            {

                if (!is_set(area->flags, AREA_CHANGED))
                    continue;

                remove_bit(area->flags, AREA_CHANGED);

                save_area(area);

            }

            writeln(ch, "Areas saved.");

            return;

        }
        do_db(do_name, ch, arg);

        return;

    }
    do_db(do_name, ch, str_empty);

}
