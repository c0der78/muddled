
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

#include "skill.h"
#include "db.h"
#include "string.h"
#include "spellfun.h"
#include "engine.h"
#include "log.h"
#include "class.h"
#include "character.h"
#include "lookup.h"
#include "affect.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

Skill *skill_table = 0;

int max_skill = 0;

#define DEFINE_GSNS
#include "gsn.h"
#undef DEFINE_GSNS

const struct gsn_type gsn_table[] =
{
#define IN_GSN_TABLE
#include "gsn.h"
#undef IN_GSN_TABLE
    {0, 0}
};

const struct spellfun_type spellfun_table[] =
{
#define IN_SPELL_TABLE
#include "spellfun.h"
#undef IN_SPELL_TABLE
    {0, 0}
};

const Lookup skill_flags[] =
{
    {0, 0}
};

Skill *new_skill()
{

    Skill *skill = (Skill *) alloc_mem(1, sizeof(Skill));

    skill->name = str_empty;

    skill->damage = str_empty;

    skill->msgObj = str_empty;

    skill->msgOff = str_empty;

    return skill;

}

void destroy_skill(Skill *skill)
{

    free_str(skill->name);

    free_str(skill->damage);

    free_str(skill->msgOff);

    free_str(skill->msgObj);

    free_mem(skill);

}

const char *gsn_name(int *pgsn)
{

    if (!pgsn)
        return 0;

    for (int i = 0; gsn_table[i].name != 0; i++)
        if (*gsn_table[i].pgsn == *pgsn)
            return gsn_table[i].name;

    return 0;

}

int *gsn_lookup(const char *name)
{

    if (nullstr(name))
        return 0;

    for (int i = 0; gsn_table[i].name != 0; i++)
        if (!str_cmp(name, gsn_table[i].name))
            return gsn_table[i].pgsn;

    return 0;

}

Skill *skill_lookup(const char *arg)
{

    if (nullstr(arg))
        return 0;

    for (int i = 0; i < max_skill; i++)
        if (!str_prefix(arg, skill_table[i].name))
            return &skill_table[i];

    return 0;

}

const char *spellfun_name(SpellFun *fun)
{

    if (!fun)
        return 0;

    for (int i = 0; spellfun_table[i].name != 0; i++)
        if (spellfun_table[i].fun == fun)
            return spellfun_table[i].name;

    return 0;

}

SpellFun *spellfun_lookup(const char *name)
{

    if (nullstr(name))
        return 0;

    for (int i = 0; spellfun_table[i].name != 0; i++)
        if (!str_cmp(spellfun_table[i].name, name))
            return spellfun_table[i].fun;

    return 0;

}

int load_skill_levels(Skill *skill)
{

    char buf[400];

    sql_stmt *stmt;

    int total = 0;

    int len =
        sprintf(buf, "select * from skill_level where skillId=%" PRId64,
                skill->id);

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {

        log_data("could not prepare statement");

        return 0;

    }
    skill->levels = (int *)alloc_mem(max_class, sizeof(int));

    while (sql_step(stmt) != SQL_DONE)
    {

        int count = sql_column_count(stmt);

        int classId = 0;

        short level = -1;

        int index;

        for (int i = 0; i < count; i++)
        {

            const char *colname = sql_column_name(stmt, i);

            if (!str_cmp(colname, "skillId"))
            {

                if (sql_column_int(stmt, i) != skill->id)
                    log_error
                    ("load_skill_levels: sql returned invalid skill");

            }
            else if (!str_cmp(colname, "classId"))
            {

                classId = sql_column_int(stmt, i);

            }
            else if (!str_cmp(colname, "level"))
            {

                level = sql_column_int(stmt, i);

            }
            else
            {

                log_error("unknown skill level column '%s'",
                          colname);

            }

        }

        for (index = 0; index < max_class; index++)
            if (class_table[index].id == classId)
                break;

        if (index != max_class)
            skill->levels[index] = level;

        total++;

    }

    if (sql_finalize(stmt) != SQL_OK)
    {
        log_data("unable to finalize skill sql statement");
    }
    return total;

}

bool valid_skill(identifier_t sn)
{

    return sn >= 0 && sn < max_skill;

}

