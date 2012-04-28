
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
#include <muddyengine/room.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <muddyengine/string.h>
#include <muddyengine/db.h>
#include <muddyengine/log.h>
#include <muddyengine/area.h>
#include <muddyengine/exit.h>
#include <muddyengine/macro.h>
#include <muddyengine/lookup.h>
#include <muddyengine/engine.h>
#include <muddyengine/util.h>
#include <muddyengine/explored.h>

Room *room_hash[ID_HASH] = { 0 };

int max_room = 0;
int max_explorable_room = 0;

const Lookup sector_table[] = {
	{SECT_INSIDE, "inside"},
	{SECT_CITY, "city"},
	{SECT_FIELD, "field"},
	{SECT_FOREST, "forest"},
	{SECT_HILLS, "hills"},
	{SECT_MOUNTAIN, "mountain"},
	{SECT_WATER_SWIM, "water_swim"},
	{SECT_WATER_NOSWIM, "water_noswim"},
	{SECT_AIR, "air"},
	{SECT_DESERT, "desert"},
	{SECT_JUNGLE, "jungle"},
	{SECT_ROAD, "road"},
	{SECT_PATH, "path"},
	{SECT_CAVE, "cave"},
	{SECT_SWAMP, "swamp"},
	{SECT_UNKNOWN, "unknown"},
	{0, 0}
};

const Lookup room_flags[] = {
	{ROOM_NOEXPLORE, "noexplore"},
	{ROOM_SAFE, "safe"},
	{ROOM_NO_RECALL, "no_recall"},
	{0, 0}
};

Room *new_room(  )
{
	Room *room = ( Room * ) alloc_mem( 1, sizeof( Room ) );

	room->name = str_empty;
	room->description = str_empty;

	room->flags = new_flag(  );

	return room;
}

void destroy_room( Room * room )
{
	free_str( room->name );
	free_str( room->description );
	destroy_flags( room->flags );

	for ( Character * ch_next, *ch = room->characters; ch != 0; ch = ch_next )
	{
		ch_next = ch->next_in_room;

		writeln( ch,
				 "The gods wreack havok upon the land and you are transported somewhere safe.." );

		char_from_room( ch );

		char_to_room( ch, get_room_by_id( DEFAULT_ROOM ) );
	}

	free_str( room->reset );

	free_mem( room );
}

void load_room_columns( Room * room, sqlite3_stmt * stmt )
{
	int count = sqlite3_column_count( stmt );
	for ( int i = 0; i < count; i++ )
	{
		const char *colname = sqlite3_column_name( stmt, i );

		if ( !str_cmp( colname, "name" ) )
		{
			room->name = str_dup( sqlite3_column_str( stmt, i ) );
		}
		else if ( !str_cmp( colname, "description" ) )
		{
			room->description = str_dup( sqlite3_column_str( stmt, i ) );
		}
		else if ( !str_cmp( colname, "roomId" ) )
		{
			room->id = sqlite3_column_int( stmt, i );
		}
		else if ( !str_cmp( colname, "reset" ) )
		{
			room->reset = str_dup( sqlite3_column_str( stmt, i ) );
		}
		else if ( !str_cmp( colname, "areaId" ) )
		{
			int areaId = sqlite3_column_int( stmt, i );

			if ( room->area != 0 )
			{
				if ( room->area->id != areaId )
					log_error( "sql returned invalid room for area" );
			}
			else
			{
				room->area = get_area_by_id( areaId );
			}
		}
		else if ( !str_cmp( colname, "sector" ) )
		{
			room->sector = sqlite3_column_int( stmt, i );
		}
		else if ( !str_cmp( colname, "flags" ) )
		{
			parse_flags( room->flags,
						 sqlite3_column_str( stmt, i ), room_flags );
		}
		else
		{
			log_warn( "unknown room column '%s'", colname );
		}
	}

	load_exits( room );
	//load_reset( room );
}

int load_rooms( Area * area )
{
	char buf[400];
	sqlite3_stmt *stmt;
	int total = 0;

	int len = sprintf( buf, "select * from room where areaId=%" PRId64, area->id );

	if ( sqlite3_prepare( sqlite3_instance, buf, len, &stmt, 0 ) != SQLITE_OK )
	{
		log_sqlite3( "could not prepare statement" );
		return 0;
	}

	while ( sqlite3_step( stmt ) != SQLITE_DONE )
	{
		Room *room = new_room(  );

		room->area = area;

		load_room_columns( room, stmt );

		LINK( room_hash[room->id % ID_HASH], room, next );
		LINK( area->rooms, room, next_in_area );
		total++;
		max_room++;

		if ( is_explorable( room ) )
			max_explorable_room++;
	}

	if ( sqlite3_finalize( stmt ) != SQLITE_OK )
	{
		log_sqlite3( "could not finalize statement" );
	}

	return total;
}

