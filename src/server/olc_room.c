
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "client.h"
#include "olc.h"
#include "telnet.h"
#include "../room.h"
#include "../str.h"
#include "../engine.h"
#include "../exit.h"
#include "../macro.h"
#include "../lookup.h"
#include "../account.h"

Editor *build_room_editor(Room *room)
{
    Editor *editor = new_editor();

    editor->data = room;

    editor->edit = room_editor;

    editor->show = room_editor_menu;

    return editor;
}

void room_editor_menu(Client *conn)
{
    clear_screen(conn);

    set_cursor(conn, 1, 1);

    Room *room = (Room *) conn->editing->data;

    conn->titlef(conn, "Room Editor - Room %d", room->id);

    writelnf(conn, "~C   Id: ~W%d", room->id);
    writelnf(conn, "~YA) ~CName: ~W%s~x", room->name);

    int count = 0;

    string_editor_preview(conn, "~YB) ~CDescription", room->description);

    writelnf(conn, "~YC) ~CSector: ~W%s~x",
             sector_table[room->sector].name);

    writelnf(conn, "~YD) ~CFlags: ~W%s~x",
             format_flags(room->flags, room_flags));

    for (int i = 0; i < MAX_DIR; i++)
    {
        if (room->exits[i] == 0)
            continue;

        count++;
    }

    writelnf(conn, "~RE) ~CExits: ~W%d exits.~x", count);

    string_editor_preview(conn, "~YF) ~CReset", room->reset);
}

void room_edit_list(Client *conn, Area *area)
{
    int count = 0;
    for (Room *room = area->rooms; room != 0; room = room->next_in_area)
    {
        writelnf(conn, "%2d) %-12.12s ", room->id, room->name);
        if (++count % 4 == 0)
            writeln(conn, "");
    }
    if (count % 4 != 0)
        writeln(conn, "");
}

void room_editor(Client *conn, const char *argument)
{
    char arg[100];

    argument = one_argument(argument, arg);

    if (!str_prefix(arg, "show"))
    {
        conn->editing->show(conn);
        return;
    }
    if (!str_cmp(arg, "Q"))
    {
        finish_editing(conn);
        return;
    }
    if (!str_cmp(arg, "list"))
    {
        room_edit_list(conn, conn->account->playing->inRoom->area);
        return;
    }
    Room *room = (Room *) conn->editing->data;

    if (!str_cmp(arg, "dig"))
    {
        if (!argument || !*argument)
        {
            writeln(conn, "~CDig an exit in which direction?~x");
            return;
        }
        long dir = value_lookup(direction_table, argument);

        if (dir == -1)
        {
            writelnf(conn, "~C'%s' is not a valid direction.~x",
                     argument);
            return;
        }
        if (room->exits[dir] != 0)
        {
            writeln(conn,
                    "~CThere is already an exit in that direction.~x");
            return;
        }
        room->exits[dir] = new_exit();
        room->exits[dir]->fromRoom = room;

        Editor *edit = build_exit_editor(room->exits[dir]);

        edit->next = conn->editing;
        conn->editing = edit;

        conn->editing->show(conn);
        return;
    }
    if (!str_cmp(arg, "save"))
    {
        save_room(room);

        writeln(conn, "~CRoom saved.~x");
        return;
    }
    if (!str_cmp(arg, "A") || !str_cmp(arg, "name"))
    {
        if (!argument || !*argument)
        {
            writeln(conn, "~CYou must provide a name to set.~x");
            return;
        }
        free_str_dup(&room->name, argument);
        conn->editing->show(conn);
        return;
    }
    if (!str_cmp(arg, "B") || !str_cmp(arg, "description"))
    {
        Editor *editor = build_string_editor(&room->description);

        editor->next = conn->editing;
        conn->editing = editor;

        conn->editing->show(conn);
        return;
    }
    if (!str_cmp(arg, "C") || !str_cmp(arg, "sector"))
    {
        long sec = value_lookup(sector_table, argument);

        if (sec == -1)
        {
            writelnf(conn, "~CValid sectors are: ~W%s~x",
                     lookup_names(sector_table));
            return;
        }
        room->sector = (sector_t) sec;

        conn->editing->show(conn);
        return;
    }
    if (!str_cmp(arg, "D") || !str_cmp(arg, "flags"))
    {
        if (edit_flag("flags", conn, room->flags, argument, room_flags))
            conn->editing->show(conn);
        return;
    }
    if (!str_cmp(arg, "E") || !str_cmp(arg, "exits"))
    {
        Editor *editor = build_exits_editor(room->exits);

        editor->next = conn->editing;
        conn->editing = editor;

        conn->editing->show(conn);
        return;
    }
    if (!str_cmp(arg, "F") || !str_cmp(arg, "reset"))
    {
        Editor *editor = build_string_editor(&room->reset);
        editor->next = conn->editing;
        conn->editing = editor;

        conn->editing->show(conn);
        return;
    }
}
