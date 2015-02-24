
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

#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include <muddled/flag.h>
#include <muddled/player.h>
#include <muddled/string.h>
#include <muddled/character.h>
#include <muddled/log.h>
#include <muddled/class.h>
#include <stdio.h>
#include <muddled/room.h>
#include <muddled/account.h>
#include <muddled/macro.h>
#include <muddled/util.h>
#include <muddled/race.h>
#include <muddled/connection.h>
#include <muddled/lookup.h>
#include <muddled/engine.h>
#include <muddled/explored.h>
#include <muddled/channel.h>

Character *first_player = 0;

const Lookup plr_flags[] =
{
    {"ticksoff", PLR_TICKS_OFF},
    {"brief", PLR_BRIEF},
    {"hints", PLR_HINTS},
    {"automapoff", PLR_AUTOMAP_OFF},
    {0, 0}
};

Player *new_player(Connection * conn)
{

    Player *p = (Player *) alloc_mem(1, sizeof(Player));

    p->title = str_empty;

    p->prompt = str_dup("[~R%h~x/~R%H~rhp ~M%m~x/~M%M~mm ~B%v/~B%V~bmv~x]");

    p->battlePrompt =
        str_dup("[~R%h~x/~R%H~rhp ~M%m~x/~M%M~mm ~B%v/~B%V~bmv~x] %E");

    p->conn = conn;

    p->account = conn->account;

    p->explored = new_flag();

    p->channels = new_flag();

    p->permHit = 100;

    p->permMana = 100;

    p->permMove = 100;

    return p;

}

void destroy_player(Player * p)
{

    free_str(p->title);

    free_str(p->prompt);

    free_str(p->battlePrompt);

    destroy_flags(p->explored);

    destroy_flags(p->channels);

    free_mem(p);

}

int delete_player(Character * ch)
{

    char buf[BUF_SIZ];

    if (!delete_character(ch))
        return 0;

    sprintf(buf, "delete from player where characterId=%" PRId64, ch->id);

    if (sql_exec(buf) != SQL_OK)
    {

        log_data("could not delete player");
        return 0;

    }
    return 1;

}

static int save_explored(sql_stmt * stmt, int index, field_map * table)
{

    const char *rle = get_explored_rle(*((Flag **) table->value));

    return sql_bind_text(stmt, index, rle, strlen(rle), 0);
}

int save_player(Character * ch)
{
    if (ch->pc == 0)
    {
        log_error("character not a player");
        return 0;
    }
    db_begin_transaction();

    if (!save_account(ch->pc->account))
    {
        return 0;
    }
    int res = save_character(ch, plr_flags);

    field_map pc_values[] =
    {
        {"playerId", &ch->id, SQL_INT},
        {"accountId", &ch->pc->account->id, SQL_INT},
        {"title", &ch->pc->title, SQL_TEXT},
        {"roomId", (ch->inRoom ? &ch->inRoom->id : 0), SQL_INT},
        {"prompt", &ch->pc->prompt, SQL_TEXT},
        {"battlePrompt", &ch->pc->battlePrompt, SQL_TEXT},
        {"explored", &ch->pc->explored, SQL_CUSTOM, save_explored},
        {"channels", &ch->pc->channels, SQL_FLAG, channel_flags},
        {
            "condition", &ch->pc->condition, SQL_CUSTOM, db_save_int_array,
            (void *)MAX_COND
        },
        {"experience", &ch->pc->experience, SQL_INT},
        {"permHit", &ch->pc->permHit, SQL_INT},
        {"permMana", &ch->pc->permMana, SQL_INT},
        {"permMove", &ch->pc->permMove, SQL_INT},
        {"created", &ch->pc->created, SQL_INT},
        {0}
    };

    if (res == 1)
    {
        if (sql_insert_query(pc_values, "player") != SQL_OK)
        {
            log_data("could not insert player");
            return 0;
        }
    }
    else if (res == 2)
    {
        if (sql_update_query(pc_values, "player", ch->id) != SQL_OK)
        {
            log_data("could not update character");
            return 0;
        }
    }
    if (!save_char_objs(ch))
        res = 0;

    if (!save_char_affects(ch))
        res = 0;

    db_end_transaction();

    return UMIN(res, 1);

}

