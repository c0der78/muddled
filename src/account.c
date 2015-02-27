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

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include "engine.h"
#include "account.h"
#include "string.h"
#include "db.h"
#include "log.h"
#include "character.h"
#include "player.h"
#include "forum.h"
#include "lookup.h"
#include "flag.h"

const Lookup account_flags[] =
{
    {"coloroff", ACC_COLOR_OFF},
    {0, 0}
};

Account *new_account(Connection *conn)
{
    Account *acc = (Account *) alloc_mem(1, sizeof(Account));

    acc->login = str_empty;
    acc->password = str_empty;
    acc->email = str_empty;

    acc->conn = conn;

    acc->forum = &forum_table[0];

    acc->flags = new_flag();

    acc->forumData =
        (AccountForum *) alloc_mem(max_forum, sizeof(AccountForum));

    for (int i = 0; i < max_forum; i++)
        acc->forumData[i].draft = str_empty;

    return acc;
}

void destroy_account(Account *acc)
{
    free_str(acc->login);
    free_str(acc->password);
    free_str(acc->email);

    destroy_flags(acc->flags);

    for (int i = 0; i < max_forum; i++)
    {
        free_str(acc->forumData[i].draft);
    }

    free_mem(acc->forumData);

    for (AccountPlayer * ch_next, *ch = acc->players; ch != 0; ch = ch_next)
    {
        ch_next = ch->next;
        destroy_account_player(ch);
    }

    if (acc->playing)
    {
        extract_char(acc->playing, true);
    }
    free_mem(acc);
}

AccountPlayer *new_account_player()
{
    AccountPlayer *p =
        (AccountPlayer *) alloc_mem(1, sizeof(AccountPlayer));

    p->name = str_empty;

    return p;
}

void destroy_account_player(AccountPlayer *p)
{
    free_str(p->name);

    free_mem(p);
}

void account_forum_set_last_note(Account *acc, time_t value)
{
    assert(acc != 0);

    int i = lookup_forum_by_id(acc->forum->id);

    if (i == FORUM_ERROR)
        return;

    acc->forumData[i].lastNote = value;
}

time_t account_forum_last_note(Account *acc)
{
    assert(acc != 0 && acc->forum != 0);

    int i = lookup_forum_by_id(acc->forum->id);

    if (i == FORUM_ERROR)
        return 0;

    return acc->forumData[i].lastNote;
}

bool account_forum_is_subscribed(Account *acc)
{
    assert(acc != 0 && acc->forum != 0);

    int i = lookup_forum_by_id(acc->forum->id);

    if (i == FORUM_ERROR)
        return true;

    return !acc->forumData[i].unsubscribed;
}

int load_account(Account *acc, const char *login)
{
    char buf[500];
    sql_stmt *stmt;
    int len = sprintf(buf, "select * from account where login='%s'", login);

    log_debug("loading account %s", login);

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {
        log_data("could not prepare sql statement");
        return 0;
    }
    if (sql_step(stmt) == SQL_DONE)
    {
        if (sql_finalize(stmt) != SQL_OK)
            log_data("could not finalize sql statement");
        return 0;
    }
    do
    {
        int i, cols = sql_column_count(stmt);
        for (i = 0; i < cols; i++)
        {
            const char *colname = sql_column_name(stmt, i);

            if (!str_cmp(colname, "accountId"))
            {
                acc->id = sql_column_int(stmt, i);
            }
            else if (!str_cmp(colname, "login"))
            {
                acc->login = str_dup(sql_column_str(stmt, i));
            }
            else if (!str_cmp(colname, "email"))
            {
                acc->email = str_dup(sql_column_str(stmt, i));
            }
            else if (!str_cmp(colname, "timezone"))
            {
                acc->timezone = sql_column_int(stmt, i);
            }
            else if (!str_cmp(colname, "autologinId"))
            {
                acc->autologinId = sql_column_int(stmt, i);
            }
            else if (!str_cmp(colname, "flags"))
            {
                parse_flags(acc->flags,
                            sql_column_str(stmt, i),
                            account_flags);
            }
            else if (!str_cmp(colname, "password"))
            {
                acc->password =
                    str_dup(sql_column_str(stmt, i));
            }
            else
            {
                log_warn("unknown account column '%s'",
                         colname);
            }
        }
    }
    while (sql_step(stmt) != SQL_DONE);

    if (sql_finalize(stmt) != SQL_OK)
    {
        log_data("could not finalize sql statement");
    }
    /* load account players */

    load_account_players(acc);

    load_account_forums(acc);

    return 1;
}

