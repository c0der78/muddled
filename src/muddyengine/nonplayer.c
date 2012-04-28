
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
#include <muddyengine/nonplayer.h>
#include <muddyengine/db.h>
#include <muddyengine/string.h>
#include <muddyengine/log.h>
#include <muddyengine/engine.h>
#include <muddyengine/character.h>
#include <muddyengine/area.h>
#include <muddyengine/lookup.h>
#include <muddyengine/macro.h>
#include <muddyengine/race.h>

const Lookup npc_flags[] = {
	{NPC_SENTINEL, "sentinel"},
	{NPC_SCAVENGER, "scavenger"},
	{NPC_AGGRESSIVE, "aggressive"},
	{NPC_WIMPY, "wimpy"},
	{NPC_STAY_AREA, "stay_area"},
	{0, 0}
};

NPC *new_npc(  )
{
	NPC *npc = ( NPC * ) alloc_mem( 1, sizeof( NPC ) );

	npc->shortDescr = str_empty;
	npc->longDescr = str_empty;
	npc->startPosition = POS_STANDING;

	return npc;
}

void destroy_npc( NPC * npc )
{
	free_str( npc->shortDescr );
	free_str( npc->longDescr );

	free_mem( npc );
}

void load_npc_columns( Character * ch, sqlite3_stmt * stmt )
{
	int count = sqlite3_column_count( stmt );

	for ( int i = 0; i < count; i++ )
	{
		const char *colname = sqlite3_column_name( stmt, i );

		if ( load_char_column( ch, stmt, colname, i ) )
		{
		}
		else if ( !str_cmp( colname, "shortDescr" ) )
		{
			ch->npc->shortDescr = str_dup( sqlite3_column_str( stmt, i ) );
		}
		else if ( !str_cmp( colname, "longDescr" ) )
		{
			ch->npc->longDescr = str_dup( sqlite3_column_str( stmt, i ) );
		}
		else if ( !str_cmp( colname, "startPosition" ) )
		{
			ch->npc->startPosition = sqlite3_column_int( stmt, i );
		}
		else if ( !str_cmp( colname, "flags" ) )
		{
			parse_flags( ch->flags, sqlite3_column_str( stmt, i ), npc_flags );
		}
		else if ( !str_cmp( colname, "areaId" ) )
		{
			ch->npc->area = get_area_by_id( sqlite3_column_int( stmt, i ) );
		}
		else
		{
			log_warn( "unknown nonplayer column '%s'", colname );
		}
	}
}

Character *load_npc( identifier_t id )
{
	char buf[400];
	sqlite3_stmt *stmt;
	Character *ch = 0;

	int len = sprintf( buf,
					   "select * from character natural join nonplayer where charId=%" PRId64,
					   id );

	if ( sqlite3_prepare( sqlite3_instance, buf, len, &stmt, 0 ) != SQLITE_OK )
	{
		log_sqlite3( "could not prepare statement" );
		return 0;
	}

	if ( sqlite3_step( stmt ) != SQLITE_DONE )
	{
		ch = new_char(  );
		ch->npc = new_npc(  );

		load_npc_columns( ch, stmt );

		LINK( ch->npc->area->npcs, ch, next_in_area );
		LINK( first_character, ch, next );
	}

	if ( sqlite3_finalize( stmt ) != SQLITE_OK )
	{
		log_sqlite3( "could not finalize statement" );
	}

	return ch;
}

int load_npcs( Area * area )
{
	char buf[400];
	sqlite3_stmt *stmt;
	int total = 0;

	int len = sprintf( buf,
					   "select * from character natural join nonplayer where areaId=%"PRId64,
					   area->id );

	if ( sqlite3_prepare( sqlite3_instance, buf, len, &stmt, 0 ) != SQLITE_OK )
	{
		log_sqlite3( "could not prepare statement" );
		return 0;
	}

	while ( sqlite3_step( stmt ) != SQLITE_DONE )
	{
		Character *ch = new_char(  );
		ch->npc = new_npc(  );

		ch->npc->area = area;

		load_npc_columns( ch, stmt );

		LINK( area->npcs, ch, next_in_area );
		LINK( first_character, ch, next );
		total++;
	}

	if ( sqlite3_finalize( stmt ) != SQLITE_OK )
	{
		log_sqlite3( "could not finalize statement" );
	}

	return total;
}

int save_npc( Character * ch )
{
	char buf[OUT_SIZ * 2];

	struct dbvalues npcvalues[] = {
		{"shortDescr", &ch->npc->shortDescr, SQLITE_TEXT},
		{"longDescr", &ch->npc->longDescr, SQLITE_TEXT},
		{"startPosition", &ch->npc->startPosition, SQLITE_INTEGER},
		{"areaId", &ch->npc->area->id, SQLITE_INTEGER},
		{0, 0, 0}
	};

	int res = save_character( ch, npc_flags );

	if ( res == 1 )
	{
		char names[BUF_SIZ] = { 0 };
		char values[OUT_SIZ] = { 0 };

		build_insert_values( npcvalues, names, values );

		sprintf( buf, "insert into nonplayer (charId,%s) values(%"PRId64",%s)",
				 names, ch->id, values );

		if ( sqlite3_exec( sqlite3_instance, buf, NULL, 0, 0 ) != SQLITE_OK )
		{
			log_sqlite3( "could not insert player" );
			return 0;
		}
	}
	else if ( res == 2 )
	{
		char values[OUT_SIZ] = { 0 };

		build_update_values( npcvalues, values );

		sprintf( buf, "update nonplayer set %s where charId=%"PRId64, values,
				 ch->id );

		if ( sqlite3_exec( sqlite3_instance, buf, NULL, 0, 0 ) != SQLITE_OK )
		{
			log_sqlite3( "could not update character" );
			return 0;
		}
	}

	return UMIN( res, 1 );
}

int delete_npc( Character * ch )
{
	char buf[BUF_SIZ];

	if ( !delete_character( ch ) )
		return 0;

	sprintf( buf, "delete from nonplayer where charId=%"PRId64, ch->id );

	if ( sqlite3_exec( sqlite3_instance, buf, NULL, 0, 0 ) != SQLITE_OK )
	{
		log_sqlite3( "could not delete player" );
		return 0;
	}

	return 1;
}

Character *npc_lookup( const char *arg )
{
	if ( is_number( arg ) )
	{
		return get_npc_by_id( atoi( arg ) );
	}
	else
	{
		for ( Character * nch = first_character; nch != 0; nch = nch->next )
		{
			if ( nch->npc == 0 )
				continue;

			if ( is_name( arg, nch->name ) )
				return nch;
		}
		return 0;
	}
}

Character *get_npc_by_id( identifier_t id )
{
	for ( Character * nch = first_character; nch != 0; nch = nch->next )
	{
		if ( nch->npc == 0 )
			continue;

		if ( nch->id == id )
			return nch;
	}
	return 0;
}
