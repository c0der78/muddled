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

#include "client.h"
#include "../room.h"
#include "olc.h"
#include <stdlib.h>
#include <string.h>
#include "../string.h"
#include "../macro.h"
#include "../room.h"
#include "../exit.h"
#include "../lookup.h"
#include "../race.h"
#include "telnet.h"
#include <ctype.h>
#include "../engine.h"

Editor *new_editor()
{

    Editor *editor = (Editor *) alloc_mem(1, sizeof(Editor));

    return editor;

}

void destroy_editor(Editor *editor)
{

    free_mem(editor);

}

void olc_prompt(Client *conn)
{

    if (conn->editing->show == string_editor_menu)
    {

        writelnf(conn, "~CType \\? for help or \\q to %s:~x ",
                 conn->editing->next ? "go back" : "quit");

    }
    else
    {

        writef(conn, "~CSelect an option, ? for help or Q to %s:~x ",
               conn->editing->next ? "go back" : "quit");

    }

}

void finish_editing(Client *conn)
{

    Editor *ed = conn->editing;

    conn->editing = ed->next;

    destroy_editor(ed);

    clear_screen(conn);

    set_cursor(conn, 1, 1);

    if (conn->editing != 0)
        conn->editing->show(conn);

}

void string_editor_menu(Client *conn)
{

    const char **pStr = (const char **)conn->editing->data;

    const char *str = *pStr;

    char buf[ARG_SIZ];

    int count = 0;

    clear_screen(conn);

    set_cursor(conn, 1, 1);

    conn->title(conn, "Text Editor");

    while (str && *str != 0)
    {

        str = get_line(str, buf);

        writelnf(conn, "~Y%2d)~x %s", ++count, buf);

    }

}

void string_editor_preview(Client *conn, const char *header, const char *pstr)
{

    int len = strlen_color(header);

    char buf[ARG_SIZ];

    int count = 0;

    if (pstr && *pstr)
    {

        pstr = get_line(pstr, buf);

        writelnf(conn, "%s: ~W%2d. %s~x", header, ++count, buf);

        while (pstr && *pstr != 0 && count < 3)
        {

            pstr = get_line(pstr, buf);

            writelnf(conn, "%-*s  ~W%2d. %s~x", len, " ", ++count,
                     buf);

        }

    }
    else
    {

        writelnf(conn, "%s: ~WEmpty~x", header);

    }

}

void string_linedel(const char *string, size_t line, char *buf)
{

    size_t cnt = 1;

    for (const char *strtmp = string; strtmp && *strtmp != 0; strtmp++)
    {

        if (cnt != line)
            *buf++ = *strtmp;

        if (*strtmp == '\n')
        {

            if (*(strtmp + 1) == '\r')
            {

                strtmp++;

                if (cnt != line)
                    *buf++ = *strtmp;

            }
            cnt++;

        }
    }

    *buf = 0;

}

void
string_lineadd(const char *string, const char *newstr, size_t line, char *buf)
{

    size_t cnt = 1;

    bool done = false;

    buf = 0;

    for (const char *strtmp = string;
            *strtmp != 0 || (!done && cnt == line); strtmp++)
    {

        if (cnt == line && !done)
        {

            strcat(buf, newstr);

            strcat(buf, "\n\r");

            cnt++;

            done = true;

        }
        *buf++ = *strtmp;

        if (done && *strtmp == 0)
            break;

        if (*strtmp == '\n')
        {

            if (*(strtmp + 1) == '\r')
                *buf++ = *(++strtmp);

            cnt++;

        }
    }

    *buf = 0;

}

void olc_syntax(Client *conn, const char *arg, ...)
{

    va_list args;

    char *str;

    int i;

    if (!arg || !*arg)
        return;

    writeln(conn, "~CCommands:~x");

    const char *const defaults[] =
    { "quit", "show", "save", "list", "delete" };

    for (i = 0; i < sizeof(defaults) / sizeof(defaults[0]); i++)
        writelnf(conn, "%10s ", defaults[i]);

    va_start(args, arg);

    str = va_arg(args, char *);

    if (str == NULL)
        return;

    i++;

    writef(conn, "%10s ", arg);

    while ((str = va_arg(args, char *)) != NULL)
    {

        writef(conn, "%10s ", str);

        if (++i % 5 == 0)
            writeln(conn, "");

    }

    va_end(args);

    if (i % 5 != 0)
        writeln(conn, "");

}

int edit_text(Client *conn, const char **pStr, const char *argument)
{
    char buf[OUT_SIZ * 4];

    if (!str_cmp(argument, "\\?"))
    {

        writeln(conn, "~CText Editor Help:");

        writeln(conn, "~Y\\?~C		- this help");

        writeln(conn, "~Y\\q~C		- exit editor");

        writeln(conn, "~Y\\c~C		- clear all lines");

        writeln(conn, "~Y\\d~C		- delete last line");

        writeln(conn, "~Y\\d#~C		- delete line #");

        writeln(conn, "~Y\\i#~C		- insert a line at #~x");

        return EDIT_NOCHANGE;

    }
    if (!str_cmp(argument, "\\q"))
    {

        return EDIT_END;

    }

    if (!str_prefix("\\d", argument))
    {

        argument += 2;

        int line, lines = count_lines(*pStr);

        if (*argument && is_number(argument))
        {

            line = atoi(argument);

            if (line < 1 || line > lines)
            {

                writelnf(conn,
                         "~CYou can't delete line %d, it doesn't exist.~x",
                         line);

                return EDIT_NOCHANGE;

            }
        }
        else
        {

            line = lines;

        }

        string_linedel(*pStr, line, buf);

        free_str(*pStr);

        *pStr = str_dup(buf);

        return EDIT_CHANGED;

    }
    if (!str_cmp("\\c", argument))
    {

        free_str(*pStr);

        *pStr = str_empty;

        return EDIT_CHANGED;

    }
    if (!str_prefix("\\i", argument))
    {

        argument += 2;

        if (!argument || !*argument || !is_number(argument))
        {

            writeln(conn,
                    "~CYou must specify a line to insert at.~x");

            return EDIT_NOCHANGE;

        }
        int line = atoi(argument);

        if (line < 1 || line > count_lines(*pStr))
        {

            writeln(conn, "~CThat is not a valid insert point.~x");

            return EDIT_NOCHANGE;

        }
        string_lineadd(*pStr, argument, line, buf);

        free_str(*pStr);

        *pStr = str_dup(buf);

        return EDIT_CHANGED;

    }
    if (*pStr)
        strcpy(buf, *pStr);

    else
        buf[0] = 0;

    strcat(buf, argument);

    strcat(buf, "\n\r");

    if (*pStr)
        free_str(*pStr);

    *pStr = str_dup(buf);

    return EDIT_CHANGED;

}

