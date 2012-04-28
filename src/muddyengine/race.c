
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
#include <muddyengine/flag.h>
#include <muddyengine/string.h>
#include <muddyengine/db.h>
#include <muddyengine/engine.h>
#include <muddyengine/macro.h>
#include <muddyengine/log.h>
#include <muddyengine/lookup.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdio.h>
#include <muddyengine/race.h>

Race *first_race = 0;

const Lookup race_flags[] = {
	{RACE_PC, "pcrace"},
	{0, 0}
};

const char *save_stats( void * );

Race *get_race_by_id( identifier_t id )
{

	for ( Race * r = first_race; r != 0; r = r->next )
	{
		if ( r->id == id )
			return r;
	}
	return 0;
}

Race *race_lookup( const char *arg )
{
	if ( !arg || !*arg )
		return 0;

	if ( is_number( arg ) )
	{
		return get_race_by_id( atoi( arg ) );
	}

	for ( Race * r = first_race; r != 0; r = r->next )
	{
		if ( !str_prefix( arg, r->name ) )
			return r;
	}
	return 0;
}

Race *new_race(  )
{
	Race *race = ( Race * ) alloc_mem( 1, sizeof( Race ) );

	race->name = str_empty;
	race->description = str_empty;
	race->flags = new_flag(  );
	memset( race->stats, 13, MAX_STAT * sizeof( int ) );

	return race;
}

void destroy_race( Race * race )
{
	free_str( race->name );
	free_str( race->description );

	free_mem( race );
}

int load_races(  )
{
	char buf[400];
	sqlite3_stmt *stmt;
	int total = 0;

	int len = sprintf( buf, "select * from race" );

	if ( sqlite3_prepare( sqlite3_instance, buf, len, &stmt, 0 ) != SQLITE_OK )
	{
		log_sqlite3( "could not prepare statement" );
		return 0;
	}

	while ( sqlite3_step( stmt ) != SQLITE_DONE )
	{
		int count = sqlite3_column_count( stmt );

		Race *race = new_race(  );

		for ( int i = 0; i < count; i++ )
		{
			const char *colname = sqlite3_column_name( stmt, i );

			if ( !str_cmp( colname, "name" ) )
			{
				race->name = str_dup( sqlite3_column_str( stmt, i ) );
			}
			else if ( !str_cmp( colname, "description" ) )
			{
				race->description = str_dup( sqlite3_column_str( stmt, i ) );
			}
			else if ( !str_cmp( colname, "raceId" ) )
			{
				race->id = sqlite3_column_int( stmt, i );
			}
			else if ( !str_cmp( colname, "stats" ) )
			{
				db_read_int_array( MAX_STAT, &race->stats, stmt, i );
			}
			else if ( !str_cmp( colname, "statMods" ) )
			{
				db_read_int_array( MAX_STAT, &race->statMods, stmt, i );
			}
			else if ( !str_cmp( colname, "flags" ) )
			{
				parse_flags( race->flags,
							 sqlite3_column_str( stmt, i ), race_flags );
			}
			else
			{
				log_warn( "unknown race column '%s'", colname );
			}
		}

		LINK( first_race, race, next );
		total++;
	}

	if ( sqlite3_finalize( stmt ) != SQLITE_OK )
	{
		log_sqlite3( "could not finalize statement" );
	}

	return total;
}

int save_race( Race * race )
{
	char buf[OUT_SIZ];

	struct dbvalues racevals[] = {
		{"name", &race->name, SQLITE_TEXT},
		{"description", &race->description, SQLITE_TEXT},
		{"flags", &race->flags, DBTYPE_FLAG, race_flags},
		{"stats", &race->stats, DBTYPE_ARRAY, db_save_int_array,
		 ( void * ) MAX_STAT},
		{"statMods", &race->statMods, DBTYPE_ARRAY, db_save_int_array,
		 ( void * ) MAX_STAT},
		{0}
	};

	if ( race->id == 0 )
	{
		char names[BUF_SIZ] = { 0 };
		char values[OUT_SIZ] = { 0 };

		build_insert_values( racevals, names, values );

		sprintf( buf, "insert into race (%s) values(%s)", names, values );

		if ( sqlite3_exec( sqlite3_instance, buf, NULL, 0, 0 ) != SQLITE_OK )
		{
			log_sqlite3( "could not insert race" );
			return 0;
		}

		race->id = sqlite3_last_insert_rowid( sqlite3_instance );

	}
	else
	{
		char values[OUT_SIZ] = { 0 };

		build_update_values( racevals, values );

		sprintf( buf, "update race set %s where raceId=%" PRId64, values, race->id );

		if ( sqlite3_exec( sqlite3_instance, buf, NULL, 0, 0 ) != SQLITE_OK )
		{
			log_sqlite3( "could not update race" );
			return 0;
		}
	}

	return 1;
}
