
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
#include <stdio.h>
#include "muddled/lookup.h"
#include "muddled/engine.h"
#include "muddled/db.h"
#include "muddled/string.h"
#include "muddled/log.h"
#include "muddled/script.h"
#include "muddled/forum.h"
#include "muddled/race.h"
#include "muddled/class.h"
#include "muddled/skill.h"
#include "muddled/area.h"
#include "muddled/social.h"
#include "muddled/social.h"
#include "muddled/help.h"
#include "muddled/hint.h"
#include "config.h"

Engine engine_info;

time_t current_time;

const char *weekdays[] =
{
    "Muday", "Aberday", "Dikuday", "Circleday", "Mercday",
    "Smaugday", "Romday"
};

const char *months[] =
{
    "Dragon", "Sword", "Dagger", "Centaur", "Sea", "Shield",
    "Mountain", "Sun", "Moon", "Wolf", "Griffon", "Horse"
};

const char *seasons[] = { "Darkness", "Life", "Light", "Leaves" };

const float exp_table[EXP_TABLE_SIZ] =
{
    5.9f,
    5.9f,
    5.9f,
    5.9f,
    6.2f,
    6.2f,
    6.2f,
    6.2f,
    6.5f,
    6.5f,
    6.5f,
    6.5f,
    6.8f,
    6.8f,
    6.8f,
    6.8f,
    7.1f,
    7.1f,
    7.1f,
    7.1f,
    7.2f
};

const Lookup stat_table[] =
{
    {"strength", STAT_STR},
    {"intelligence", STAT_INT},
    {"wisdom", STAT_WIS},
    {"dexterity", STAT_DEX},
    {"constitution", STAT_CON},
    {"luck", STAT_LUCK},
    {0, 0}
};

const Lookup engine_flags[] =
{
    {0, 0}
};

void initialize_default_engine()
{
    engine_info.name = str_dup("Muddled");

}

FILE *engine_open_file_in_dir(const char *folder, const char *name,
                              const char *perm)
{
    char buf[BUF_SIZ];
    sprintf(buf, "%s/%s/%s", engine_info.root_path, folder, name);
    return fopen(buf, perm);
}

FILE *engine_open_file(const char *filepath, const char *perm)
{
    char buf[BUF_SIZ];
    sprintf(buf, "%s/%s", engine_info.root_path, filepath);
    return fopen(buf, perm);
}

field_map *engine_field_map(Engine *info)
{
    static field_map *table = 0;

    field_map engine_values[] =
    {
        {"engineId", &info->id, SQL_INT},
        {"name", &info->name, SQL_TEXT},
        {"logins", &info->total_logins, SQL_INT},
        {"flags", &info->flags, SQL_FLAG, engine_flags},
        {0}
    };
    if (table == 0)
    {
        table = alloc_mem(sizeof(engine_values) / sizeof(engine_values[0]), sizeof(engine_values[0]));
    }
    memcpy(table, engine_values, sizeof(engine_values));

    return table;
}

int load_engine(const char *db_file, const char *root_path)
{
    /*char buf[500];
    sql_stmt *stmt;
    int len = sprintf(buf, "select * from engine");*/

    engine_info.flags = new_flag();
    engine_info.logging = new_flag();
    engine_info.root_path = str_dup(root_path);

    if (db_open(db_file, root_path))
    {
        log_data("Can't open database");
        db_close();
        exit(EXIT_FAILURE);
    }

    if (!db_load_by_id(engine_field_map(&engine_info), "engine", 1))
    {
        log_data("could not load engine info");
        return 0;
    }

    log_info("Starting %s", engine_info.name);

    return 1;
}

int save_engine()
{

    engine_info.id = db_save(engine_field_map(&engine_info), "engine", engine_info.id);

    return engine_info.id != 0;
}

void *alloc_mem(size_t num, size_t size)
{
    return calloc(num, size);
}

void free_mem(void *data)
{
    free(data);
}

void initialize_engine(const char *db_file, const char *root_path)
{
    log_info("Running from %s", root_path);

    current_time = time(0);

    init_lua();

    load_engine(db_file, root_path);

    synchronize_tables();

    log_info("loaded %d races", load_races());

    log_info("loaded %d classes", load_classes());

    log_info("loaded %d skills", load_skills());

    log_info("loaded %d areas", load_areas());

    log_info("loaded %d socials", load_socials());

    log_info("loaded %d helps", load_helps());

    log_info("loaded %d hints", load_hints());

    log_info("loaded %d forums", load_forums());


}
