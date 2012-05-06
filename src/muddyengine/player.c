
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
#include <math.h>
#include <inttypes.h>
#include <muddyengine/flag.h>
#include <muddyengine/player.h>
#include <muddyengine/string.h>
#include <muddyengine/character.h>
#include <muddyengine/log.h>
#include <muddyengine/class.h>
#include <stdio.h>
#include <muddyengine/room.h>
#include <muddyengine/account.h>
#include <muddyengine/macro.h>
#include <muddyengine/util.h>
#include <muddyengine/race.h>
#include <muddyengine/connection.h>
#include <muddyengine/lookup.h>
#include <muddyengine/engine.h>
#include <muddyengine/explored.h>
#include <muddyengine/channel.h>

Character *first_player = 0;

const Lookup plr_flags[] = {
	{ "ticksoff", PLR_TICKS_OFF},
	{ "brief", PLR_BRIEF},
	{ "hints", PLR_HINTS},
	{ "automapoff", PLR_AUTOMAP_OFF},
	{0, 0}
};

Player *new_player( Connection * conn )
{

	Player *p = ( Player * ) alloc_mem( 1, sizeof( Player ) );

	p->title = str_empty;

	p->prompt = str_dup( "[~R%h~x/~R%H~rhp ~M%m~x/~M%M~mm ~B%v/~B%V~bmv~x]" );

	p->battlePrompt =
		str_dup( "[~R%h~x/~R%H~rhp ~M%m~x/~M%M~mm ~B%v/~B%V~bmv~x] %E" );

	p->conn = conn;

	p->account = conn->account;

	p->explored = new_flag(  );

	p->channels = new_flag(  );

	p->permHit = 100;

	p->permMana = 100;

	p->permMove = 100;

	return p;

}

void destroy_player( Player * p )
{

	free_str( p->title );

	free_str( p->prompt );

	free_str( p->battlePrompt );

	destroy_flags( p->explored );

	destroy_flags( p->channels );

	free_mem( p );

}

int delete_player( Character * ch )
{

	char buf[BUF_SIZ];

	if ( !delete_character( ch ) )

		return 0;

	sprintf( buf, "delete from player where charId=%" PRId64, ch->id );

	if ( db_exec( buf) != DB_OK )
	{

		log_data( "could not delete player" );

		return 0;

	}

	return 1;

}

static const char *save_explored( void *arg )
{

	return get_explored_rle( *( ( Flag ** ) arg ) );

}

static const char *save_condition( void *arg )
{

	int *cond = ( int * ) arg;

	static char buf[BUF_SIZ];

	int len = 0;

	for ( int i = 0; i < MAX_COND; i++ )

		len += sprintf( &buf[len], "%d,", cond[i] );

	if ( len > 0 )

		buf[len - 1] = 0;

	return buf;

}

static void read_condition( void *data, db_stmt * stmt, int col )
{

	int *cond = ( int * ) data;

	int index = 0;

	const char *pstr =
		strtok( ( char * ) db_column_str( stmt, col ), "," );

	while ( pstr )

	{

		cond[index++] = atoi( pstr );

		pstr = strtok( NULL, "," );

	}

}

int save_player( Character * ch )
{ 

	char buf[OUT_SIZ * 2];

	if ( ch->pc == 0 )
	{

		log_error( "character not a player" );

		return 0;

	}

	db_begin_transaction(  );

	if ( !save_account( ch->pc->account ) )
	{

		return 0;

	}

	struct dbvalues pcvalues[] = {
		{"accountId", &ch->pc->account->id, DB_INTEGER},
		{ "title", &ch->pc->title, DB_TEXT},
		{ "roomId", ( ch->inRoom ? &ch->inRoom->id : 0 ), DB_INTEGER},
		{ "prompt", &ch->pc->prompt, DB_TEXT},
		{ "battlePrompt", &ch->pc->battlePrompt, DB_TEXT},
		{ "explored", &ch->pc->explored, DBTYPE_CUSTOM, save_explored},
		{ "channels", &ch->pc->channels, DB_FLAG, channel_flags},
		{ "condition", &ch->pc->condition, DBTYPE_CUSTOM, save_condition},
		{ "experience", &ch->pc->experience, DB_INTEGER},
		{ "permHit", &ch->pc->permHit, DB_INTEGER},
		{ "permMana", &ch->pc->permMana, DB_INTEGER},
		{ "permMove", &ch->pc->permMove, DB_INTEGER},
		{ "created", &ch->pc->created, DB_INTEGER},
		{0}
	};

	int res = save_character( ch, plr_flags );

	if ( res == 1 )
	{

		char names[BUF_SIZ] = { 0 };

		char values[OUT_SIZ] = { 0 };

		build_insert_values( pcvalues, names, values );

		sprintf( buf, "insert into player (charId,%s) values(%"PRId64",%s)",
				 names, ch->id, values );

		if ( db_exec( buf) != DB_OK )
		{

			log_data( "could not insert player" );

			return 0;

		}

	}
	else if ( res == 2 )
	{

		char values[OUT_SIZ] = { 0 };

		build_update_values( pcvalues, values );

		sprintf( buf, "update player set %s where charId=%"PRId64, values, ch->id );

		if ( db_exec( buf) != DB_OK )
		{

			log_data( "could not update character" );

			return 0;

		}

	}

	if ( !save_char_objs( ch ) )

		res = 0;

	if ( !save_char_affects( ch ) )

		res = 0;

	db_end_transaction(  );

	return UMIN( res, 1 );

}

