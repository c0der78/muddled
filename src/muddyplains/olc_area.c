
/******************************************************************************
 *         __  __           _     _         ____  _       _                   *
 *        |  \/  |_   _  __| | __| |_   _  |  _ \| | __ _(_)_ __  ___         *
 *        | |\/| | | | |/ _` |/ _` | | | | | |_) | |/ _` | | '_ \/ __|        *
 *        | |  | | |_| | (_| | (_| | |_| | |  __/| | (_| | | | | \__ \        *
 *        |_|  |_|\__,_|\__,_|\__,_|\__, | |_|   |_|\__,_|_|_| |_|___/        *
 *                                  |___/                                     *
 *                                                                            *
 *    (C) 2010 by Ryan Jennings <c0der78@gmail.com> www.ryan-jennings.net     *
 *	           Many thanks to creators of muds before me.                 *
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
#include <muddyengine/string.h>
#include <muddyengine/engine.h>
#include <muddyengine/area.h>
#include <muddyengine/lookup.h>
#include <muddyengine/macro.h>

Editor *build_area_editor(Area * area)
{

    Editor *editor = new_editor();

    editor->data = area;

    editor->edit = area_editor;

    editor->show = area_editor_menu;

    return editor;

}

void area_editor_menu(Client * conn)
{

    clear_screen(conn);

    set_cursor(conn, 1, 1);

    Area *area = (Area *) conn->editing->data;

    conn->titlef(conn, "Area Editor - Area %d", area->id);

    writelnf(conn, "~C   Id: ~W%d~x", area->id);

    writelnf(conn, "~YA) ~CName: ~W%s~x", area->name);

    writelnf(conn, "~YB) ~CFlags: ~W%s~x",
             format_flags(area->flags, area_flags));

}

void area_edit_list(Client * conn)
{

    int count = 0;

    for (Area * area = first_area; area != 0; area = area->next) {

        writelnf(conn, "%2d) %12.12s ", area->id, area->name);

        if (++count % 4 == 0)
            writeln(conn, "");

    }

    if (count % 4 != 0)
        writeln(conn, "");

}

void area_editor(Client * conn, const char *argument)
{

    char arg[100];

    argument = one_argument(argument, arg);

    if (!str_prefix(arg, "show")) {

        conn->editing->show(conn);

        return;

    }
    if (!str_cmp(arg, "Q")) {

        finish_editing(conn);

        return;

    }
    if (!str_cmp(arg, "list")) {

        area_edit_list(conn);

        return;

    }
    Area *area = (Area *) conn->editing->data;

    if (!str_cmp(arg, "save")) {

        save_area(area);

        writeln(conn, "~CArea saved.~x");

        return;

    }
    if (!str_cmp(arg, "A") || !str_cmp(arg, "name")) {

        if (!argument || !*argument) {

            writeln(conn, "~CYou must provide a name to set.~x");

            return;

        }
        free_str_dup(&area->name, argument);

        conn->editing->show(conn);

        return;

    }
    if (!str_cmp(arg, "B") || !str_cmp(arg, "flags")) {

        if (edit_flag("flags", conn, area->flags, argument, area_flags))
            conn->editing->show(conn);

        return;

    }
}
