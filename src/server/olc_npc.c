
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

#include "client.h"
#include "olc.h"
#include "telnet.h"
#include <muddled/character.h>
#include <muddled/nonplayer.h>
#include <muddled/string.h>
#include <muddled/lookup.h>
#include <muddled/room.h>
#include <muddled/area.h>
#include <muddled/macro.h>
#include <stdlib.h>
#include <muddled/engine.h>
#include <muddled/flag.h>
#include <muddled/account.h>
#include <muddled/log.h>
#include <muddled/util.h>

Editor *build_npc_editor(Character * npc)
{

    Editor *editor = new_editor();

    editor->data = npc;

    editor->edit = npc_editor;

    editor->show = npc_editor_menu;

    return editor;

}

void npc_editor_menu(Client * conn)
{

    clear_screen(conn);

    set_cursor(conn, 1, 1);

    Character *npc = (Character *) conn->editing->data;

    conn->titlef(conn, "NonPlayer Editor - npc %d", npc->id);

    character_editor_menu(conn, npc);

    writelnf(conn, "~YF) ~CShort Description: ~W%s~x",
             npc->npc->shortDescr);

    writelnf(conn, "~YG) ~CLong Description: ~W%s~x", npc->npc->longDescr);

    writelnf(conn, "~YH) ~CPosition: ~W%s~x",
             position_table[npc->npc->startPosition].name);

    writelnf(conn, "~YI) ~CFlags: ~W%s~x",
             format_flags(npc->flags, npc_flags));

    writelnf(conn, "~YJ) ~CAlignment: ~W%d~x", npc->alignment);

}

void npc_edit_list(Client * conn, Area * area)
{

    int count = 0;

    for (Character * vch = area->npcs; vch != 0; vch = vch->next_in_area)
    {

        writelnf(conn, "%2d) %-12.12s ", vch->id, vch->npc->shortDescr);

        if (++count % 4 == 0)
            writeln(conn, "");

    }

    if (count % 4 != 0)
        writeln(conn, "");

}

void npc_editor(Client * conn, const char *argument)
{

    char arg[100];

    argument = one_argument(argument, arg);

    if (!str_prefix(arg, "show"))
    {

        conn->editing->show(conn);

        return;

    }
    if (!str_cmp(arg, "list"))
    {

        npc_edit_list(conn, conn->account->playing->inRoom->area);

        return;

    }
    Character *npc = (Character *) conn->editing->data;

    if (!str_cmp(arg, "Q"))
    {

        if (npc->inRoom == 0)
            destroy_char(npc);

        finish_editing(conn);

        return;

    }
    if (!str_cmp(arg, "save"))
    {

        save_npc(npc);

        writeln(conn, "~CNPC saved.~x");

        return;

    }
    if (!str_cmp(arg, "delete"))
    {

        delete_npc(npc);

        writeln(conn, "~CNPC deleted.~x");

        extract_char(npc, true);

        finish_editing(conn);

        return;

    }
    if (character_editor(conn, npc, arg, argument))
        return;

    if (!str_cmp(arg, "F") || !str_cmp(arg, "short"))
    {

        if (!argument || !*argument)
        {

            writeln(conn,
                    "~CYou must provide a short description.~x");

            return;

        }
        free_str_dup(&npc->npc->shortDescr, argument);

        conn->editing->show(conn);

        return;

    }
    if (!str_cmp(arg, "G") || !str_cmp(arg, "long"))
    {

        if (!argument || !*argument)
        {

            writeln(conn,
                    "~CYou must provide a long description.~x");

            return;

        }
        free_str_dup(&npc->npc->longDescr, argument);

        conn->editing->show(conn);

        return;

    }
    if (!str_cmp(arg, "H") || !str_cmp(arg, "position"))
    {

        if (!argument || !*argument || argument[0] == '?')
        {

            writelnf(conn, "~CValid positions are: ~W%s~x",
                     lookup_names(position_table));

            return;

        }
        long p = value_lookup(position_table, argument);

        if (p == -1)
        {

            writeln(conn, "~CThat is not a valid position.~x");

            return;

        }
        npc->npc->startPosition = (position_t) p;

        conn->editing->show(conn);

        return;

    }
    if (!str_cmp(arg, "I") || !str_cmp(arg, "flags"))
    {

        if (edit_flag("flags", conn, npc->flags, argument, npc_flags))
            conn->editing->show(conn);

        return;

    }
    if (!str_cmp(arg, "J") || !str_cmp(arg, "alignment"))
    {

        int a = atoi(argument);

        if (a < -MAX_ALIGN || a > MAX_ALIGN)
        {

            writelnf(conn, "~CValue must be between %d and %d.~x",
                     -MAX_ALIGN, MAX_ALIGN);

            return;

        }
        npc->alignment = a;

        conn->editing->show(conn);

        return;

    }
}