void load_player_columns( Account * acc, Character * ch, db_stmt * stmt )
{

	int i, cols = db_column_count( stmt );

	for ( i = 0; i < cols; i++ )
	{

		const char *colname = db_column_name( stmt, i );

		if ( load_char_column( ch, stmt, colname, i ) )
		{

		}
		else if ( !str_cmp( colname, "title" ) )
		{

			ch->pc->title = str_dup( db_column_str( stmt, i ) );

		}
		else if ( !str_cmp( colname, "prompt" ) )
		{

			free_str( ch->pc->prompt );

			ch->pc->prompt = str_dup( db_column_str( stmt, i ) );

		}
		else if ( !str_cmp( colname, "battlePrompt" ) )
		{

			free_str( ch->pc->battlePrompt );

			ch->pc->battlePrompt = str_dup( db_column_str( stmt, i ) );

		}
		else if ( !str_cmp( colname, "accountId" ) )
		{

			if ( acc && acc->id != db_column_int( stmt, i ) )

				log_error( "sql returned invalid account for player" );

		}
		else if ( !str_cmp( colname, "flags" ) )
		{

			parse_flags( ch->flags, db_column_str( stmt, i ), plr_flags );

		}
		else if ( !str_cmp( colname, "roomId" ) )
		{

			ch->inRoom = get_room_by_id( db_column_int( stmt, i ) );

		}
		else if ( !str_cmp( colname, "explored" ) )
		{

			convert_explored_rle( ch->pc->explored,
								  db_column_str( stmt, i ) );

		}
		else if ( !str_cmp( colname, "channels" ) )
		{

			parse_flags( ch->pc->channels,
						 db_column_str( stmt, i ), channel_flags );

		}
		else if ( !str_cmp( colname, "condition" ) )
		{

			read_condition( ch->pc->condition, stmt, i );

		}
		else if ( !str_cmp( colname, "experience" ) )
		{

			ch->pc->experience = db_column_int( stmt, i );

		}
		else if ( !str_cmp( colname, "permHit" ) )
		{

			ch->pc->permHit = db_column_int( stmt, i );

		}
		else if ( !str_cmp( colname, "permMana" ) )
		{

			ch->pc->permMana = db_column_int( stmt, i );

		}
		else if ( !str_cmp( colname, "permMove" ) )
		{

			ch->pc->permMove = db_column_int( stmt, i );

		}
		else if ( !str_cmp( colname, "created" ) )
		{

			ch->pc->created = db_column_int( stmt, i );

		}
		else
		{

			log_warn( "unknown player column '%s'", colname );

		}

	}

}

Character *load_player_by_id( Connection * conn, identifier_t charId )
{

	char buf[400];

	db_stmt *stmt;

	db_begin_transaction(  );

	int len = sprintf( buf,
					   "select * from character natural join player where charId=%"PRId64,
					   charId );

	if ( db_query( buf,  len,  &stmt) != DB_OK )
	{

		log_data( "could not prepare sql statement" );

		return 0;

	}

	Character *ch = new_char(  );

	ch->pc = new_player( conn );

	if ( db_step( stmt ) != DB_DONE )
	{

		load_player_columns( conn->account, ch, stmt );

	}

	if ( db_finalize( stmt ) != DB_OK )
	{

		log_data( "unable to finalize statement" );

	}

	load_char_objs( ch );

	load_char_affects( ch );

	db_end_transaction(  );

	return ch;

}

Character *load_player_by_name( Connection * conn, const char *name )
{

	char buf[400];

	db_stmt *stmt;

	db_begin_transaction(  );

	int len = sprintf( buf,
					   "select * from character natural join player where name='%s'",
					   escape_db_str( name ) );

	if ( db_query( buf,  len,  &stmt) != DB_OK )
	{

		log_data( "could not prepare sql statement" );

		return 0;

	}

	Character *ch = new_char(  );

	ch->pc = new_player( conn );

	if ( db_step( stmt ) != DB_DONE )
	{

		load_player_columns( conn->account, ch, stmt );

	}

	if ( db_finalize( stmt ) != DB_OK )
	{

		log_data( "unable to finalize statement" );

	}

	load_char_objs( ch );

	load_char_affects( ch );

	db_end_transaction(  );

	return ch;

}

Character *get_player_by_id( int id )
{

	for ( Character * ch = first_player; ch != 0; ch = ch->next_player )
	{

		if ( ch->id == id )

			return ch;

	}

	return 0;

}

Character *player_lookup( const char *arg )
{

	if ( is_number( arg ) )
	{

		return get_player_by_id( atoi( arg ) );

	}
	else
	{

		for ( Character * nch = first_player; nch != 0; nch = nch->next_player )
		{

			if ( is_name( arg, nch->name ) )

				return nch;

		}

		return 0;

	}

}

long exp_to_level( const Character * ch )
{

	if ( !ch->pc )
		return 0;

	double total = 0;

	int mod = UMIN( MAX_PLAYABLE_LEVEL, ch->level ) / LEVEL_GROUPS;

	for ( int a = 0; a < ch->level * class_count( ch ); a++ )

	{

		double t = ( pow( a, 2 ) * exp_table[mod] );

		total += t;

	}

	return ( long ) total;

}

bool is_drunk( Character * ch )
{

	return ch->pc->condition[COND_DRUNK] > 10;

}
