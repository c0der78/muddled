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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <inttypes.h>
#include "engine.h"
#include "str.h"
#include "character.h"
#include "object.h"
#include "log.h"
#include "db.h"
#include "player.h"
#include "nonplayer.h"
#include "connection.h"
#include "macro.h"
#include "flag.h"
#include "room.h"
#include "race.h"
#include "util.h"
#include "account.h"
#include "class.h"
#include "affect.h"
#include "lookup.h"
#include "private.h"

Character *first_character = 0;

const Lookup sex_table[] =
{
    {"neutral", SEX_NEUTRAL},
    {"male", SEX_MALE},
    {"female", SEX_FEMALE},
    {"hermaphrodite", SEX_HERMAPHRODITE},
    {0, 0}
};

const Lookup position_table[] =
{
    {"fighting", POS_FIGHTING},
    {"standing", POS_STANDING},
    {"sitting", POS_SITTING},
    {"resting", POS_RESTING},
    {"stunned", POS_STUNNED},
    {"incapacitated", POS_INCAPICATED},
    {"mortally wounded", POS_MORTAL},
    {"dead", POS_DEAD},
    {0, 0}
};

static void writef_line_to_char(const Character *ch, const char *fmt, ...)
{
    va_list args;

    if (ch->pc == 0 || ch->pc->conn == 0)
        return;

    va_start(args, fmt);
    ch->pc->conn->vwrite(ch->pc->conn, fmt, args);
    va_end(args);

    ch->pc->conn->writeln(ch->pc->conn, "");
}

static void write_line_to_char(const Character *ch, const char *arg)
{
    if (ch->pc == 0)
        return;

    xwriteln(ch->pc->conn, arg);
}

static void writef_to_char(const Character *ch, const char *fmt, ...)
{
    va_list args;

    if (ch->pc == 0 || ch->pc->conn == 0)
        return;

    va_start(args, fmt);
    ch->pc->conn->vwrite(ch->pc->conn, fmt, args);
    va_end(args);
}

static void write_to_char(const Character *ch, const char *arg)
{
    if (ch->pc == 0 || ch->pc->conn == 0)
        return;

    ch->pc->conn->writeln(ch->pc->conn, arg);
}

static void page_to_char(Character *ch, const char *arg)
{
    if (!ch || !ch->pc || !ch->pc->conn)
        return;

    ch->pc->conn->page(ch->pc->conn, arg);
}

static void titlef_to_char(const Character *ch, const char *fmt, ...)
{
    va_list args;

    if (ch->pc == 0 || !ch->pc->conn)
        return;

    va_start(args, fmt);
    ch->pc->conn->vtitle(ch->pc->conn, fmt, args);
    va_end(args);
}

static void title_to_char(const Character *ch, const char *arg)
{
    if (ch->pc == 0 || !ch->pc->conn)
        return;

    ch->pc->conn->title(ch->pc->conn, arg);
}

Character *new_char()
{
    Character *ch = (Character *) alloc_mem(1, sizeof(Character));

    ch->flags = new_flag();
    ch->affectedBy = new_flag();
    ch->next = 0;
    ch->next_in_area = 0;
    ch->name = str_empty;
    ch->position = POS_STANDING;
    ch->sex = SEX_NEUTRAL;
    ch->description = str_empty;
    ch->hit = ch->maxHit = 100;
    ch->mana = ch->maxMana = 100;
    ch->move = ch->maxMove = 100;
    ch->writelnf = writef_line_to_char;
    ch->writef = writef_to_char;
    ch->writeln = write_line_to_char;
    ch->write = write_to_char;
    ch->page = page_to_char;
    ch->title = title_to_char;
    ch->titlef = titlef_to_char;
    ch->classes = alloc_mem(1, sizeof(int));
    ch->classes[0] = -1;
    ch->size = SIZE_AVERAGE;
    for (int i = 0; i < MAX_DAM; i++)
        ch->resists[i] = 100;

    return ch;
}

void destroy_char(Character *ch)
{
    free_str(ch->name);

    destroy_flags(ch->flags);

    destroy_flags(ch->affectedBy);

    free_str(ch->description);

    free_mem(ch->classes);

    if (ch->pc != 0)
    {
        destroy_player(ch->pc);
    }
    if (ch->npc != 0)
    {
        destroy_npc(ch->npc);
    }
    free_mem(ch);
}

