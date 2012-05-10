
/******************************************************************************
 *         __  __           _     _         ____  _       _                   *
 *        |  \/  |_   _  __| | __| |_   _  |  _ \| | __ _(_)_ __  ___         *
 *        | |\/| | | | |/ _` |/ _` | | | | | |_) | |/ _` | | '_ \/ __|        *
 *        | |  | | |_| | (_| | (_| | |_| | |  __/| | (_| | | | | \__ \        *
 *        |_|  |_|\__,_|\__,_|\__,_|\__, | |_|   |_|\__,_|_|_| |_|___/        *
 *                                  |___/                                     *
 *                                                                            *
 *    (C) 2010 by Ryan Jennings <c0der78@gmail.com> www.ryan-jennings.net     *
 *	           Many thanks to creators of muds before me.                 *
 *                                                                            *
 *        In order to use any part of this Mud, you must comply with the      *
 *     license in 'license.txt'.  In particular, you may not remove either    *
 *                        of these copyright notices.                         *
 *                                                                            *
 *       Much time and thought has gone into this software and you are        *
 *     benefitting.  I hope that you share your changes too.  What goes       *
 *                            around, comes around.                           *
 ******************************************************************************/
#include <muddyengine/lookup.h>
#include <muddyengine/string.h>
#include <muddyengine/flag.h>
#include <muddyengine/race.h>
#include <muddyengine/log.h>
#include <muddyengine/db.h>
#include <muddyengine/engine.h>
#include <muddyengine/affect.h>
#include <muddyengine/help.h>
#include <stdio.h>
#include <inttypes.h>

void synch_table(const char *tablename, const Lookup * table)
{
    char buf[400];

    sprintf(buf, "CREATE TABLE IF NOT EXISTS [%s] ("
	    "[name] VARCHAR NOT NULL PRIMARY KEY UNIQUE,"
	    "[value] INTEGER)", tablename);

    if (sql_exec(buf) != SQL_OK) {
	log_data("could not create %s", tablename);
	return;
    }
    for (const Lookup * ptable = table; ptable->name != 0; ptable++) {
	sprintf(buf,
		"insert into %s (name,value) values('%s','%" PRIXPTR "')",
		tablename, ptable->name, ptable->value);

	if (sql_exec(buf) != SQL_OK) {

	    sprintf(buf,
		    "update %s set name='%s', value='%" PRIXPTR
		    "' where name='%s'", tablename, ptable->name,
		    ptable->value, ptable->name);

	    if (sql_exec(buf) != SQL_OK) {
		log_data("could not save into %s", tablename);

		return;
	    }
	}
    }
}

void synchronize_tables()
{
    log_info("synchronizing tables");

    synch_table("direction", direction_table);
    synch_table("sex", sex_table);
    synch_table("position", position_table);
    synch_table("help_category", help_categories);
    synch_table("wear_type", wear_types);
    synch_table("object_type", object_types);
    synch_table("sector", sector_table);
    synch_table("stats", stat_table);
    synch_table("dam_type", dam_types);
    synch_table("affect_type", affect_callbacks);
}

uintptr_t value_lookup(const Lookup * table, const char *arg)
{
    if (!arg || !*arg)
	return -1;

    for (const Lookup * t = table; t->name != 0; t++) {
	if (!str_prefix(arg, t->name))
	    return t->value;
    }

    return -1;
}

int index_lookup(const Lookup * table, const char *arg)
{
    if (nullstr(arg))
	return -1;

    for (int i = 0; table[i].name != 0; i++) {
	if (!str_prefix(arg, table[i].name))
	    return i;
    }

    return -1;
}

const char *lookup_names(const Lookup * table)
{
    static char buf[500];

    buf[0] = 0;

    for (const Lookup * t = table; t->name != 0; t++) {
	strcat(buf, t->name);
	strcat(buf, " ");
    }

    return buf;
}

const char *lookup_name(const Lookup * table, uintptr_t value)
{
    for (const Lookup * t = table; t->name != 0; t++) {
	if (t->value == value)
	    return t->name;
    }

    return 0;
}
