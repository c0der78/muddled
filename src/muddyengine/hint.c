
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

#include <muddyengine/db.h>
#include <muddyengine/hint.h>
#include <muddyengine/engine.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <muddyengine/string.h>
#include <muddyengine/log.h>
#include <muddyengine/macro.h>

Hint *hint_table = 0;

int max_hint = 0;

Hint *new_hint(  )
{

	Hint *hint = ( Hint * ) alloc_mem( 1, sizeof( Hint ) );

	hint->text = str_empty;

	return hint;

}

void destroy_hint( Hint * hint )
{

	free_str( hint->text );

	free_mem( hint );

}

int load_hints(  )
{

	char buf[400];

	sqlite3_stmt *stmt;

	int total = 0;

	int len = sprintf( buf, "select count(*) from hint" );

	if ( sqlite3_prepare( sqlite3_instance, buf, len, &stmt, 0 ) != SQLITE_OK )
	{

		log_sqlite3( "could not prepare statement" );

		return 0;

	}

	if ( sqlite3_step( stmt ) == SQLITE_DONE )
	{

		log_sqlite3( "could not count hints" );

		return 0;

	}

	max_hint = sqlite3_column_int( stmt, 0 );

	if ( sqlite3_finalize( stmt ) != SQLITE_OK )
	{

		log_sqlite3( "could not finalize statement" );

	}

	hint_table = ( Hint * ) alloc_mem( max_hint, sizeof( Hint ) );

	len = sprintf( buf, "select * from hint" );

	if ( sqlite3_prepare( sqlite3_instance, buf, len, &stmt, 0 ) != SQLITE_OK )
	{

		log_sqlite3( "could not prepare statement" );

		return 0;

	}

	while ( sqlite3_step( stmt ) != SQLITE_DONE )
	{

		int count = sqlite3_column_count( stmt );

		for ( int i = 0; i < count; i++ )
		{

			const char *colname = sqlite3_column_name( stmt, i );

			if ( !str_cmp( colname, "text" ) )
			{

				hint_table[total].text =
					str_dup( sqlite3_column_str( stmt, i ) );

			}
			else if ( !str_cmp( colname, "hintId" ) )
			{

				hint_table[total].id = sqlite3_column_int( stmt, i );

			}
			else if ( !str_cmp( colname, "level" ) )
			{

				hint_table[total].level = sqlite3_column_int( stmt, i );

			}
			else
			{

				log_warn( "unknown hint column '%s'", colname );

			}

		}

		total++;

	}

	if ( sqlite3_finalize( stmt ) != SQLITE_OK )
	{

		log_sqlite3( "could not finalize statement" );

	}

	if ( total != max_hint )
	{

		log_warn( "counted hints did not match number read" );

	}

	return total;

}

int save_hint( Hint * hint )
{

	char buf[OUT_SIZ];

	struct dbvalues hintvals[] = {
		{"text", &hint->text, SQLITE_TEXT},
		{"level", &hint->level, SQLITE_INTEGER},
		{0}
	};

	if ( hint->id == 0 )
	{

		char names[BUF_SIZ] = { 0 };

		char values[OUT_SIZ] = { 0 };

		build_insert_values( hintvals, names, values );

		sprintf( buf, "insert into hint (%s) values(%s)", names, values );

		if ( sqlite3_exec( sqlite3_instance, buf, NULL, 0, 0 ) != SQLITE_OK )
		{

			log_sqlite3( "could not insert hint" );

			return 0;

		}

		hint->id = sqlite3_last_insert_rowid( sqlite3_instance );

	}
	else
	{

		char values[OUT_SIZ] = { 0 };

		build_update_values( hintvals, values );

		sprintf( buf, "update hint set %s where hintId=%" PRId64, values, hint->id );

		if ( sqlite3_exec( sqlite3_instance, buf, NULL, 0, 0 ) != SQLITE_OK )
		{

			log_sqlite3( "could not update hint" );

			return 0;

		}

	}

	return 1;

}