Room *load_room( identifier_t id )
{
	char buf[400];
	sqlite3_stmt *stmt;
	Room *room = 0;

	int len = sprintf( buf, "select * from room where roomId=%"PRId64" limit 1", id );

	if ( sqlite3_prepare( sqlite3_instance, buf, len, &stmt, 0 ) != SQLITE_OK )
	{
		log_sqlite3( "could not prepare statement" );
		return 0;
	}

	if ( sqlite3_step( stmt ) != SQLITE_DONE )
	{
		room = new_room(  );

		load_room_columns( room, stmt );

		LINK( room_hash[room->id % ID_HASH], room, next );
		if ( room->area != 0 )
			LINK( room->area->rooms, room, next_in_area );

		max_room++;

		if ( is_explorable( room ) )
			max_explorable_room++;

	}

	if ( sqlite3_finalize( stmt ) != SQLITE_OK )
	{
		log_sqlite3( "could not finalize statement" );
	}

	return room;
}

int save_room_only( Room * room )
{
	char buf[OUT_SIZ * 4];

	struct dbvalues roomvals[] = {
		{"name", &room->name, SQLITE_TEXT},
		{"description", &room->description, SQLITE_TEXT},
		{"areaId", &room->area->id, SQLITE_INTEGER},
		{"sector", &room->sector, SQLITE_INTEGER},
		{"flags", &room->flags, DBTYPE_FLAG, room_flags},
		{"reset", &room->reset, SQLITE_TEXT},
		{0}
	};

	if ( room->id == 0 )
	{
		char names[BUF_SIZ] = { 0 };
		char values[OUT_SIZ * 4] = { 0 };

		build_insert_values( roomvals, names, values );

		sprintf( buf, "insert into room (%s) values(%s)", names, values );

		if ( sqlite3_exec( sqlite3_instance, buf, NULL, 0, 0 ) != SQLITE_OK )
		{
			log_sqlite3( "could not insert room" );
			return 0;
		}

		room->id = sqlite3_last_insert_rowid( sqlite3_instance );

	}
	else
	{
		char values[OUT_SIZ * 4] = { 0 };

		build_update_values( roomvals, values );

		sprintf( buf, "update room set %s where roomId=%" PRId64, values, room->id );

		if ( sqlite3_exec( sqlite3_instance, buf, NULL, 0, 0 ) != SQLITE_OK )
		{
			log_sqlite3( "could not update room" );
			return 0;
		}
	}

	return 1;
}

int save_room( Room * room )
{
	save_room_only( room );

	for ( int i = 0; i < MAX_DIR; i++ )
	{
		if ( room->exits[i] == 0 )
			continue;

		save_exit( room->exits[i], i );
	}

	return 1;
}

Room *get_room_by_id( identifier_t id )
{
	identifier_t hash = id % ID_HASH;

	for ( Room * room = room_hash[hash]; room != 0; room = room->next )
	{
		if ( room->id == id )
			return room;
	}
	return 0;
}

Room *room_lookup( const char *arg )
{
	if ( is_number( arg ) )
	{
		return get_room_by_id( atoi( arg ) );
	}

	for ( int i = 0; i < ID_HASH; i++ )
	{
		for ( Room * room = room_hash[i]; room != 0; room = room->next )
		{
			if ( !str_prefix( arg, strip_color( room->name ) ) )
				return room;
		}
	}
	return 0;
}

Room *get_default_room(  )
{

	for ( int i = 0; i < ID_HASH; i++ )
	{
		for ( Room * room = room_hash[i]; room != 0; room = room->next )
			return room;
	}

	Room *room = new_room(  );
	room->area = get_default_area(  );
	room->name = str_dup( "The Default Room" );
	room->description = str_dup( "This is the default room." );
	room->sector = SECT_UNKNOWN;

	save_room( room );

	LINK( room_hash[room->id % ID_HASH], room, next );
	if ( room->area != 0 )
		LINK( room->area->rooms, room, next_in_area );

	max_room++;

	if ( is_explorable( room ) )
		max_explorable_room++;

	return room;
}
