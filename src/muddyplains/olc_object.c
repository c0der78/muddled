
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
#include <muddyengine/db.h>
#include <muddyengine/object.h>
#include <muddyengine/string.h>
#include <muddyengine/lookup.h>
#include <muddyengine/room.h>
#include <muddyengine/area.h>
#include <stdlib.h>
#include <muddyengine/engine.h>
#include <muddyengine/account.h>
#include <muddyengine/macro.h>
#include <muddyengine/util.h>

Editor *build_object_editor(Object * object)
{

    Editor *editor = new_editor();

    editor->data = object;

    editor->edit = object_editor;

    editor->show = object_editor_menu;

    return editor;

}

void object_editor_menu(Client * conn)
{

    clear_screen(conn);

    set_cursor(conn, 1, 1);

    Object *object = (Object *) conn->editing->data;

    conn->titlef(conn, "Object Editor - object %d", object->id);

    writelnf(conn, "~C   Id: ~W%d", object->id);

    writelnf(conn, "~YA) ~CType: ~W%s~x", object_types[object->type].name);

    writelnf(conn, "~YB) ~CName: ~W%s~x", object->name);

    writelnf(conn, "~YC) ~CShort Description: ~W%s~x", object->shortDescr);

    writelnf(conn, "~YD) ~CLong Description: ~W%s~x", object->longDescr);

    string_editor_preview(conn, "~YE) ~CDescription", object->description);

    writelnf(conn, "~YF) ~CWeight: ~W%.1f~x", object->weight);

    writelnf(conn, "~YG) ~CCondition: ~W%.1f~x", object->condition);

    writelnf(conn, "~YH) ~CWear Loc: ~W%s~x",
             lookup_name(wear_flags, object->wearFlags));

}

void object_edit_list(Client * conn, Area * area)
{

    int count = 0;

    for (Object * obj = area->objects; obj != 0; obj = obj->next_in_area) {

        writelnf(conn, "%2d) %-12.12s ", obj->id, obj->shortDescr);

        if (++count % 4 == 0)
            writeln(conn, "");

    }

    if (count % 4 != 0)
        writeln(conn, "");

}

void object_editor(Client * conn, const char *argument)
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

        object_edit_list(conn, conn->account->playing->inRoom->area);

        return;

    }
    Object *object = (Object *) conn->editing->data;

    if (!str_cmp(arg, "save")) {

        save_object(object);

        writeln(conn, "~CNPC saved.~x");

        return;

    }
    if (!str_cmp(arg, "delete")) {

        delete_object(object);

        extract_obj(object);

        finish_editing(conn);

        return;

    }
    if (!str_cmp(arg, "A") || !str_cmp(arg, "type")) {

        long type = value_lookup(object_types, argument);

        if (type == -1) {

            writelnf(conn, "~CValid types are: ~W%s~x",
                     lookup_names(object_types));

            return;

        }
        object->type = (object_type) type;

        conn->editing->show(conn);

        return;

    }
    if (!str_cmp(arg, "B") || !str_cmp(arg, "name")) {

        if (!argument || !*argument) {

            writeln(conn, "~CYou must provide a name to set.~x");

            return;

        }
        free_str_dup(&object->name, argument);

        conn->editing->show(conn);

        return;

    }
    if (!str_cmp(arg, "D") || !str_cmp(arg, "description")) {

        Editor *editor = build_string_editor(&object->description);

        editor->next = conn->editing;

        conn->editing = editor;

        conn->editing->show(conn);

        return;

    }
    if (!str_cmp(arg, "C") || !str_cmp(arg, "short")) {

        if (!argument || !*argument) {

            writeln(conn,
                    "~CYou must provide a short description.~x");

            return;

        }
        free_str_dup(&object->shortDescr, argument);

        conn->editing->show(conn);

        return;

    }
    if (!str_cmp(arg, "D") || !str_cmp(arg, "long")) {

        if (!argument || !*argument) {

            writeln(conn,
                    "~CYou must provide a long description.~x");

            return;

        }
        free_str_dup(&object->longDescr, argument);

        conn->editing->show(conn);

        return;

    }
    if (!str_cmp(arg, "F") || !str_cmp(arg, "weight")) {

        if (!argument || !*argument || is_number(argument) != 2) {

            writeln(conn, "~CYou must specify a decimal number.~x");

            return;

        }
        object->weight = (float)atof(argument);

        conn->editing->show(conn);

        return;

    }
    if (!str_cmp(arg, "G") || !str_cmp(arg, "condition")) {

        if (!argument || !*argument || is_number(argument) != 2) {

            writeln(conn, "~CYou must specify a decimal number.~x");

            return;

        }
        float cond = (float)atof(argument);

        if (cond < 0.0 || cond > 100.0) {

            writeln(conn,
                    "~CValid ranges for condition is 0.0 to 100.0.~x");

            return;

        }
        object->condition = cond;

        conn->editing->show(conn);

        return;

    }
    if (!str_cmp(arg, "H") || !str_cmp(arg, "wearloc")) {

        long loc = value_lookup(wear_flags, argument);

        if (loc == -1) {

            writelnf(conn, "~CValid locations are: ~W%s~x",
                     lookup_names(wear_flags));

            return;

        }
        object->wearFlags = (wear_type) loc;

        conn->editing->show(conn);

        return;

    }
}
