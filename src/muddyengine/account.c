
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
#include <stdio.h>
#include <muddyengine/account.h>
#include <muddyengine/string.h>
#include <muddyengine/db.h>
#include <muddyengine/log.h>
#include <muddyengine/character.h>
#include <muddyengine/player.h>
#include <muddyengine/engine.h>
#include <stdlib.h>
#include <inttypes.h>
#include <muddyengine/forum.h>
#include <muddyengine/lookup.h>
#include <muddyengine/flag.h>

const Lookup account_flags[] = {
	{ACC_COLOR_OFF, "coloroff"},
	{0, 0}
};

Account *new_account( Connection * conn )
{
	Account *acc = ( Account * ) alloc_mem( 1, sizeof( Account ) );

	acc->login = str_empty;
	acc->password = str_empty;
	acc->email = str_empty;

	acc->conn = conn;

	acc->forum = &forum_table[0];

	acc->flags = new_flag(  );

	acc->lastNote = ( time_t * ) alloc_mem( max_forum, sizeof( time_t ) );

	return acc;
}

void destroy_account( Account * acc )
{
	free_str( acc->login );
	free_str( acc->password );
	free_str( acc->email );

	destroy_flags( acc->flags );

	free_mem( acc->lastNote );

	for ( AccountPlayer * ch_next, *ch = acc->players; ch != 0; ch = ch_next )
	{
		ch_next = ch->next;
		destroy_account_player( ch );
	}

	if ( acc->playing )
	{
		extract_char( acc->playing, true );
	}
	free_mem( acc );
}

AccountPlayer *new_account_player(  )
{
	AccountPlayer *p =
		( AccountPlayer * ) alloc_mem( 1, sizeof( AccountPlayer ) );

	p->name = str_empty;

	return p;
}

void destroy_account_player( AccountPlayer * p )
{
	free_str( p->name );

	free_mem( p );
}

static const char *save_last_note( void *arg )
{
	static char buf[OUT_SIZ];
	time_t *data = *( ( time_t ** ) arg );
	int len = 0;

	buf[0] = 0;

	for ( int i = 0; i < max_forum; i++ )
	{
		len += sprintf( &buf[len], "%s=%ld,", forum_table[i].name, data[i] );
	}

	if ( len > 0 )
		buf[len - 1] = 0;

	return buf;
}

static void read_last_note( void *data, db_stmt * stmt, int i )
{
	const char *pstr = strtok( ( char * ) db_column_str( stmt, i ), "=" );

	time_t *lastNote = ( time_t * ) data;

	while ( pstr != 0 )
	{
		int f = forum_lookup( pstr );

		if ( f != FORUM_ERROR )
		{
			const char *val = strtok( NULL, "," );

			lastNote[f] = atol( val );
		}

		pstr = strtok( NULL, "=" );
	}
}

int load_account( Account * acc, const char *login )
{
	char buf[500];
	db_stmt *stmt;
	int len = sprintf( buf, "select * from account where login='%s'", login );

	log_debug( "loading account %s", login );

	if ( db_query( buf,  len,  &stmt) != DB_OK )
	{
		log_data( "could not prepare sql statement" );
		return 0;
	}

	if ( db_step( stmt ) == SQLITE_DONE )
	{
		if ( db_finalize( stmt ) != DB_OK )
			log_data( "could not finalize sql statement" );
		return 0;
	}

	do
	{
		int i, cols = db_column_count( stmt );
		for ( i = 0; i < cols; i++ )
		{
			const char *colname = db_column_name( stmt, i );

			if ( !str_cmp( colname, "accountId" ) )
			{
				acc->id = db_column_int( stmt, i );
			}
			else if ( !str_cmp( colname, "login" ) )
			{
				acc->login = str_dup( db_column_str( stmt, i ) );
			}
			else if ( !str_cmp( colname, "email" ) )
			{
				acc->email = str_dup( db_column_str( stmt, i ) );
			}
			else if ( !str_cmp( colname, "timezone" ) )
			{
				acc->timezone = db_column_int( stmt, i );
			}
			else if ( !str_cmp( colname, "autologinId" ) )
			{
				acc->autologinId = db_column_int( stmt, i );
			}
			else if ( !str_cmp( colname, "flags" ) )
			{
				parse_flags( acc->flags,
							 db_column_str( stmt, i ), account_flags );
			}
			else if ( !str_cmp( colname, "password" ) )
			{
				acc->password = str_dup( db_column_str( stmt, i ) );
			}
			else if ( !str_cmp( colname, "lastNote" ) )
			{
				read_last_note( acc->lastNote, stmt, i );
			}
			else
			{
				log_warn( "unknown account column '%s'", colname );
			}
		}
	}
	while ( db_step( stmt ) != SQLITE_DONE );

	// load account players

	load_account_players( acc );

	if ( db_finalize( stmt ) != DB_OK )
	{
		log_data( "could not finalize sql statement" );
	}

	return 1;
}

