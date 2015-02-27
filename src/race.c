
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
#include "muddled/flag.h"
#include "muddled/string.h"
#include "muddled/engine.h"
#include "muddled/macro.h"
#include "muddled/log.h"
#include "muddled/lookup.h"
#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include "muddled/race.h"

Race *first_race = 0;

const Lookup race_flags[] =
{
    {"pcrace", RACE_PC},
    {0, 0}
};

Race *get_race_by_id(identifier_t id)
{

    for (Race *r = first_race; r != 0; r = r->next)
    {
        if (r->id == id)
            return r;
    }
    return 0;
}

Race *race_lookup(const char *arg)
{
    if (!arg || !*arg)
        return 0;

    if (is_number(arg))
    {
        return get_race_by_id(atoi(arg));
    }
    for (Race *r = first_race; r != 0; r = r->next)
    {
        if (!str_prefix(arg, r->name))
            return r;
    }
    return 0;
}

Race *new_race()
{
    Race *race = (Race *) alloc_mem(1, sizeof(Race));

    race->name = str_empty;
    race->description = str_empty;
    race->flags = new_flag();
    memset(race->stats, 13, MAX_STAT * sizeof(int));

    return race;
}

void destroy_race(Race *race)
{
    free_str(race->name);
    free_str(race->description);

    free_mem(race);
}

int load_races()
{
    char buf[400];
    sql_stmt *stmt;
    int total = 0;

    int len = sprintf(buf, "select * from race");

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {
        log_data("could not prepare statement");
        return 0;
    }
    while (sql_step(stmt) != SQL_DONE)
    {
        int count = sql_column_count(stmt);

        Race *race = new_race();

        for (int i = 0; i < count; i++)
        {
            const char *colname = sql_column_name(stmt, i);

            if (!str_cmp(colname, "name"))
            {
                race->name = str_dup(sql_column_str(stmt, i));
            }
            else if (!str_cmp(colname, "summary"))
            {
                race->description =
                    str_dup(sql_column_str(stmt, i));
            }
            else if (!str_cmp(colname, "raceId"))
            {
                race->id = sql_column_int(stmt, i);
            }
            else if (!str_cmp(colname, "stats"))
            {
                db_read_int_array(MAX_STAT, &race->stats, stmt,
                                  i);
            }
            else if (!str_cmp(colname, "statMods"))
            {
                db_read_int_array(MAX_STAT, &race->statMods,
                                  stmt, i);
            }
            else if (!str_cmp(colname, "flags"))
            {
                parse_flags(race->flags,
                            sql_column_str(stmt, i),
                            race_flags);
            }
            else
            {
                log_warn("unknown race column '%s'", colname);
            }
        }

        LINK(first_race, race, next);
        total++;
    }

    if (sql_finalize(stmt) != SQL_OK)
    {
        log_data("could not finalize statement");
    }
    return total;
}

int save_race(Race *race)
{
    field_map race_values[] =
    {
        {"name", &race->name, SQL_TEXT},
        {"sumary", &race->description, SQL_TEXT},
        {"flags", &race->flags, SQL_FLAG, race_flags},
        {
            "stats", &race->stats, SQL_ARRAY, db_save_int_array,
            (void *)MAX_STAT
        },
        {
            "statMods", &race->statMods, SQL_ARRAY, db_save_int_array,
            (void *)MAX_STAT
        },
        {0}
    };

    if (race->id == 0)
    {
        if (sql_insert_query(race_values, "race") != SQL_OK)
        {
            log_data("could not insert race");
            return 0;
        }
        race->id = db_last_insert_rowid();

    }
    else
    {
        if (sql_update_query(race_values, "race", race->id) != SQL_OK)
        {
            log_data("could not update race");
            return 0;
        }
    }

    return 1;
}
