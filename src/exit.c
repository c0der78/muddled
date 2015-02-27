
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

#include "flag.h"
#include "exit.h"
#include <stdlib.h>
#include <inttypes.h>
#include "db.h"
#include <stdio.h>
#include "room.h"
#include "log.h"
#include "lookup.h"
#include "string.h"
#include "engine.h"

const Lookup direction_table[] =
{
    {"north", DIR_NORTH},
    {"east", DIR_EAST},
    {"south", DIR_SOUTH},
    {"west", DIR_WEST},
    {"up", DIR_UP},
    {"down", DIR_DOWN},
    {0, 0}
};

const Lookup exit_flags[] =
{
    {"closed", EXIT_CLOSED},
    {"locked", EXIT_LOCKED},
    {0, 0}
};

Exit *new_exit()
{

    Exit *ex = (Exit *) alloc_mem(1, sizeof(Exit));

    ex->flags = new_flag();

    ex->status = new_flag();

    return ex;

}

void destroy_exit(Exit *ex)
{

    destroy_flags(ex->flags);

    destroy_flags(ex->status);

    free_mem(ex);

}

void finalize_exits()
{

    for (int i = 0; i < ID_HASH; i++)
    {

        for (Room *room = room_hash[i]; room != 0; room = room->next)
        {

            for (direction_t dir = DIR_NORTH; dir < MAX_DIR; dir++)
            {

                if (room->exits[dir] == 0)
                    continue;

                identifier_t id = room->exits[dir]->to.id;

                Room *toRoom = get_room_by_id(id);

                if (toRoom == 0)
                    log_bug("invalid room (%" PRId64
                            ") for exit (%" PRId64 ")", id,
                            room->exits[dir]->id);

                room->exits[dir]->to.room = toRoom;

            }
        }
    }
}

int load_exits(Room *room)
{

    char buf[BUF_SIZ];

    sql_stmt *stmt;

    int total = 0;

    int len = sprintf(buf, "select * from exit where fromRoom=%" PRId64,
                      room->id);

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {

        log_data("could not prepare statement");

        return 0;

    }
    while (sql_step(stmt) != SQL_DONE)
    {

        int count = sql_column_count(stmt);

        Exit *exit = new_exit();

        exit->fromRoom = room;

        direction_t dir = MAX_DIR;

        for (int i = 0; i < count; i++)
        {

            const char *colname = sql_column_name(stmt, i);

            if (!str_cmp(colname, "exitId"))
            {

                exit->id = sql_column_int(stmt, i);

            }
            else if (!str_cmp(colname, "fromRoom"))
            {

                if (room->id != sql_column_int(stmt, i))
                    log_error
                    ("sql returned invalid exit for room");

            }
            else if (!str_cmp(colname, "toRoom"))
            {

                exit->to.id = sql_column_int(stmt, i);

            }
            else if (!str_cmp(colname, "direction"))
            {

                dir = sql_column_int(stmt, i);

            }
            else if (!str_cmp(colname, "flags"))
            {

                parse_flags(exit->flags,
                            sql_column_str(stmt, i),
                            exit_flags);

                copy_flags(exit->status, exit->flags);

            }
            else
            {

                log_warn("unknown room column '%s'", colname);

            }

        }

        if (dir == MAX_DIR)
        {

            log_bug("exit with no direction");

            destroy_exit(exit);

        }
        else if (room->exits[dir] != 0)
        {

            log_bug("room already has exit");

            destroy_exit(exit);

        }
        else if (exit->to.id == 0)
        {
            log_bug("exit (%" PRId64 ") with no room", exit->id);

            destroy_exit(exit);
        }
        else
        {

            room->exits[dir] = exit;

            total++;

        }

    }

    if (sql_finalize(stmt) != SQL_OK)
    {

        log_data("could not finalize statement");

    }
    return total;

}

int save_exit(Exit *exit, direction_t dir)
{
    if (exit->to.room == 0)
    {

        log_error("Exit %" PRId64 " with no room %" PRId64, exit->id,
                  exit->to.id);

        return 0;

    }
    field_map exit_values[] =
    {
        {"toRoom", &exit->to.room->id, SQL_INT}
        ,
        {"fromRoom", &exit->fromRoom->id, SQL_INT}
        ,
        {"direction", &dir, SQL_INT}
        ,
        {"flags", &exit->flags, SQL_FLAG, exit_flags}
        ,
        {0}
    };

    if (exit->id == 0)
    {
        if (sql_insert_query(exit_values, "exit") != SQL_OK)
        {

            log_data("could not insert exit");

            return 0;

        }
        exit->id = db_last_insert_rowid();

    }
    else
    {
        if (sql_update_query(exit_values, "exit", exit->id) != SQL_OK)
        {

            log_data("could not update exit");

            return 0;

        }
    }

    return 1;

}
