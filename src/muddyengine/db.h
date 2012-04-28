
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
#ifndef DB_H
#define DB_H

#include <sqlite3.h>
#include <stdint.h>

#define DB_OK SQLITE_OK

#define db_stmt sqlite3_stmt

#define db_type sqlite3

struct dbvalues
{
	const char *name;
	const void *value;
	int type;
	const void *arg1;
	const void *arg2;
};

typedef struct field_map
{
	const char *name;
	const void *value;
	int type;
	const void *arg1;
	const void *arg2;
	int flags;
} field_map;

typedef struct table_map
{
	const char *name;
	const char *key;
	const void *base;
	field_map *fields;
} table_map;

typedef const char *( *map_save_t ) ( const void * );
typedef const char *( *map_save_array_t ) ( int, const void * );
typedef void ( *map_read_t ) ( void *, db_stmt *, int );
typedef void ( *map_read_array_t ) ( int, void *, db_stmt *, int );

#define DBTYPE_FLAG	1000
#define DBTYPE_CUSTOM	1001
#define DBTYPE_FLOAT	1002
#define DBTYPE_ARRAY 1003

void build_update_values( struct dbvalues *, char * );
void build_insert_values( struct dbvalues *, char *, char * );
const char *escape_db_str( const char * );
const char *db_save_int_array( int, void * );
void db_read_int_array( int, void *, db_stmt *, int );
int db_begin_transaction(  );
int db_end_transaction(  );
int db_exec(const char *);
sqlite3 *enginedb();
int db_errcode();
const char *db_errmsg();
int db_last_insert_rowid();
int db_exec(const char *);
void db_close();
int db_open(const char *);
int db_query(const char *, int, db_stmt **);
int db_step(db_stmt *);
int db_finalize(db_stmt *);
int db_column_count(db_stmt *);
const char *db_column_name(db_stmt *, int);
int db_column_int(db_stmt *, int);
int64_t db_column_int64(db_stmt *, int);
const char *db_column_str(db_stmt *, int);

#define field(type, base, value, data) \
	*( (type *) ( (size_t) (value) - (size_t) (base) + (size_t) (data) ) )

#define field_int(base, value, data) \
	*( (int *) ( (size_t) (value) - (size_t) (base) + (size_t) (data) ) )

#define field_int_array(base, value, data) \
	*( (int **) ( (size_t) (value) - (size_t) (base) + (size_t) (data) ) )

#define field_str(base, value, data) \
	*( (const char **) ( (size_t) (value) - (size_t) (base) + (size_t) (data) ) )

#define field_double(base, value, data) \
	*( (double *) ( (size_t) (value) - (size_t) (base) + (size_t) (data) ) )

#define field_flag(base, value, data) \
	*( (Flag **) ( (size_t) (value) - (size_t) (base) + (size_t) (data) ) )

#endif							//  #ifndef DB_H
