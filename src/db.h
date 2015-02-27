
/******************************************************************************
 *                                       _     _ _          _                 *
 *                   _ __ ___  _   _  __| | __| | | ___  __| |                *
 *                  | '_ ` _ \| | | |/ _` |/ _` | |/ _ \/ _` |                *
 *                  | | | | | | |_| | (_| | (_| | |  __/ (_| |                *
 *                  |_| |_| |_|\__,_|\__,_|\__,_|_|\___|\__,_|                *
 *                                                                            *
 *         (C) 2010 by Ryan Jennings <c0der78@gmail.com> www.arg3.com         *
 *                 Many thanks to creators of muds before me.                 *
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
#include "flag.h"

#define sql         sqlite3
#define sql_stmt    sqlite3_stmt
#define sql_int64   sqlite3_int64
#define sql_uint64  sqlite3_uint64

#define SQL_OK      SQLITE_OK
#define SQL_DONE    SQLITE_DONE
#define SQL_NONTYPE SQLITE_NOTFOUND

#define SQL_INT     SQLITE_INTEGER
#define SQL_DOUBLE  SQLITE_FLOAT
#define SQL_BLOB    SQLITE_BLOB
#define SQL_NULL    SQLITE_NULL
#define SQL_TEXT    SQLITE_TEXT

#define SQL_FLAG    1000
#define SQL_CUSTOM  1001
#define SQL_FLOAT   1002
#define SQL_ARRAY   SQL_CUSTOM
#define SQL_LOOKUP  1003

typedef struct field_map
{
	const char *name;
	const void *value;
	int type;
	const void *arg1;
	const void *arg2;
	int flags;
} field_map;

typedef int (*custom_field_t) (sql_stmt *, int, const field_map *);
typedef void (*sql_callback_t) (sql_stmt *);

const char *tablenameid(const char *tablename);
const char *escape_sql_str(const char *);
int db_save_int_array(sql_stmt *, int, const field_map *);
void db_read_int_array(int, void *, sql_stmt *, int);
int db_begin_transaction();
int db_end_transaction();
int sql_exec(const char *);
sql *enginedb();
int sql_errcode();
const char *sql_errmsg();
int db_last_insert_rowid();
void db_close();
int db_open(const char *, const char *);
int sql_query(const char *, int, sql_stmt **);
int sql_insert_query(field_map *, const char *);
int sql_update_query(field_map *, const char *, identifier_t);
int sql_select_query(field_map *, const char *, sql_callback_t, const char *);

const char *sql_column_str(sql_stmt *, int);
int sql_column_index(sql_stmt *, const char *);
int sql_col_int(sql_stmt *, const char *);

int db_save_lookup(sql_stmt *, int, field_map *);

#define sql_step            sqlite3_step
#define sql_finalize        sqlite3_finalize
#define sql_reset           sqlite3_reset

#define sql_bind_blob       sqlite3_bind_blob
#define sql_bind_double     sqlite3_bind_double
#define sql_bind_float      sqlite3_bind_double
#define sql_bind_int        sqlite3_bind_int
#define sql_bind_int64      sqlite3_bind_int64
#define sql_bind_null       sqlite3_bind_null
#define sql_bind_text       sqlite3_bind_text
#define sql_bind_text16     sqlite3_bind_text16
#define sql_bind_value      sqlite3_bind_value
#define sql_bind_zeroblob   sqlite3_bind_zeroblob

#define sql_column_blob     sqlite3_column_blob
#define sql_column_bytes    sqlite3_column_bytes
#define sql_column_bytes16  sqlite3_column_bytes16
#define sql_column_double   sqlite3_column_double
#define sql_column_float    sqlite3_column_double
#define sql_column_int      sqlite3_column_int
#define sql_column_int64    sqlite3_column_int64
#define sql_column_text     sqlite3_column_text
#define sql_column_text16   sqlite3_column_text16
#define sql_column_type     sqlite3_column_type
#define sql_column_value    sqlite3_column_value
#define sql_column_name     sqlite3_column_name
#define sql_column_count    sqlite3_column_count

/*!
 * saves a record @param table the dbvalues structure to save @param
 * tableName the name of the table @param id the current id of the value
 * @return the identifier of the value
 */
identifier_t db_save(field_map *, const char *, identifier_t);
int db_load_by_id(field_map *, const char *, identifier_t);
int db_load_all(const char *, sql_callback_t, const char *, ...) __attribute__ ((format(printf, 3, 4)));
int sql_load_columns(sql_stmt *stmt, field_map *table);
int db_delete(const char *tablename, identifier_t);

/*
 * field map functions
 */
int fm_int(const field_map *);
float fm_float(const field_map *);
double fm_double(const field_map *);
const char *fm_str(const field_map *);
Flag *fm_flag(const field_map *);

#endif              /* // #ifndef DB_H */
