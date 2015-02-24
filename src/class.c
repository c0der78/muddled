
/******************************************************************************
 *         __  __           _     _         ____  _       _                   *
 *        |  \/  |_   _  __| | __| |_   _  |  _ \| | __ _(_)_ __  ___         *
 *        | |\/| | | | |/ _` |/ _` | | | | | |_) | |/ _` | | '_ \/ __|        *
 *        | |  | | |_| | (_| | (_| | |_| | |  __/| | (_| | | | | \__ \        *
 *        |_|  |_|\__,_|\__,_|\__,_|\__, | |_|   |_|\__,_|_|_| |_|___/        *
 *                                  |___/                                     *
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
#include "muddled/class.h"
#include "muddled/db.h"
#include "muddled/log.h"
#include "muddled/engine.h"
#include "muddled/string.h"
#include "muddled/character.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <inttypes.h>

Class *class_table = 0;
int max_class = 0;

Class *new_class()
{
    Class *c = (Class *) alloc_mem(1, sizeof(Class));

    c->name = str_empty;
    c->description = str_empty;

    return c;
}

void destroy_class(Class *c)
{
    free_str(c->name);
    free_str(c->description);

    free_mem(c);
}

int load_classes()
{
    char buf[400];
    sql_stmt *stmt;
    int total = 0;

    int len = sprintf(buf, "select count(*) from class");

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {
        log_data("could not prepare statement");
        return 0;
    }
    if (sql_step(stmt) == SQL_DONE)
    {
        log_data("could not count hints");
        return 0;
    }
    max_class = sql_column_int(stmt, 0);

    if (sql_finalize(stmt) != SQL_OK)
    {
        log_data("could not finalize statement");
    }
    class_table = (Class *) alloc_mem(max_class, sizeof(Class));

    len = sprintf(buf, "select * from class");

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {
        log_data("could not prepare statement");
        return 0;
    }
    while (sql_step(stmt) != SQL_DONE)
    {
        int count = sql_column_count(stmt);

        // Class * c = new_class();

        for (int i = 0; i < count; i++)
        {
            const char *colname = sql_column_name(stmt, i);

            if (!str_cmp(colname, "name"))
            {
                class_table[total].name =
                    str_dup(sql_column_str(stmt, i));
            }
            else if (!str_cmp(colname, "summary"))
            {
                class_table[total].description =
                    str_dup(sql_column_str(stmt, i));
            }
            else if (!str_cmp(colname, "classId"))
            {
                class_table[total].id = sql_column_int(stmt, i);
            }
            else if (!str_cmp(colname, "fMana"))
            {
                class_table[total].fMana =
                    sql_column_int(stmt, i);
            }
            else if (!str_cmp(colname, "thac0"))
            {
                class_table[total].thac0 =
                    sql_column_int(stmt, i);
            }
            else if (!str_cmp(colname, "thac32"))
            {
                class_table[total].thac32 =
                    sql_column_int(stmt, i);
            }
            else
            {
                log_warn("unknown class column '%s'", colname);
            }
        }

        // LINK(first_class, c, next);
        total++;
    }

    if (sql_finalize(stmt) != SQL_OK)
    {
        log_data("could not finalize statement");
    }
    return total;
}

int get_class_by_id(identifier_t id)
{
    for (int i = 0; i < max_class; i++)
    {
        if (class_table[i].id == id)
            return i;
    }
    return -1;
}

int class_lookup(const char *arg)
{
    if (!arg || !*arg)
        return -1;

    if (is_number(arg))
        return get_class_by_id(atoi(arg));

    for (int i = 0; i < max_class; i++)
    {
        if (!str_prefix(arg, class_table[i].name))
            return i;
    }

    return -1;
}

bool is_valid_class(int index)
{
    return index >= 0 && index < max_class;
}

bool has_class(Character *ch, int classId)
{
    for (int *i = ch->classes; i && *i != -1; i++)
    {
        if (class_table[*i].id == classId)
            return true;
    }
    return false;
}

const char *class_short(Character *ch)
{
    if (ch->classes == 0 || *ch->classes == -1)
    {
        log_warn("char %" PRId64 " with no class", ch->id);
        return "Unknown";
    }
    static char buf[4][100];
    static int i;

    ++i, i %= 4;

    if (ch->classes[1] == -1)
    {
        strcpy(buf[i], capitalize(class_table[*ch->classes].name));
        return buf[i];
    }
    int len = 0;

    for (int *c = ch->classes; c && *c != -1; c++)
    {
        len +=
            sprintf(&buf[i][len], "%.3s/",
                    capitalize(class_table[*c].name));
    }

    if (len > 0)
        buf[i][len - 1] = 0;

    return buf[i];
}

const char *class_who(Character *ch)
{
    static char buf[100];

    if (ch->classes == 0)
    {
        log_warn("char %" PRId64 " with no class", ch->id);
        return "Unk";
    }
    if (ch->classes[1] == -1)
    {
        sprintf(buf, "%.3s",
                capitalize(class_table[*ch->classes].name));
        return buf;
    }
    int count = 0;

    for (int *c = ch->classes; c && *c != -1; c++)
        count++;

    sprintf(buf, "%c+%d", toupper((int)class_table[*ch->classes].name[0]),
            count - 1);

    return buf;
}

int class_count(const Character *ch)
{
    int count = 0;

    for (int *p = ch->classes; p && *p != -1; p++)
        count++;

    return count;
}