void string_edit(Client *conn, const char *argument)
{

    int rval =
        edit_text(conn, (const char **)conn->editing->data, argument);

    switch (rval)
    {

    default:

    case EDIT_NOCHANGE:

        break;

    case EDIT_CHANGED:

        conn->editing->show(conn);

        break;

    case EDIT_END:

        finish_editing(conn);

        break;

    }

}

Editor *build_string_editor(const char **pStr)
{

    Editor *editor = new_editor();

    editor->edit = string_edit;

    editor->data = pStr;

    editor->show = string_editor_menu;

    return editor;

}

void character_editor_menu(Client *conn, Character *ch)
{

    writelnf(conn, "~C   Id: ~W%d", ch->id);

    writelnf(conn, "~YA) ~CName: ~W%s~x", ch->name);

    writelnf(conn, "~YB) ~CSex: ~W%s~x", sex_table[ch->sex].name);

    writelnf(conn, "~YC) ~CLevel: ~W%d~x", ch->level);

    string_editor_preview(conn, "~YD) ~CDescription", ch->description);

    writelnf(conn, "~YE) ~CRace: ~W%s~x", capitalize(ch->race->name));

    writelnf(conn, "~YF) ~CGold: ~W%.2f~x", ch->gold);

}

int
character_editor(Client *conn, Character *ch, const char *arg,
                 const char *argument)
{

    if (!str_cmp(arg, "A") || !str_cmp(arg, "name"))
    {

        if (!argument || !*argument)
        {

            writeln(conn, "~CYou must provide a name to set.~x");

            return 1;

        }
        free_str_dup(&ch->name, argument);

        conn->editing->show(conn);

        return 1;

    }
    if (!str_cmp(arg, "D") || !str_cmp(arg, "description"))
    {

        Editor *editor = build_string_editor(&ch->description);

        editor->next = conn->editing;

        conn->editing = editor;

        conn->editing->show(conn);

        return 1;

    }
    if (!str_cmp(arg, "B") || !str_cmp(arg, "sex"))
    {

        if (!argument || !*argument || argument[0] == '?')
        {

            writelnf(conn, "~CValid sexes are: ~W%s~x",
                     lookup_names(sex_table));

            return 1;

        }
        long s = value_lookup(sex_table, argument);

        if (s == -1)
        {

            writeln(conn, "~CNo such sex.~x");

            return 1;

        }
        ch->sex = (sex_t) s;

        conn->editing->show(conn);

        return 1;

    }
    if (!str_cmp(arg, "C") || !str_cmp(arg, "level"))
    {

        if (!argument || !*argument)
        {

            writeln(conn, "~CYou must specify a level to set.~x");

            return 1;

        }
        int l = atoi(argument);

        if (l == 0 || l >= MAX_LEVEL * 2)
        {

            writelnf(conn, "~CA valid level is between 1 and %d.~x",
                     MAX_LEVEL * 2);

            return 1;

        }
        ch->level = l;

        conn->editing->show(conn);

        return 1;

    }
    if (!str_cmp(arg, "E") || !str_cmp(arg, "race"))
    {

        if (!argument || !*argument)
        {

            writeln(conn, "~CYou must specify a race to set.~x");

            return 1;

        }
        Race *r = race_lookup(argument);

        if (r == 0)
        {

            writeln(conn, "~CInvalid race.~x");

            return 1;

        }
        ch->race = r;

        conn->editing->show(conn);

        return 1;

    }
    if (!str_cmp(arg, "F") || !str_cmp(arg, "gold"))
    {

        if (nullstr(argument) || !is_number(argument))
        {

            writeln(conn,
                    "~CYou must specify a decimal number to set.~x");

            return 1;

        }
        ch->gold = atof(argument);

        conn->editing->show(conn);

        return 1;

    }
    return 0;

}

int
edit_flag(const char *arg, Client *conn, Flag *flags,
          const char *argument, const Lookup *table)
{

    if (nullstr(argument) || argument[0] == '?')
    {

        writelnf(conn, "~CValid flags are: ~W%s~x",
                 lookup_names(table));

        return EDIT_NOCHANGE;

    }
    int res = parse_flags_toggle(flags, argument, table);

    if (res == 0)
    {

        writelnf(conn, "~CNo such flag.  See '%s ?' for a list.", arg);

        return EDIT_NOCHANGE;

    }
    return EDIT_CHANGED;

}

int
edit_string(const char *arg, Client *conn, const char **str,
            const char *argument)
{

    if (nullstr(argument) || argument[0] == '?')
    {

        writeln(conn, "~CYou must provide an argument.~x");

        return EDIT_NOCHANGE;

    }
    free_str_dup(str, argument);

    conn->editing->show(conn);

    return EDIT_CHANGED;

}
