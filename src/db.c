
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
#include <stdarg.h>
#include <muddyengine/db.h>
#include <muddyengine/log.h>
#include <muddyengine/engine.h>
#include <muddyengine/string.h>
#include <string.h>
#include <stdio.h>
#include "config.h"

sqlite3 *sqlite3_instance = 0;

void init_sqlite3(  )
{
	int rc = sqlite3_open( SQLITE3_FILE, &sqlite3_instance );	//, SQLITE_OPEN_READWRITE|SQLITE_OPEN_FULLMUTEX, 0);
	if ( rc )
	{
		log_sqlite3( "Can't open database" );
		sqlite3_close( sqlite3_instance );
		exit( EXIT_FAILURE );
	}
}

void close_sqlite3(  )
{
	if ( sqlite3_instance != 0 )
	{
		log_info( "shutting down database" );
		sqlite3_close( sqlite3_instance );
		sqlite3_instance = 0;
	}
}

const char *escape_db_str( const char *pstr )
{
	static char buf[OUT_SIZ * 3];

	char *pbuf = buf;

	while ( pstr && *pstr )
	{
		if ( *pstr == '\'' )
			*pbuf++ = '\'';

		*pbuf++ = *pstr++;
	}

	*pbuf = 0;

	return buf;
}

void set_db_values( struct dbvalues *table, char *values )
{
	if ( table->value == 0 )
	{
		strcat( values, "null," );
		return;
	}
	switch ( table->type )
	{
		case SQLITE_INTEGER:
			sprintf( &values[strlen( values )], "%d",
					 *( ( int * ) table->value ) );
			strcat( values, "," );
			break;
		case SQLITE_TEXT:
			strcat( values, "'" );
			strcat( values,
					escape_db_str( *( ( const char ** ) table->value ) ) );
			strcat( values, "'," );
			break;
		case SQLITE_FLOAT:
			sprintf( &values[strlen( values )], "%f",
					 *( ( double * ) table->value ) );
			strcat( values, "," );
			break;
		case DBTYPE_FLOAT:
			sprintf( &values[strlen( values )], "%f",
					 *( ( float * ) table->value ) );
			strcat( values, "," );
			break;
		case DBTYPE_CUSTOM:
		{
			map_save_t func = ( map_save_t ) ( table->arg1 );
			strcat( values, "'" );
			strcat( values, escape_db_str( ( *func ) ( table->value ) ) );
			strcat( values, "'," );
		}
			break;
		case DBTYPE_ARRAY:
		{
			map_save_array_t func = ( map_save_array_t ) ( table->arg1 );
			strcat( values, "'" );
			strcat( values, ( *func ) ( *( int* ) table->arg2, table->value ) );
			strcat( values, "'," );
		}
			break;
		case DBTYPE_FLAG:
			strcat( values, "'" );
			strcat( values,
					format_flags( *( ( Flag ** ) table->value ),
								  ( const Lookup * ) table->arg1 ) );
			strcat( values, "'," );
			break;
		default:
			log_bug( "unknown save type for field %s", table->name );
			break;
	}
}

void
save_field_values( table_map * table, field_map * field, char *values,
				   void *data )
{
	if ( field->value == 0 )
	{
		strcat( values, "null," );
		return;
	}
	switch ( field->type )
	{
		case SQLITE_INTEGER:
			sprintf( &values[strlen( values )], "%d",
					 field_int( field->value, table->base, data ) );
			strcat( values, "," );
			break;
		case SQLITE_TEXT:
			strcat( values, "'" );
			strcat( values,
					escape_db_str( field_str
								   ( field->value, table->base, data ) ) );
			strcat( values, "'," );
			break;
		case SQLITE_FLOAT:
			sprintf( &values[strlen( values )], "%f",
					 field_double( field->value, table->base, data ) );
			strcat( values, "," );
			break;
		case DBTYPE_CUSTOM:
		{
			map_save_t func = ( map_save_t ) ( field->arg1 );
			strcat( values, "'" );
			strcat( values, escape_db_str( ( *func ) ( field->value ) ) );
			strcat( values, "'," );
		}
			break;
		case DBTYPE_ARRAY:
		{
			map_save_array_t func = ( map_save_array_t ) ( field->arg1 );
			strcat( values, "'" );
			strcat( values, ( *func ) ( *( int* ) field->arg2, field->value ) );
			strcat( values, "'," );
		}
			break;
		case DBTYPE_FLAG:
			strcat( values, "'" );
			strcat( values,
					format_flags( field_flag
								  ( field->value, table->base, data ),
								  ( const Lookup * ) field->arg1 ) );
			strcat( values, "'," );
			break;
		default:
			log_bug( "unknown save type for %s field %s", table->name,
					 field->name );
			break;
	}
}