void extract_char(Character *ch, bool fPull)
{

    for (Object * obj_next, *obj = ch->carrying; obj != NULL;
            obj = obj_next)
    {
        obj_next = obj->next_content;
        extract_obj(obj);
    }

    if (ch->inRoom != NULL)
        char_from_room(ch);

    /*
     * Death room is set in the clan tabe now
     */
    if (!fPull)
    {
        char_to_room(ch, get_room_by_id(DEFAULT_ROOM));
        return;
    }
    UNLINK(first_character, Character, ch, next);

    if (ch->pc)
        UNLINK(first_player, Character, ch, next_player);

    destroy_char(ch);
}

int load_char_objs(Character *ch)
{
    char buf[400];
    sql_stmt *stmt;
    int total = 0;

    int len = sprintf(buf,
                      "select * from char_objects where carriedById=%"
                      PRId64,
                      ch->id);

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {
        log_data("could not prepare statement");
        return 0;
    }
    while (sql_step(stmt) != SQL_DONE)
    {

        Object *obj = new_object();

        obj->carriedBy = ch;

        load_obj_columns(obj, stmt);

        LINK(obj->area->objects, obj, next_in_area);
        LINK(first_object, obj, next);
        if (!obj->inObj)
        {
            LINK(ch->carrying, obj, next_content);
            if (obj->wearLoc != WEAR_NONE)
            {
                equip_char(ch, obj, obj->wearLoc);
            }
        }
        else
        {
            LINK(obj->inObj->contains, obj, next_content);
        }

        total++;
    }

    if (sql_finalize(stmt) != SQL_OK)
    {
        log_data("could not finalize statement");
    }
    return total;
}

static int save_char_classes(sql_stmt *stmt, int index, const field_map *table)
{
    int *data = *((int **)table->value);

    static char buf[100];
    int len = 0;

    for (int i = 0; data[i] != -1; i++)
    {
        len += sprintf(&buf[len], "%d,", data[i]);
    }

    if (len > 0)
        buf[len - 1] = 0;

    return sql_bind_text(stmt, index, buf, len, 0);
}

static int read_char_classes(void *arg, sql_stmt *stmt, int i)
{
    const char *pstr = strtok((char *)sql_column_str(stmt, i), ",");
    int total = 0;
    int **data = (int **)arg;

    while (pstr)
    {

        int c = is_number(pstr) ? atoi(pstr) : -1;

        if (is_valid_class(c))
        {
            *data = (int *)realloc(*data, ++total * sizeof(int));
            (*data)[total - 1] = c;
        }
        pstr = strtok(NULL, ",");
    }

    *data = (int *)realloc(*data, (total + 1) * sizeof(int));
    (*data)[total] = -1;

    return total;
}

int
load_char_column(Character *ch, sql_stmt *stmt, const char *colname, int i)
{
    if (!str_cmp(colname, "name"))
    {
        ch->name = str_dup(sql_column_str(stmt, i));
        return 1;
    }
    else if (!str_cmp(colname, "version"))
    {
        ch->version = sql_column_int(stmt, i);
        return 1;
    }
    else if (!str_cmp(colname, "description"))
    {
        ch->description = str_dup(sql_column_str(stmt, i));
        return 1;
    }
    else if (!str_cmp(colname, "level"))
    {
        ch->level = sql_column_int(stmt, i);
        return 1;
    }
    else if (!str_cmp(colname, "characterId"))
    {
        ch->id = sql_column_int(stmt, i);
        return 1;
    }
    else if (!str_cmp(colname, "position"))
    {
        ch->position = sql_column_int(stmt, i);
        return 1;
    }
    else if (!str_cmp(colname, "hit"))
    {
        ch->hit = sql_column_int(stmt, i);
        return 1;
    }
    else if (!str_cmp(colname, "maxHit"))
    {
        ch->maxHit = sql_column_int(stmt, i);
        return 1;
    }
    else if (!str_cmp(colname, "move"))
    {
        ch->move = sql_column_int(stmt, i);
        return 1;
    }
    else if (!str_cmp(colname, "maxMove"))
    {
        ch->maxMove = sql_column_int(stmt, i);
        return 1;
    }
    else if (!str_cmp(colname, "mana"))
    {
        ch->mana = sql_column_int(stmt, i);
        return 1;
    }
    else if (!str_cmp(colname, "maxMana"))
    {
        ch->maxMana = sql_column_int(stmt, i);
        return 1;
    }
    else if (!str_cmp(colname, "sex"))
    {
        ch->sex = sql_column_int(stmt, i);
        return 1;
    }
    else if (!str_cmp(colname, "gold"))
    {
        ch->gold = sqlite3_column_double(stmt, i);
        return 1;
    }
    else if (!str_cmp(colname, "raceId"))
    {
        ch->race = get_race_by_id(sql_column_int(stmt, i));
        return 1;
    }
    else if (!str_cmp(colname, "classes"))
    {
        read_char_classes(&ch->classes, stmt, i);
        return 1;
    }
    else if (!str_cmp(colname, "stats"))
    {
        db_read_int_array(MAX_STAT, &ch->stats, stmt, i);
        return 1;
    }
    else if (!str_cmp(colname, "resists"))
    {
        db_read_int_array(MAX_STAT, &ch->resists, stmt, i);
        return 1;
    }
    else if (!str_cmp(colname, "alignment"))
    {
        ch->alignment = sql_column_int(stmt, i);
        return 1;
    }
    else if (!str_cmp(colname, "size"))
    {
        ch->size = (float)sqlite3_column_double(stmt, i);
        return 1;
    }
    return 0;
}

