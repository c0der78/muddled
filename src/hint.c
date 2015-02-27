
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
#include "hint.h"
#include "engine.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "str.h"
#include "log.h"
#include "macro.h"

Hint *hint_table = 0;

int max_hint = 0;

Hint *new_hint()
{

    Hint *hint = (Hint *) alloc_mem(1, sizeof(Hint));

    hint->text = str_empty;

    return hint;

}

void destroy_hint(Hint *hint)
{

    free_str(hint->text);

    free_mem(hint);

}

int load_hints()
{

    char buf[400];

    sql_stmt *stmt;

    int total = 0;

    int len = sprintf(buf, "select count(*) from hint");

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
    max_hint = sql_column_int(stmt, 0);

    if (sql_finalize(stmt) != SQL_OK)
    {

        log_data("could not finalize statement");

    }
    hint_table = (Hint *) alloc_mem(max_hint, sizeof(Hint));

    len = sprintf(buf, "select * from hint");

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {

        log_data("could not prepare statement");

        return 0;

    }
    while (sql_step(stmt) != SQL_DONE)
    {

        int count = sql_column_count(stmt);

        for (int i = 0; i < count; i++)
        {

            const char *colname = sql_column_name(stmt, i);

            if (!str_cmp(colname, "text"))
            {

                hint_table[total].text =
                    str_dup(sql_column_str(stmt, i));

            }
            else if (!str_cmp(colname, "hintId"))
            {

                hint_table[total].id = sql_column_int(stmt, i);

            }
            else if (!str_cmp(colname, "level"))
            {

                hint_table[total].level =
                    sql_column_int(stmt, i);

            }
            else
            {

                log_warn("unknown hint column '%s'", colname);

            }

        }

        total++;

    }

    if (sql_finalize(stmt) != SQL_OK)
    {

        log_data("could not finalize statement");

    }
    if (total != max_hint)
    {

        log_warn("counted hints did not match number read");

    }
    return total;

}

int save_hint(Hint *hint)
{
    field_map hint_values[] =
    {
        {"text", &hint->text, SQL_TEXT},
        {"level", &hint->level, SQL_INT},
        {0}
    };

    if (hint->id == 0)
    {
        if (sql_insert_query(hint_values, "hint") != SQL_OK)
        {

            log_data("could not insert hint");

            return 0;

        }
        hint->id = db_last_insert_rowid();

    }
    else
    {

        if (sql_update_query(hint_values, "hint", hint->id) != SQL_OK)
        {

            log_data("could not update hint");

            return 0;

        }
    }

    return 1;

}
