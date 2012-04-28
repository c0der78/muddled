
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
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <muddyengine/flag.h>
#include <muddyengine/area.h>
#include <muddyengine/log.h>
#include <muddyengine/string.h>
#include <muddyengine/db.h>
#include <muddyengine/nonplayer.h>
#include <muddyengine/room.h>
#include <muddyengine/exit.h>
#include <muddyengine/object.h>
#include <muddyengine/macro.h>
#include <muddyengine/lookup.h>

Area *first_area = 0;
int max_area = 0;

const Lookup area_flags[] = {
	{AREA_NOEXPLORE, "noexplore"},
	{AREA_CHANGED, "changed"},
	{0, 0}
};

Area *new_area(  )
{
	Area *area = ( Area * ) alloc_mem( 1, sizeof( Area ) );

	area->name = str_empty;

	area->npcs = 0;

	area->flags = new_flag(  );

	return area;
}

void destroy_area( Area * area )
{
	free_str( area->name );

	destroy_flags( area->flags );

	free_mem( area );
}

void load_area_columns( Area * area, sqlite3_stmt * stmt )
{
	int count = sqlite3_column_count( stmt );

	for ( int i = 0; i < count; i++ )
	{
		const char *colname = sqlite3_column_name( stmt, i );

		if ( !str_cmp( colname, "areaId" ) )
		{
			area->id = sqlite3_column_int( stmt, i );
		}
		else if ( !str_cmp( colname, "name" ) )
		{
			area->name = str_dup( sqlite3_column_str( stmt, i ) );
		}
		else if ( !str_cmp( colname, "flags" ) )
		{
			parse_flags( area->flags,
						 sqlite3_column_str( stmt, i ), area_flags );
		}
		else
		{
			log_warn( "unknown column %s for area", colname );
		}
	}

}

Area *load_area( identifier_t id )
{
	char buf[500];
	sqlite3_stmt *stmt;
	Area *area = 0;

	int len =
		sprintf( buf, "select * from area where areaId = %"PRId64" limit 1", id );

	if ( sqlite3_prepare( sqlite3_instance, buf, len, &stmt, 0 ) != SQLITE_OK )
	{
		log_sqlite3( "could not prepare statement" );
		return 0;
	}

	if ( sqlite3_step( stmt ) != SQLITE_DONE )
	{
		area = new_area( );

		load_area_columns( area, stmt );

		log_info( "loaded %s (%d npcs, %d objs, %d rooms)", area->name,
				  load_npcs( area ), load_objects( area ), load_rooms( area ) );

		LINK( first_area, area, next );

		max_area++;
	}

	if ( sqlite3_finalize( stmt ) != SQLITE_OK )
	{
		log_sqlite3( "could not finalize statement" );
	}

	return area;
}

int load_areas(  )
{
	char buf[500];
	sqlite3_stmt *stmt;
	int total = 0;

	int len = sprintf( buf, "select * from area" );

	if ( sqlite3_prepare( sqlite3_instance, buf, len, &stmt, 0 ) != SQLITE_OK )
	{
		log_sqlite3( "could not prepare statement" );
		return 0;
	}

	while ( sqlite3_step( stmt ) != SQLITE_DONE )
	{
		Area *area = new_area(  );

		load_area_columns( area, stmt );

		log_info( "loaded %s (%d npcs, %d objs, %d rooms)", area->name,
				  load_npcs( area ), load_objects( area ), load_rooms( area ) );

		LINK( first_area, area, next );

		total++;
		max_area++;
	}

	if ( sqlite3_finalize( stmt ) != SQLITE_OK )
	{
		log_sqlite3( "could not finalize statement" );
	}

	finalize_exits(  );

	return total;
}

Area *get_area_by_id( identifier_t id )
{
	for ( Area * area = first_area; area != 0; area = area->next )
	{
		if ( area->id == id )
			return area;
	}
	return 0;
}

int save_area_only( Area * area )
{
	char buf[BUF_SIZ];

	remove_bit( area->flags, AREA_CHANGED );

	struct dbvalues areavals[] = {
		{"name", &area->name, SQLITE_TEXT},
		{"flags", &area->flags, DBTYPE_FLAG, area_flags},
		{0}
	};

	if ( area->id == 0 )
	{
		char names[BUF_SIZ] = { 0 };
		char values[OUT_SIZ] = { 0 };

		build_insert_values( areavals, names, values );

		sprintf( buf, "insert into area (%s) values(%s)", names, values );

		if ( sqlite3_exec( sqlite3_instance, buf, NULL, 0, 0 ) != SQLITE_OK )
		{
			log_sqlite3( "could not insert area" );
			return 0;
		}

		area->id = sqlite3_last_insert_rowid( sqlite3_instance );

	}
	else
	{
		char values[OUT_SIZ] = { 0 };

		build_update_values( areavals, values );

		sprintf( buf, "update area set %s where areaId=%" PRId64, values, area->id );

		if ( sqlite3_exec( sqlite3_instance, buf, NULL, 0, 0 ) != SQLITE_OK )
		{
			log_sqlite3( "could not update area" );
			return 0;
		}
	}
	return 1;
}

int save_area( Area * area )
{
	db_begin_transaction(  );

	save_area_only( area );

	for ( Object * obj = area->objects; obj; obj = obj->next_in_area )
	{
		save_object( obj );
	}

	for ( Character * npc = area->npcs; npc; npc = npc->next_in_area )
	{
		save_npc( npc );
	}

	for ( Room * room = area->rooms; room; room = room->next_in_area )
	{
		save_room( room );
	}

	db_end_transaction(  );

	return 1;
}

Area *area_lookup( const char *arg )
{
	if ( !arg || !*arg )
		return 0;

	if ( is_number( arg ) )
	{
		return get_area_by_id( atoi( arg ) );
	}

	for ( Area * area = first_area; area != 0; area = area->next )
	{
		if ( !str_prefix( arg, strip_color( area->name ) ) )
			return area;
	}

	return 0;
}

Area *get_default_area(  )
{
	if ( first_area != 0 )
		return first_area;

	Area *area = new_area(  );
	area->name = "The Default Area";
	LINK( first_area, area, next );

	max_area++;
	return area;
}