int delete_character(Character *ch)
{
    char buf[BUF_SIZ];

    sprintf(buf, "delete from character where characterId=%" PRId64,
            ch->id);

    if (sql_exec(buf) != SQL_OK)
    {
        log_data("could not delete character");
        return 0;
    }
    return 1;
}

int save_char_objs(Character *ch)
{
    char buf[400];
    sql_stmt *stmt;
    int len;
    Object *obj;

    for (obj = ch->carrying; obj; obj = obj->next_content)
    {
        len =
            sprintf(buf,
                    "select objectId from char_objects where carriedById=%"
                    PRId64 " and objectId=%" PRId64, ch->id, obj->id);

        if (sql_query(buf, len, &stmt) != SQL_OK)
        {
            log_data("could not prepare statement");
            return 0;
        }
        bool update = sql_step(stmt) != SQL_DONE;

        if (sql_finalize(stmt) != SQL_OK)
        {
            log_data("unable to finalize sql statement");
            return 0;
        }
        if (!update)
        {
            obj->id = 0;
        }
        if (!save_object(obj))
            return 0;
    }

    // try and cleanup old objects

    len =
        sprintf(buf,
                "select * from char_objects where carriedById=%" PRId64,
                ch->id);

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {
        log_data("could not prepare statement");
        return 0;
    }
    while (sql_step(stmt) != SQL_DONE)
    {
        obj = ch->carrying;

        identifier_t id = sql_column_int64(stmt, 0);

        while (obj)
        {
            if (obj->id == id)
                break;
            obj = obj->next;
        }

        if (!obj)
        {
            sprintf(buf,
                    "delete from object where carriedById=%" PRId64
                    " and objectId=%" PRId64, ch->id, id);

            if (sql_exec(buf) != SQL_OK)
            {
                log_data("could not delete character object %"
                         PRId64, id);
                return 0;
            }
        }
    }

    if (sql_finalize(stmt) != SQL_OK)
    {
        log_data("could not finalize statement");
        return 0;
    }
    return 1;
}

int save_character(Character *ch, const Lookup *flag_table)
{
    static const int CharSaveVersion = 1;

    const int maxStat = MAX_STAT;
    const int maxDam = MAX_DAM;

    field_map char_values[] =
    {
        {"version", &CharSaveVersion, SQL_INT},
        {"name", &ch->name, SQL_TEXT},
        {"description", &ch->description, SQL_TEXT},
        {"level", &ch->level, SQL_INT},
        {"sex", &ch->sex, SQL_LOOKUP, sex_table},
        {"raceId", &ch->race->id, SQL_INT},
        {"hit", &ch->hit, SQL_INT},
        {"maxHit", &ch->maxHit, SQL_INT},
        {"mana", &ch->mana, SQL_INT},
        {"maxMana", &ch->maxMana, SQL_INT},
        {"move", &ch->move, SQL_INT},
        {"maxMove", &ch->maxMove, SQL_INT},
        {"gold", &ch->gold, SQL_DOUBLE},
        {"position", &ch->position, SQL_LOOKUP, position_table},
        {"flags", &ch->flags, SQL_FLAG, flag_table},
        {"classes", &ch->classes, SQL_CUSTOM, NULL, NULL, 0, save_char_classes},
        {
            "stats", &ch->stats, SQL_ARRAY, NULL, &maxStat, 0, db_save_int_array

        },
        {"alignment", &ch->alignment, SQL_INT},
        {
            "resists", &ch->resists, SQL_ARRAY, NULL, &maxDam, 0, db_save_int_array

        },
        {"size", &ch->size, SQL_FLOAT},
        {"level", &ch->level, SQL_INT},
        {0}
    };

    if (ch->id == 0)
    {
        if (sql_insert_query(char_values, "character") != SQL_OK)
        {
            log_data("could not insert character");
            return 0;
        }
        ch->id = db_last_insert_rowid();

        return 1;
    }
    else
    {
        if (sql_update_query(char_values, "character", ch->id) !=
                SQL_OK)
        {
            log_data("could not update character");
            return 0;
        }
        return 2;
    }
}