const char *db_save_int_array( int max, void *arg )
{
	int *values = ( int * ) arg;
	static char buf[BUF_SIZ];
	int len = 0;

	for ( int i = 0; i < max; i++ )
	{
		len += sprintf( &buf[len], "%d,", ( values )[i] );
	}

	if ( len > 0 )
		buf[len - 1] = 0;

	return buf;
}

void db_read_int_array( int max, void *arg, sqlite3_stmt * stmt, int i )
{
	int *values = ( int * ) arg;
	const char *value = strtok( ( char * ) sqlite3_column_str( stmt, i ), "," );
	int index = 0;

	while ( value )
	{
		if ( index >= max )
			break;

		( values )[index++] = atoi( value );

		value = strtok( NULL, "," );
	}
}

void load_field_values(  )
{
}

sqlite3_stmt *db_select_all( table_map * table, ... )
{
	char buf[500];
	sqlite3_stmt *stmt;
	va_list args;
	table_map *arg;
	int len = sprintf( buf, "select * from %s", table->name );

	va_start( args, table );

	while ( ( arg = va_arg( args, table_map * ) ) != 0 )
	{
		len += sprintf( &buf[len], ",%s", arg->name );
	}
	va_end( args );

	if ( sqlite3_prepare( sqlite3_instance, buf, len, &stmt, 0 ) != SQLITE_OK )
	{
		log_sqlite3( "could not prepare sql statement" );
		return 0;
	}

	return stmt;
}

sqlite3_stmt *db_select_where( const char *fields, const char *where,
							   table_map * table, ... )
{
	char buf[BUF_SIZ];
	sqlite3_stmt *stmt;
	va_list args;
	table_map *arg;
	int len = sprintf( buf, "select %s from %s", fields, table->name );

	va_start( args, table );

	while ( ( arg = va_arg( args, table_map * ) ) != 0 )
	{
		len += sprintf( &buf[len], ",%s", arg->name );
	}
	va_end( args );

	if ( !nullstr( where ) )
	{
		len += sprintf( &buf[len], " %s", where );
	}

	if ( sqlite3_prepare( sqlite3_instance, buf, len, &stmt, 0 ) != SQLITE_OK )
	{
		log_sqlite3( "could not prepare sql statement" );
		return 0;
	}

	return stmt;
}

void build_insert_values( struct dbvalues *table, char *names, char *values )
{
	for ( int i = 0; table[i].name != 0; i++ )
	{
		strcat( names, table[i].name );
		strcat( names, "," );
	}

	if ( names[0] != 0 )
		names[strlen( names ) - 1] = 0;

	for ( int i = 0; table[i].name != 0; i++ )
	{
		set_db_values( &table[i], values );
	}

	if ( values[0] != 0 )
		values[strlen( values ) - 1] = 0;
}

void build_update_values( struct dbvalues *table, char *values )
{
	for ( int i = 0; table[i].name != 0; i++ )
	{
		strcat( values, table[i].name );
		strcat( values, "=" );

		set_db_values( &table[i], values );
	}

	if ( values[0] != 0 )
		values[strlen( values ) - 1] = 0;
}

int db_begin_transaction(  )
{
	if ( sqlite3_exec( sqlite3_instance, "BEGIN", 0, 0, 0 ) != SQLITE_OK )
	{
		log_sqlite3( "unable to begin transaction." );
		return 0;
	}

	return 1;

}

int db_end_transaction(  )
{
	if ( sqlite3_exec( sqlite3_instance, "END", 0, 0, 0 ) != SQLITE_OK )
	{
		log_sqlite3( "unable to end transaction." );
		return 0;
	}

	return 1;
}