int load_skills()
{

    char buf[400];

    sql_stmt *stmt;

    int total = 0;

    int len = sprintf(buf, "select count(*) from skill");

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {

        log_data("could not prepare statement");

        return 0;

    }
    if (sql_step(stmt) == SQL_DONE)
    {

        log_data("could not count skills");

        return 0;

    }
    max_skill = sql_column_int(stmt, 0);

    if (sql_finalize(stmt) != SQL_OK)
    {

        log_data("could not finalize statement");

    }
    skill_table = (Skill *) alloc_mem(max_skill, sizeof(Skill));

    len = sprintf(buf, "select * from skill");

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

            if (!str_cmp(colname, "name"))
            {

                skill_table[total].name =
                    str_dup(sql_column_str(stmt, i));

            }
            else if (!str_cmp(colname, "skillId"))
            {

                skill_table[total].id = sql_column_int(stmt, i);

            }
            else if (!str_cmp(colname, "msgOff"))
            {

                skill_table[total].msgOff =
                    str_dup(sql_column_str(stmt, i));

            }
            else if (!str_cmp(colname, "msgObj"))
            {

                skill_table[total].msgObj =
                    str_dup(sql_column_str(stmt, i));

            }
            else if (!str_cmp(colname, "mana"))
            {

                skill_table[total].mana =
                    sql_column_int(stmt, i);

            }
            else if (!str_cmp(colname, "wait"))
            {

                skill_table[total].wait =
                    sql_column_int(stmt, i);

            }
            else if (!str_cmp(colname, "cost"))
            {

                skill_table[total].cost =
                    sqlite3_column_double(stmt, i);

            }
            else if (!str_cmp(colname, "damage"))
            {

                skill_table[total].damage =
                    str_dup(sql_column_str(stmt, i));

            }
            else if (!str_cmp(colname, "flags"))
            {

                parse_flags(&skill_table[total].flags,
                            sql_column_str(stmt, i),
                            skill_flags);

            }
            else if (!str_cmp(colname, "minPos"))
            {

                skill_table[total].minPos =
                    sql_column_int(stmt, i);

            }
            else if (!str_cmp(colname, "spell"))
            {

                skill_table[total].spellfun =
                    spellfun_lookup(sql_column_str(stmt, i));

            }
            else if (!str_cmp(colname, "gsn"))
            {

                skill_table[total].pgsn =
                    gsn_lookup(sql_column_str(stmt, i));

                if (skill_table[total].pgsn != 0)
                    *skill_table[total].pgsn = total;

            }
            else
            {

                log_warn("unknown skill column '%s'", colname);

            }

        }

        load_skill_levels(&skill_table[total]);

        total++;

    }

    if (sql_finalize(stmt) != SQL_OK)
    {

        log_data("could not finalize statement");

    }
    if (total != max_skill)
    {

        log_warn("counted skills did not match number read");

    }
    return total;

}

static int save_gsn(sql_stmt *stmt, int index, const field_map *table)
{

    const char *name = gsn_name(*((int **)table->value));

    return sql_bind_text(stmt, index, name, strlen(name), 0);

}

static int save_skillspell(sql_stmt *stmt, int index, const field_map *table)
{

    const char *name = spellfun_name(*((SpellFun **) table->value));

    return sql_bind_text(stmt, index, name, strlen(name), 0);

}

int save_skill(Skill *skill)
{
    field_map skill_values[] =
    {
        {"name", &skill->name, SQL_TEXT},
        {"damage", &skill->damage, SQL_TEXT},
        {"msgOff", &skill->msgOff, SQL_TEXT},
        {"msgObj", &skill->msgObj, SQL_TEXT},
        {"cost", &skill->cost, SQL_DOUBLE},
        {"minPos", &skill->minPos, SQL_INT},
        {"mana", &skill->mana, SQL_INT},
        {"wait", &skill->wait, SQL_INT},
        {"gsn", &skill->pgsn, SQL_CUSTOM,  NULL, NULL, 0, save_gsn},
        {"spell", &skill->spellfun, SQL_CUSTOM, NULL, NULL, 0, save_skillspell},
        {"flags", &skill->flags, SQL_FLAG, skill_flags},
        {0}
    };

    if (skill->id == 0)
    {
        if (sql_insert_query(skill_values, "skill") != SQL_OK)
        {
            log_data("could not insert skill");
            return 0;
        }
        skill->id = db_last_insert_rowid();

    }
    else
    {
        if (sql_update_query(skill_values, "skill", skill->id) !=
                SQL_OK)
        {
            log_data("could not update skill");
            return 0;
        }
    }

    return 1;

}

SPELL(magic_missile)
{

    writeln(ch, "Got magic missile");

}

SPELL(energy_shield)
{

    if (is_affected(ch, sn))
    {

        writeln(ch, "You already surrounded with an energy shield.");

        return;

    }
    Affect *paf = new_affect();

    paf->from = sn;

    paf->level = ch->level;

    paf->duration = 24;

    paf->modifier = 20;

    paf->callback = &affect_apply_resists;

    affect_to_char(ch, paf);

    writeln(ch, "You surround yourself with an energy shield.");

    act(TO_ROOM, ch, 0, 0, "$n surrounds $mself with an energy shield.");

}