int load_char_affects(Character *ch)
{
    char buf[400];
    sql_stmt *stmt;
    int total = 0;

    int len = sprintf(buf,
                      "select * from char_affect where characterId=%"
                      PRId64,
                      ch->id);

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {
        log_data("could not prepare statement");
        return 0;
    }
    while (sql_step(stmt) != SQL_DONE)
    {

        int affId = sql_col_int(stmt, "affectId");

        Affect *aff = load_affect_by_id(affId);

        if (aff != 0)
        {
            aff->duration = sql_col_int(stmt, "duration");

            affect_to_char(ch, aff);
        }
        total++;
    }

    if (sql_finalize(stmt) != SQL_OK)
    {
        log_data("could not finalize statement");
    }
    return total;
}

int save_char_affects(Character *ch)
{
    char buf[400];
    sql_stmt *stmt;
    int len;
    Affect *aff;

    for (aff = ch->affects; aff; aff = aff->next)
    {
        len =
            sprintf(buf,
                    "select charAffectId from char_affect where characterId=%"
                    PRId64 " and affectId=%" PRId64, ch->id, aff->id);

        if (sql_query(buf, len, &stmt) != SQL_OK)
        {
            log_data("could not prepare statement");
            return 0;
        }
        bool update = sql_step(stmt) != SQL_DONE;

        sql_int64 id = sql_column_int64(stmt, 1);

        if (sql_finalize(stmt) != SQL_OK)
        {
            log_data("unable to finalize sql statement");
            return 0;
        }
        if (!save_affect(aff))
            return 0;

        field_map char_affect[] =
        {
            {"characterId", &ch->id, SQL_INT}
            ,
            {"affectId", &aff->id, SQL_INT}
            ,
            {0}
        };

        if (!update)
        {
            if (sql_insert_query(char_affect, "char_affect") !=
                    SQL_OK)
            {
                log_data("could not insert affect");
                return 0;
            }
        }
        else
        {
            if (sql_update_query(char_affect, "char_affect", id) !=
                    SQL_OK)
            {
                log_data("could not update character");
                return 0;
            }
        }

    }

    // try and cleanup old objects

    len =
        sprintf(buf,
                "select charAffectId from char_affect where characterId=%"
                PRId64, ch->id);

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {
        log_data("could not prepare statement");
        return 0;
    }
    while (sql_step(stmt) != SQL_DONE)
    {
        identifier_t id = sql_column_int64(stmt, 1);

        for (aff = ch->affects; aff; aff = aff->next)
        {
            if (aff->id == id)
                break;
        }

        if (!aff)
        {
            sprintf(buf,
                    "delete from char_affect where characterId=%"
                    PRId64 " and affectId=%" PRId64, ch->id, id);

            if (sql_exec(buf) != SQL_OK)
            {
                log_data("could not delete character affect %"
                         PRId64, id);
                return 0;
            }
        }
    }

    if (sql_finalize(stmt) != SQL_OK)
    {
        log_data("could not finalize statement");
        return 0;
    }
    return 1;
}

short scrwidth(const Character *ch)
{
    return !ch->pc || !ch->pc->conn ? 80 : ch->pc->conn->scrWidth;
}

short scrheight(const Character *ch)
{
    return !ch->pc || !ch->pc->conn ? 24 : ch->pc->conn->scrHeight;
}

bool is_immortal(const Character *ch)
{
    return (ch->level >= LEVEL_IMMORTAL);
}

bool is_implementor(const Character *ch)
{
    return (ch->level >= MAX_LEVEL);
}

bool is_playing(const Character *ch)
{
    return ch->pc && ch->pc->conn && ch->pc->conn->is_playing(ch->pc->conn);
}

bool is_player(const Character *ch)
{
    return ch->pc && !is_immortal(ch);
}

Character *get_char_world(Character *ch, const char *argument)
{
    char arg[BUF_SIZ];
    long number;
    Character *wch;
    int count;

    if ((wch = get_char_room(ch, argument)) != NULL)
        return wch;

    number = number_argument(argument, arg);
    count = 0;
    for (wch = first_character; wch != NULL; wch = wch->next)
    {
        if (wch->inRoom == NULL || !can_see(ch, wch)
                || !is_name(arg, wch->name))
            continue;
        if (++count == number)
            return wch;
    }

    return NULL;
}
