
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
#include "../flag.h"
#include "../exit.h"
#include "../room.h"
#include "../str.h"
#include "../lookup.h"
#include <stdlib.h>
#include "telnet.h"
#include "olc.h"

void exit_editor_menu(Client *);

Editor *build_exit_editor(Exit *exit)
{

    Editor *editor = new_editor();

    editor->data = exit;

    editor->edit = exit_editor;

    editor->show = exit_editor_menu;

    return editor;

}

Editor *build_exits_editor(Exit **exits)
{

    Editor *editor = new_editor();

    editor->data = exits;

    editor->edit = exits_editor;

    editor->show = exits_editor_menu;

    return editor;

}

void exit_editor_menu(Client *conn)
{

    clear_screen(conn);

    set_cursor(conn, 1, 1);

    Exit *exit = (Exit *) conn->editing->data;

    conn->titlef(conn, "Exit Editor - Room %d", exit->fromRoom->id);

    writelnf(conn, "   ~CId: ~W%d~x", exit->id);

    writelnf(conn, "~YA) ~CToRoom: ~W%d~x",
             exit->to.room == 0 ? 0 : exit->to.room->id);

    writelnf(conn, "~YB) ~CFlags: ~W%d~x",
             format_flags(exit->flags, exit_flags));

}

void exits_editor_menu(Client *conn)
{

    Exit **exits = (Exit **) conn->editing->data;

    clear_screen(conn);

    set_cursor(conn, 1, 1);

    if (conn->editing->next != 0)
        conn->titlef(conn, "Exits Editor - Room %d",
                     ((Room *) conn->editing->next->data)->id);

    else
        conn->title(conn, "Exits Editor");

    writeln(conn, "~Y(A)~Cdd an exit.");

    writeln(conn, "~Y(D)~Celete an exit.");

    writeln(conn, "~Y(E)~Cdit an exit.~x");

    writeln(conn, "");

    writeln(conn, "~CExits:~x");

    int count = 0;

    for (const Lookup *t = direction_table; t->name != 0; t++)
    {

        if (exits[t->value] == 0)
            continue;

        writelnf(conn, "  ~Y%d) %s~C -> Room %d (%s)~x", ++count,
                 capitalize(t->name),
                 exits[t->value]->to.room->id,
                 exits[t->value]->to.room->name);

    }

}

void exits_editor(Client *conn, const char *argument)
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
    Exit **exits = (Exit **) conn->editing->data;

    if (!str_cmp(arg, "A") || !str_prefix(arg, "create"))
    {

        if (!argument || !*argument)
        {

            writeln(conn, "~CCreate an exit in which direction?~x");

            return;

        }
        long dir = value_lookup(direction_table, argument);

        if (dir == -1)
        {

            writelnf(conn, "~C'%s' is not a valid direction.~x",
                     argument);

            return;

        }
        if (exits[dir] != 0)
        {

            writeln(conn,
                    "~CThere is already an exit in that direction.~x");

            return;

        }
        exits[dir] = new_exit();

        exits[dir]->fromRoom = (Room *) conn->editing->next->data;

        Editor *edit = build_exit_editor(exits[dir]);

        edit->next = conn->editing;

        conn->editing = edit;

        conn->editing->show(conn);

        return;

    }
    if (!str_cmp(arg, "D") || !str_prefix(arg, "delete"))
    {

        if (!argument || !*argument)
        {

            writeln(conn, "~CCreate an exit in which direction?~x");

            return;

        }
        const Lookup *dir;

        int count = 0;

        int num = atoi(argument);

        for (dir = direction_table; dir->name != 0; dir++)
        {

            if (exits[dir->value] != 0)
                count++;

            if ((num != 0 && num == count)
                    || !str_prefix(argument, dir->name))
            {

                break;

            }
        }

        if (dir->name == 0)
        {

            writelnf(conn, "~C'%s' is not a valid direction.~x",
                     argument);

            return;

        }
        if (exits[dir->value] == 0)
        {

            writeln(conn,
                    "~CThere is no exit in that direction.~x");

            return;

        }
        destroy_exit(exits[dir->value]);

        exits[dir->value] = 0;

        conn->editing->show(conn);

        return;

    }
    if (!str_cmp(arg, "E") || !str_prefix(arg, "edit"))
    {

        if (!argument || !*argument)
        {

            writeln(conn,
                    "~CWhich direction do you wish to edit?~x");

            return;

        }
        int count = 0;

        const Lookup *dir;

        int num = atoi(argument);

        for (dir = direction_table; dir->name != 0; dir++)
        {

            if (exits[dir->value] != 0)
                count++;

            if ((num != 0 && num == count)
                    || !str_prefix(argument, dir->name))
            {

                break;

            }
        }

        if (dir->name == 0)
        {

            writelnf(conn, "~C'%s' is not a valid direction.~x",
                     argument);

            return;

        }
        if (exits[dir->value] == 0)
        {

            writeln(conn,
                    "~CThere is no exit in that direction.~x");

            return;

        }
        Editor *ed = build_exit_editor(exits[dir->value]);

        ed->next = conn->editing;

        conn->editing = ed;

        conn->editing->show(conn);

        return;

    }
}

void exit_editor(Client *conn, const char *argument)
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
    Exit *exit = (Exit *) conn->editing->data;

    if (!str_cmp(arg, "A") || !str_cmp(arg, "ToRoom"))
    {

        if (!argument || !*argument || !is_number(argument))
        {

            writeln(conn,
                    "~CPlease specify the id of the room you want to delete.~x");

            return;

        }
        Room *r = get_room_by_id(atoi(argument));

        if (r == 0)
        {

            writeln(conn, "~CNo such room.~x");

            return;

        }
        exit->to.room = r;

        conn->editing->show(conn);

        return;

    }
    if (!str_cmp(arg, "B") || !str_cmp(arg, "flags"))
    {

        if (edit_flag("flags", conn, exit->flags, argument, exit_flags))
            conn->editing->show(conn);

        return;

    }
}