field_map *account_forum_fields(Account *acc, AccountForum *forum)
{
    static field_map *table = 0;

    field_map account_values[] =
    {
        {"accountForumId", &forum->id, SQL_INT},
        {"forumId", &forum->id, SQL_INT},
        {"accountId", &acc->id, SQL_INT},
        {"lastNote", &forum->lastNote, SQL_INT},
        {"unsubscribed", &forum->unsubscribed, SQL_INT},
        {"draft", &forum->draft, SQL_TEXT},
        {0}
    };

    if (table == 0)
    {
        table = alloc_mem(1, sizeof(account_values));
    }

    memcpy(table, account_values, sizeof(account_values));

    return table;
}

int save_account_forum(Account *acc, int f)
{
    AccountForum *forum = &acc->forumData[f];


    forum->id = db_save(account_forum_fields(acc, forum), ACCOUNT_FORUM_TABLE, forum->id);

    return forum->id != 0;
}

field_map *account_fields(Account *acc)
{
    static field_map *table = 0;

    field_map accvalues[] =
    {
        {"accountId", &acc->id, SQL_INT},
        {"login", &acc->login, SQL_TEXT},
        {"email", &acc->email, SQL_TEXT},
        {"password", &acc->password, SQL_TEXT},
        {"timezone", &acc->timezone, SQL_INT},
        {"autologinId", &acc->autologinId, SQL_INT},
        {"flags", &acc->flags, SQL_FLAG, account_flags},
        {0}
    };

    if (table == 0)
    {
        table = alloc_mem(1, sizeof(accvalues));
    }
    memcpy(table, accvalues, sizeof(accvalues));
    return table;
}

int save_account(Account *acc)
{

    acc->id = db_save(account_fields(acc), ACCOUNT_TABLE, acc->id);

    for (int i = 0; i < max_forum; ++i)
        save_account_forum(acc, i);

    return acc->id != 0;
}

int delete_account(Account *acc)
{
    return db_delete(ACCOUNT_TABLE, acc->id) == SQL_OK;
}

int load_account_players(Account *acc)
{
    char buf[400];
    sql_stmt *stmt;

    int len = sprintf(buf,
                      "select * from character natural join player where accountId=%"
                      PRId64,
                      acc->id);

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {
        log_data("could not prepare sql statement");
        return 0;
    }
    if (sql_step(stmt) == SQL_DONE)
    {
        if (sql_finalize(stmt) != SQL_OK)
            log_data("could not finalize sql statement");
        return 0;
    }
    AccountPlayer *ch = new_account_player();

    do
    {
        int i, cols = sql_column_count(stmt);

        for (i = 0; i < cols; i++)
        {
            const char *colname = sql_column_name(stmt, i);

            if (!str_cmp(colname, "name"))
            {
                ch->name = str_dup(sql_column_str(stmt, i));
            }
            else if (!str_cmp(colname, "level"))
            {
                ch->level = sql_column_int(stmt, i);
            }
            else if (!str_cmp(colname, "accountId"))
            {
                if (acc->id != sql_column_int(stmt, i))
                {
                    log_error
                    ("sql retrieved invalid player for account");
                    break;
                }
                LINK(acc->players, ch, next);
            }
            else if (!str_cmp(colname, "characterId"))
            {
                ch->charId = sql_column_int(stmt, i);
            }
        }
    }
    while (sql_step(stmt) != SQL_DONE);

    if (sql_finalize(stmt) != SQL_OK)
    {
        log_data("unable to finalize statement");
    }
    return 1;

}

int load_account_forums(Account *acc)
{
    char buf[400];
    sql_stmt *stmt;

    int len = sprintf(buf,
                      "select * from account_forum where accountId=%"
                      PRId64,
                      acc->id);

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {
        log_data("could not prepare sql statement");
        return 0;
    }
    if (sql_step(stmt) == SQL_DONE)
    {
        if (sql_finalize(stmt) != SQL_OK)
            log_data("could not finalize sql statement");
        return 0;
    }
    do
    {
        int i, cols = sql_column_count(stmt);

        AccountForum f;

        for (i = 0; i < cols; i++)
        {
            const char *colname = sql_column_name(stmt, i);
            if (!str_cmp(colname, "timestamp"))
            {
                f.lastNote = sql_column_int(stmt, i);
            }
            else if (!str_cmp(colname, "unsubscribed"))
            {
                f.unsubscribed = sql_column_int(stmt, i) == 1;
            }
            else if (!str_cmp(colname, "draft"))
            {
                f.draft = sql_column_str(stmt, i);
            }
            else if (!str_cmp(colname, "forumId"))
            {
                f.id = sql_column_int(stmt, i);
            }
        }

        int index = lookup_forum_by_id(f.id);

        if (index != FORUM_ERROR)
        {
            memcpy(&acc->forumData[index], &f, sizeof(f));
        }
    }
    while (sql_step(stmt) != SQL_DONE);

    if (sql_finalize(stmt) != SQL_OK)
    {
        log_data("unable to finalize statement");
    }
    return 1;
}