void load_player_columns(Account * acc, Character * ch, sql_stmt * stmt)
{

    int i, cols = sql_column_count(stmt);

    for (i = 0; i < cols; i++)
    {

        const char *colname = sql_column_name(stmt, i);

        if (load_char_column(ch, stmt, colname, i))
        {

        }
        else if (!str_cmp(colname, "title"))
        {

            ch->pc->title = str_dup(sql_column_str(stmt, i));

        }
        else if (!str_cmp(colname, "prompt"))
        {

            free_str(ch->pc->prompt);

            ch->pc->prompt = str_dup(sql_column_str(stmt, i));

        }
        else if (!str_cmp(colname, "battlePrompt"))
        {

            free_str(ch->pc->battlePrompt);

            ch->pc->battlePrompt = str_dup(sql_column_str(stmt, i));

        }
        else if (!str_cmp(colname, "accountId"))
        {

            if (acc && acc->id != sql_column_int(stmt, i))
                log_error
                ("sql returned invalid account for player");

        }
        else if (!str_cmp(colname, "flags"))
        {

            parse_flags(ch->flags, sql_column_str(stmt, i),
                        plr_flags);

        }
        else if (!str_cmp(colname, "roomId"))
        {

            ch->inRoom = get_room_by_id(sql_column_int(stmt, i));

        }
        else if (!str_cmp(colname, "explored"))
        {

            convert_explored_rle(ch->pc->explored,
                                 sql_column_str(stmt, i));

        }
        else if (!str_cmp(colname, "channels"))
        {

            parse_flags(ch->pc->channels,
                        sql_column_str(stmt, i), channel_flags);

        }
        else if (!str_cmp(colname, "condition"))
        {

            db_read_int_array(MAX_COND, ch->pc->condition, stmt, i);

        }
        else if (!str_cmp(colname, "experience"))
        {

            ch->pc->experience = sql_column_int(stmt, i);

        }
        else if (!str_cmp(colname, "permHit"))
        {

            ch->pc->permHit = sql_column_int(stmt, i);

        }
        else if (!str_cmp(colname, "permMana"))
        {

            ch->pc->permMana = sql_column_int(stmt, i);

        }
        else if (!str_cmp(colname, "permMove"))
        {

            ch->pc->permMove = sql_column_int(stmt, i);

        }
        else if (!str_cmp(colname, "created"))
        {

            ch->pc->created = sql_column_int(stmt, i);

        }
        else
        {

            log_warn("unknown player column '%s'", colname);

        }

    }

}

Character *load_player_by_id(Connection * conn, identifier_t charId)
{

    char buf[400];

    sql_stmt *stmt;

    db_begin_transaction();

    int len = sprintf(buf,
                      "select * from character join player on playerId=characterId where characterId=%"
                      PRId64,
                      charId);

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {

        log_data("could not prepare sql statement");

        return 0;

    }
    Character *ch = new_char();

    ch->pc = new_player(conn);

    if (sql_step(stmt) != SQL_DONE)
    {

        load_player_columns(conn->account, ch, stmt);

    }
    if (sql_finalize(stmt) != SQL_OK)
    {

        log_data("unable to finalize statement");

    }
    load_char_objs(ch);

    load_char_affects(ch);

    db_end_transaction();

    return ch;

}

Character *load_player_by_name(Connection * conn, const char *name)
{

    char buf[400];

    sql_stmt *stmt;

    db_begin_transaction();

    int len = sprintf(buf,
                      "select * from character natural join player where name='%s'",
                      escape_sql_str(name));

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {

        log_data("could not prepare sql statement");

        return 0;

    }
    Character *ch = new_char();

    ch->pc = new_player(conn);

    if (sql_step(stmt) != SQL_DONE)
    {

        load_player_columns(conn->account, ch, stmt);

    }
    if (sql_finalize(stmt) != SQL_OK)
    {

        log_data("unable to finalize statement");

    }
    load_char_objs(ch);

    load_char_affects(ch);

    db_end_transaction();

    return ch;

}

Character *get_player_by_id(int id)
{

    for (Character * ch = first_player; ch != 0; ch = ch->next_player)
    {

        if (ch->id == id)
            return ch;

    }

    return 0;

}

Character *player_lookup(const char *arg)
{

    if (is_number(arg))
    {

        return get_player_by_id(atoi(arg));

    }
    else
    {

        for (Character * nch = first_player; nch != 0;
                nch = nch->next_player)
        {

            if (is_name(arg, nch->name))
                return nch;

        }

        return 0;

    }

}

long exp_to_level(const Character * ch)
{

    if (!ch->pc)
        return 0;

    double total = 0;

    int mod = UMIN(MAX_PLAYABLE_LEVEL, ch->level) / LEVEL_GROUPS;

    for (int a = 0; a < ch->level * class_count(ch); a++)
    {

        double t = (pow(a, 2) * exp_table[mod]);

        total += t;

    }

    return (long)total;

}

bool is_drunk(Character * ch)
{

    return ch->pc->condition[COND_DRUNK] > 10;

}