int save_account( Account * acc )
{
	char buf[BUF_SIZ];

	struct dbvalues accvalues[] = {
		{"login", &acc->login, SQLITE_TEXT},
		{"email", &acc->email, SQLITE_TEXT},
		{"password", &acc->password, SQLITE_TEXT},
		{"timezone", &acc->timezone, SQLITE_INTEGER},
		{"autologinId", &acc->autologinId, SQLITE_INTEGER},
		{"lastNote", &acc->lastNote, DBTYPE_CUSTOM, save_last_note},
		{"flags", &acc->flags, DBTYPE_FLAG, account_flags},
		{0, 0, 0}
	};

	if ( acc->id == 0 )
	{
		char names[BUF_SIZ] = { 0 };
		char values[OUT_SIZ] = { 0 };

		build_insert_values( accvalues, names, values );

		sprintf( buf, "insert into account (%s) values(%s)", names, values );

		if ( db_exec( buf) != DB_OK )
		{
			log_data( "could not insert character" );
			return 0;
		}

		acc->id = db_last_insert_rowid();
	}
	else
	{
		char values[OUT_SIZ] = { 0 };

		build_update_values( accvalues, values );

		sprintf( buf, "update account set %s where accountId=%" PRId64, values,
				 acc->id );

		if ( db_exec( buf) != DB_OK )
		{
			log_data( "could not update character" );
			return 0;
		}
	}

	return 1;
}

int delete_account( Account * acc )
{

	char buf[500];
	db_stmt *stmt;
	int len = sprintf( buf, "delete from account where accountId=%" PRId64, acc->id );

	log_debug( "deleting account %s", acc->login );

	if ( db_query( buf,  len,  &stmt) != DB_OK )
	{
		log_data( "could not prepare sql statement" );
		return 0;
	}

	if ( db_step( stmt ) != SQLITE_DONE )
	{
		if ( db_finalize( stmt ) != DB_OK )
			log_data( "could not finalize sql statement" );
		return 0;
	}

	if ( db_finalize( stmt ) != DB_OK )
	{
		log_data( "could not finalize sql statement" );
	}

	return 1;
}

int load_account_players( Account * acc )
{
	char buf[400];
	db_stmt *stmt;

	int len = sprintf( buf,
					   "select * from character natural join player where accountId=%" PRId64,
					   acc->id );

	if ( db_query( buf,  len,  &stmt) != DB_OK )
	{
		log_data( "could not prepare sql statement" );
		return 0;
	}

	if ( db_step( stmt ) == SQLITE_DONE )
	{
		if ( db_finalize( stmt ) != DB_OK )
			log_data( "could not finalize sql statement" );
		return 0;
	}

	AccountPlayer *ch = new_account_player(  );

	do
	{
		int i, cols = db_column_count( stmt );

		for ( i = 0; i < cols; i++ )
		{
			const char *colname = db_column_name( stmt, i );

			if ( !str_cmp( colname, "name" ) )
			{
				ch->name = str_dup( db_column_str( stmt, i ) );
			}
			else if ( !str_cmp( colname, "level" ) )
			{
				ch->level = db_column_int( stmt, i );
			}
			else if ( !str_cmp( colname, "accountId" ) )
			{
				if ( acc->id != db_column_int( stmt, i ) )
				{
					log_error( "sql retrieved invalid player for account" );
					break;
				}
				LINK( acc->players, ch, next );
			}
			else if ( !str_cmp( colname, "charId" ) )
			{
				ch->charId = db_column_int( stmt, i );
			}
		}
	}
	while ( db_step( stmt ) != SQLITE_DONE );

	if ( db_finalize( stmt ) != DB_OK )
	{
		log_data( "unable to finalize statement" );
	}

	return 1;

}
