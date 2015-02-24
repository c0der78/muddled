
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
#include "olc.h"
#include "telnet.h"
#include "client.h"
#include <muddled/help.h>
#include <muddled/string.h>
#include <muddled/lookup.h>
#include <muddled/macro.h>

Editor *build_help_editor(Help * help)
{
    Editor *editor = new_editor();

    editor->data = help;
    editor->edit = help_editor;
    editor->show = help_editor_menu;

    return editor;
}

void help_editor_menu(Client * conn)
{
    clear_screen(conn);

    set_cursor(conn, 1, 1);

    conn->title(conn, "Help Editor");

    Help *help = (Help *) conn->editing->data;

    writelnf(conn, "   ~CId: ~W%d", help->id);

    writelnf(conn, "~YA) ~CKeywords: ~W%s~x", help->keywords);

    string_editor_preview(conn, "~YB) ~CText", help->text);

    writelnf(conn, "~YC) ~CSyntax: ~W%s~x", help->syntax);

    writelnf(conn, "~YD) ~CCategory: ~W%s~x",
             help_categories[help->category].name);

    writelnf(conn, "~YE) ~CRelated: ~W%s~x", help_related_string(help));
}

void help_edit_list(Client * conn)
{
    int count = 0;
    for (Help * help = first_help; help != 0; help = help->next)
    {
        writef(conn, "%2d) %-12.12s ", help->id, help->keywords);
        if (++count % 4 == 0)
            writeln(conn, "");
    }
    if (count % 4 != 0)
        writeln(conn, "");
}

void help_editor(Client * conn, const char *argument)
{
    char arg[100];

    argument = one_argument(argument, arg);

    if (!str_cmp(arg, "Q"))
    {
        finish_editing(conn);
        return;
    }
    if (!str_prefix(arg, "show"))
    {
        conn->editing->show(conn);
        return;
    }
    Help *help = (Help *) conn->editing->data;

    if (!str_cmp(arg, "save"))
    {
        save_help(help);

        writeln(conn, "~CHelp saved.~x");

        return;
    }
    if (!str_cmp(arg, "list"))
    {
        help_edit_list(conn);
        return;
    }
    if (!str_cmp(arg, "A") || !str_cmp(arg, "keywords"))
    {
        if (!argument || !*argument)
        {
            writeln(conn, "~CChange keywords to what?~x");
            return;
        }
        free_str(help->keywords);
        help->keywords = str_dup(argument);

        conn->editing->show(conn);
        return;
    }
    if (!str_cmp(arg, "B") || !str_cmp(arg, "text"))
    {
        Editor *editor = build_string_editor(&help->text);

        editor->next = conn->editing;
        conn->editing = editor;

        conn->editing->show(conn);
        return;
    }
    if (!str_cmp(arg, "C") || !str_cmp(arg, "syntax"))
    {
        free_str(help->syntax);
        help->syntax = str_dup(argument);

        conn->editing->show(conn);
        return;
    }
    if (!str_cmp(arg, "D") || !str_cmp(arg, "category"))
    {
        if (!argument || !*argument || argument[0] == '?')
        {
            writelnf(conn, "~Cvalid categories: ~W%s~x",
                     lookup_names(help_categories));
            return;
        }
        long cat = value_lookup(help_categories, argument);

        if (cat == -1)
        {
            writeln(conn, "~CInvalid category.~x");
            return;
        }
        help->category = (help_category) cat;

        conn->editing->show(conn);

        return;
    }
    if (!str_cmp(arg, "E") || !str_cmp(arg, "related"))
    {
        if (nullstr(argument))
        {
            writeln(conn, "~CYou must provide a help to relate.~x");
            return;
        }
        Help *rel = help_find(argument);

        if (rel == 0)
        {
            writeln(conn, "~CRelated help not found.~x");
            return;
        }
        LINK(help->related, rel, next_related);

        conn->editing->show(conn);
        return;
    }
}
