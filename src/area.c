
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
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "flag.h"
#include "area.h"
#include "log.h"
#include "string.h"
#include "db.h"
#include "nonplayer.h"
#include "room.h"
#include "exit.h"
#include "object.h"
#include "macro.h"
#include "lookup.h"

Area *first_area = 0;
int max_area = 0;

const Lookup area_flags[] =
{
    {"noexplore", AREA_NOEXPLORE},
    {"changed", AREA_CHANGED},
    {0, 0}
};

field_map *area_field_map(Area *area)
{
    static field_map *area_values;

    field_map temp[] =
    {
        {"areaId", &area->id, SQL_INT },
        {"name", &area->name, SQL_TEXT},
        {"flags", &area->flags, SQL_FLAG, area_flags},
        {0}
    };
    return area_values = temp;
}

Area *new_area()
{
    Area *area = (Area *) alloc_mem(1, sizeof(Area));

    area->name = str_empty;

    area->npcs = 0;

    area->flags = new_flag();

    return area;
}

void destroy_area(Area *area)
{
    free_str(area->name);

    destroy_flags(area->flags);

    free_mem(area);
}

/*void load_area_columns(Area * area, sql_stmt * stmt)
{
    int count = sql_column_count(stmt);

    for (int i = 0; i < count; i++) {
        const char *colname = sql_column_name(stmt, i);

        if (!str_cmp(colname, "areaId")) {
            area->id = sql_column_int(stmt, i);
        } else if (!str_cmp(colname, "name")) {
            area->name = str_dup(sql_column_str(stmt, i));
        } else if (!str_cmp(colname, "flags")) {
            parse_flags(area->flags, sql_column_str(stmt, i),
                    area_flags);
        } else {
            log_warn("unknown column %s for area", colname);
        }
    }

}*/

Area *load_area(identifier_t id)
{
    /*char buf[500];
    sql_stmt *stmt;
    Area *area = 0;

    int len = sprintf(buf,
              "select * from area where areaId = %"
              PRId64 " limit 1",
              id);


    if (sql_query(buf, len, &stmt) != SQL_OK) {
        log_data("could not prepare statement");
        return 0;
    }
    if (sql_step(stmt) != SQL_DONE) {
        area = new_area();

        load_area_columns(area, stmt);

        log_info("loaded %s (%d npcs, %d objs, %d rooms)", area->name,
             load_npcs(area), load_objects(area), load_rooms(area));

        LINK(first_area, area, next);

        max_area++;
    }
    if (sql_finalize(stmt) != SQL_OK) {
        log_data("could not finalize statement");
    }*/

    Area *area = new_area();

    int res = db_load_by_id(area_field_map(area), "area",  id);

    if (!res)
    {
        log_data("unable to load area");
        destroy_area(area);
        return 0;
    }

    return area;
}


static void create_area(sql_stmt *stmt)
{
    Area *area = new_area();

    sql_load_columns(stmt, area_field_map(area));

    log_info("loaded %s (%d npcs, %d objs, %d rooms)", area->name,
             load_npcs(area), load_objects(area), load_rooms(area));

    LINK(first_area, area, next);

    max_area++;
}

int load_areas()
{
    /*char buf[500];
    sql_stmt *stmt;
    int total = 0;

    int len = sprintf(buf, "select * from area");

    if (sql_query(buf, len, &stmt) != SQL_OK) {
        log_data("could not prepare statement");
        return 0;
    }
    while (sql_step(stmt) != SQL_DONE) {
        Area *area = new_area();

        load_area_columns(area, stmt);

        log_info("loaded %s (%d npcs, %d objs, %d rooms)", area->name,
             load_npcs(area), load_objects(area), load_rooms(area));

        LINK(first_area, area, next);

        total++;
        max_area++;
    }

    if (sql_finalize(stmt) != SQL_OK) {
        log_data("could not finalize statement");
    }
    finalize_exits();

    return total;*/

    db_load_all("area", create_area, 0);

    finalize_exits();

    return max_area;
}

Area *get_area_by_id(identifier_t id)
{
    for (Area *area = first_area; area != 0; area = area->next)
    {
        if (area->id == id)
            return area;
    }
    return 0;
}

int save_area_only(Area *area)
{
    remove_bit(area->flags, AREA_CHANGED);



    if (area->id == 0)
    {
        if (sql_insert_query(area_field_map(area), "area") != SQL_OK)
        {
            log_data("could not insert area");
            return 0;
        }
        area->id = db_last_insert_rowid();

    }
    else
    {
        if (sql_update_query(area_field_map(area), "area", area->id) != SQL_OK)
        {
            log_data("could not update area");
            return 0;
        }
    }
    return 1;
}

int save_area(Area *area)
{
    db_begin_transaction();

    save_area_only(area);

    for (Object *obj = area->objects; obj; obj = obj->next_in_area)
    {
        save_object(obj);
    }

    for (Character *npc = area->npcs; npc; npc = npc->next_in_area)
    {
        save_npc(npc);
    }

    for (Room *room = area->rooms; room; room = room->next_in_area)
    {
        save_room(room);
    }

    db_end_transaction();

    return 1;
}

Area *area_lookup(const char *arg)
{
    if (!arg || !*arg)
        return 0;

    if (is_number(arg))
    {
        return get_area_by_id(atoi(arg));
    }
    for (Area *area = first_area; area != 0; area = area->next)
    {
        if (!str_prefix(arg, strip_color(area->name)))
            return area;
    }

    return 0;
}

Area *get_default_area()
{
    if (first_area != 0)
        return first_area;

    Area *area = new_area();
    area->name = "The Default Area";
    LINK(first_area, area, next);

    max_area++;
    return area;
}
