
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

#include <muddyengine/help.h>
#include <stdlib.h>
#include <inttypes.h>
#include <muddyengine/string.h>
#include <muddyengine/db.h>
#include <stdio.h>
#include <muddyengine/log.h>
#include <muddyengine/macro.h>
#include <muddyengine/engine.h>

const char *greeting = 0;

Help *first_help = 0;

const Lookup help_categories[] = {
	{HELP_UNKNOWN, "unknown"},
	{HELP_INFO, "information"},
	{HELP_COMMUNICATION, "communication"},
	{0, 0}
};

Help *new_help(  )
{

	Help *help = ( Help * ) alloc_mem( 1, sizeof( Help ) );

	help->keywords = str_empty;

	help->syntax = str_empty;

	help->text = str_empty;

	help->related = 0;

	help->category = HELP_UNKNOWN;

	return help;

}

void destroy_help( Help * help )
{

	free_str( help->keywords );

	free_str( help->text );

	free_str( help->syntax );

	free_mem( help );

}

int
load_help_column( Help * help, db_stmt * stmt, const char *colname, int i )
{

	if ( !str_cmp( colname, "keywords" ) )
	{

		help->keywords = str_dup( db_column_str( stmt, i ) );

		return 1;

	}
	else if ( !str_cmp( colname, "syntax" ) )
	{

		help->syntax = str_dup( db_column_str( stmt, i ) );

		return 1;

	}
	else if ( !str_cmp( colname, "text" ) )
	{

		help->text = str_dup( db_column_str( stmt, i ) );

		return 1;

	}
	else if ( !str_cmp( colname, "helpId" ) )
	{

		help->id = db_column_int64( stmt, i );

		return 1;

	}
	else if ( !str_cmp( colname, "category" ) )
	{

		help->category = db_column_int( stmt, i );

		return 1;

	}
	else
	{
		log_warn( "unknown help column '%s'", colname );

	}

	return 0;

}

int load_related_helps(  )
{

	for ( Help * help = first_help; help; help = help->next )

	{

		char buf[400];

		db_stmt *stmt;

		int len = sprintf( buf, "select * from help_related where helpId=%"PRId64,
						   help->id );

		if ( db_query( buf,  len,  &stmt) !=
			 DB_OK )
		{

			log_data( "could not prepare statement" );

			return 0;

		}

		while ( db_step( stmt ) != SQLITE_DONE )
		{

			int related = db_column_int( stmt, 1 );

			for ( Help * rel = first_help; help; help = help->next )

			{

				if ( rel->id == related )
				{

					LINK( help->related, rel, next_related );

					break;

				}

			}

		}

		if ( db_finalize( stmt ) != DB_OK )
		{

			log_data( "could not finalize statement" );

		}

	}

	return 1;

}

int load_helps(  )
{

	char buf[400];

	db_stmt *stmt;

	int total = 0;

	int len = sprintf( buf, "select * from help" );

	if ( db_query( buf,  len,  &stmt) != DB_OK )
	{

		log_data( "could not prepare statement" );

		return 0;

	}

	while ( db_step( stmt ) != SQLITE_DONE )
	{

		int count = db_column_count( stmt );

		Help *help = new_help(  );

		for ( int i = 0; i < count; i++ )
		{

			const char *colname = db_column_name( stmt, i );

			load_help_column( help, stmt, colname, i );

		}

		if ( is_name( "greeting", help->keywords ) )
		{

			greeting = help->text;

		}

		LINK( first_help, help, next );

		total++;

	}

	if ( db_finalize( stmt ) != DB_OK )
	{

		log_data( "could not finalize statement" );

	}

	load_related_helps(  );

	return total;

}

int save_help( Help * help )
{

	char buf[OUT_SIZ * 3];

	struct dbvalues helpvals[] = {
		{"keywords", &help->keywords, SQLITE_TEXT},
		{"text", &help->text, SQLITE_TEXT},
		{"related", &help->related, SQLITE_TEXT},
		{"category", &help->category, SQLITE_INTEGER},
		{0}
	};

	if ( help->id == 0 )
	{

		char names[BUF_SIZ] = { 0 };

		char values[OUT_SIZ * 3] = { 0 };

		build_insert_values( helpvals, names, values );

		sprintf( buf, "insert into help (%s) values(%s)", names, values );

		if ( db_exec( buf) != DB_OK )
		{

			log_data( "could not insert help" );

			return 0;

		}

		help->id = db_last_insert_rowid();

	}
	else
	{

		char values[OUT_SIZ * 3] = { 0 };

		build_update_values( helpvals, values );

		sprintf( buf, "update help set %s where helpId=%" PRId64, values, help->id );

		if ( db_exec( buf) != DB_OK )
		{

			log_data( "could not update help" );

			return 0;

		}

	}

	return 1;

}

Help *help_find( const char *keyword )
{

	int id = atoi( keyword );

	for ( Help * help = first_help; help != 0; help = help->next )
	{

		if ( ( id != 0 && id == help->id )
			 || is_name( keyword, help->keywords ) )

			return help;

	}

	return 0;

}

Help *help_match( const char *keyword )
{

	for ( Help * help = first_help; help != 0; help = help->next )
	{
		char arg[BUF_SIZ];

		const char *key = one_argument( help->keywords, arg );

		do
		{
			if ( match( arg, keyword ) )
				return help;

			key = one_argument( key, arg );
		}
		while ( !nullstr( key ) );

	}

	return 0;

}

const char *help_related_string( Help * help )
{

	static char buf[BUF_SIZ] = { 0 };

	for ( Help * rel = help->related; rel; rel = rel->next_related )

	{

		strcat( buf, ", " );

		strcat( buf, rel->keywords );

	}

	return &buf[2];

}
