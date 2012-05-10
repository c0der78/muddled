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
#include <ctype.h>
#include <muddyengine/db.h>
#include <muddyengine/log.h>
#include <muddyengine/engine.h>
#include <muddyengine/string.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include "config.h"

sqlite3 *enginedb()
{
    return engine_info.db;
}

int sql_errcode()
{
    sqlite3 *db = enginedb();
    return db == 0 ? SQL_OK : sqlite3_errcode(enginedb());
}

const char *sql_errmsg()
{
    return sqlite3_errmsg(enginedb());
}

const char *tablenameid(const char *tablename)
{
    static char buf[4][100];
    static int s = 0;

    ++s, s %= 4;

    char *result = buf[s];

    int c = 0;

    for (int i = 0; tablename[i] != '\0'; ++i) {
	if (tablename[i] == '_') {
	    if (tablename[++i] == '\0')
		break;

	    result[c++] = UPPER(tablename[i]);
	} else {
	    result[c++] = tablename[i];
	}
    }

    result[c] = 0;

    strcat(result, "Id");
    return buf[s];
}

int db_last_insert_rowid()
{
    return sqlite3_last_insert_rowid(enginedb());
}

int sql_exec(const char *buf)
{
    return sqlite3_exec(enginedb(), buf, 0, 0, 0);
}

void db_close()
{
    if (sqlite3_close(enginedb()) != SQLITE_OK) {
	log_data("unable to close db");
    }
}

int db_open(const char *root_path)
{
    return sqlite3_open(formatf("%s/" SQLITE3_FILE, root_path),
			&engine_info.db);
}

int sql_query(const char *buf, int len, sql_stmt ** stmt)
{
    return sqlite3_prepare(enginedb(), buf, len, stmt, 0);
}

int sql_col_int(sql_stmt * stmt, const char *column)
{
    return sqlite3_column_int(stmt, sql_column_index(stmt, column));
}

const char *sql_column_str(sql_stmt * stmt, int column)
{
    return (const char *) sqlite3_column_text(stmt, column);
}

int sql_column_index(sql_stmt * stmt, const char *name)
{
    for (int i = 0, size = sql_column_count(stmt); i < size; ++i) {
	if (!strcmp(sql_column_name(stmt, i), name)) {
	    return i;
	}
    }
    return -1;
}

int field_map_int(field_map * table)
{
    return *((int *) table->value);
}

float field_map_float(field_map * table)
{
    return *((float *) table->value);
}

Flag *field_map_flag(field_map * table)
{
    return *((Flag **) table->value);
}

double field_map_double(field_map * table)
{
    return *((double *) table->value);
}

const char *field_map_str(field_map * table)
{
    return *((const char **) table->value);
}

const char *escape_sql_str(const char *pstr)
{
    static char buf[OUT_SIZ * 3];

    char *pbuf = buf;

    while (pstr && *pstr) {
	if (*pstr == '\'')
	    *pbuf++ = '\'';

	*pbuf++ = *pstr++;
    }

    *pbuf = 0;

    return buf;
}

sql_int64 db_save(field_map * table, const char *tableName, sql_int64 id)
{
    if (id == 0) {
	if (sql_insert_query(table, tableName) != SQL_DONE)
	    log_data("could not insert to %s", tableName);

	return db_last_insert_rowid();
    } else {
	if (sql_update_query(table, tableName, id) != SQL_DONE)
	    log_data("could not update %s", tableName);

	return id;
    }
}

int sql_bind_table_value(sql_stmt * stmt, int index, field_map * table)
{

    log_trace("binding %s...", table->name);

    if (table->value == 0) {
	return sql_bind_null(stmt, index);
    }
    switch (table->type) {
    case SQL_INT:
	return sql_bind_int(stmt, index, field_map_int(table));
    case SQL_TEXT:
	{
	    const char *str = field_map_str(table);
	    return sql_bind_text(stmt, index, str, strlen(str), 0);
	}
    case SQL_DOUBLE:
	return sql_bind_double(stmt, index, field_map_double(table));
    case SQL_FLOAT:
	return sql_bind_float(stmt, index, field_map_float(table));
    case SQL_FLAG:
	{
	    Flag *value = field_map_flag(table);
	    if (value == 0 || is_empty(value))
		return sql_bind_null(stmt, index);

	    const char *str =
		format_flags(value, (const Lookup *) table->arg1);
	    return sql_bind_text(stmt, index, str, strlen(str), 0);
	}
    case SQL_LOOKUP:
	{
	    int value = field_map_int(table);
	    const Lookup *lookup = (const Lookup *) table->arg1;
	    assert(lookup != 0);
	    const char *name = lookup_name(lookup, value);
	    if (name == 0)
		return sql_bind_null(stmt, index);
	    return sql_bind_text(stmt, index, name, strlen(name), 0);
	}
    case SQL_CUSTOM:
	{
	    custom_sql_bind func = (custom_sql_bind) (table->arg1);
	    assert(func != 0);
	    return (*func) (stmt, index, table);
	}
    default:
	log_bug("unknown save type for field %s", table->name);
	return SQL_NONTYPE;
    }
}

int sql_bind_values(sql_stmt * stmt, field_map * table)
{

    for (int i = 0; table[i].name != 0; i++) {
	int err = sql_bind_table_value(stmt, i + 1, &table[i]);

	if (err != SQL_OK)
	    return err;
    }

    return SQL_OK;
}

int db_save_int_array(sql_stmt * stmt, int index, field_map * table)
{
    int *values = (int *) table->value;
    size_t max = (size_t) table->arg2;

    static char buf[BUF_SIZ];
    int len = 0;

    for (int i = 0; i < max; i++) {
	len += sprintf(&buf[len], "%d,", (values)[i]);
    }

    if (len > 0)
	buf[len - 1] = 0;

    return sql_bind_text(stmt, index, buf, len, 0);
}

void db_read_int_array(int max, void *arg, sql_stmt * stmt, int i)
{
    int *values = (int *) arg;
    const char *value = strtok((char *) sql_column_str(stmt, i), ",");
    int index = 0;

    while (value) {
	if (index >= max)
	    break;

	(values)[index++] = atoi(value);

	value = strtok(NULL, ",");
    }
}

void load_field_values()
{
}

int sql_insert_query(field_map * table, const char *tablename)
{
    char buf[OUT_SIZ] = { 0 };
    char columns[OUT_SIZ] = { 0 };
    char params[OUT_SIZ] = { 0 };

    for (int i = 0; table[i].name != 0; i++) {
	strcat(columns, table[i].name);
	strcat(params, "?");

	if (table[i + 1].name != 0) {
	    strcat(columns, ",");
	    strcat(params, ",");
	}
    }

    sql_stmt *stmt;

    int len =
	sprintf(buf, "insert into %s (%s) values(%s)", tablename, columns,
		params);

    log_trace("%s", buf);

    int err = sql_query(buf, len, &stmt);

    if (err != SQL_OK) {
	return err;
    }
    err = sql_bind_values(stmt, table);

    if (err != SQL_OK) {
	return err;
    }
    err = sql_step(stmt);

    if (err != SQL_DONE) {
	return err;
    }
    return sql_finalize(stmt);
}

int
sql_update_query(field_map * table, const char *tablename, sql_int64 id)
{
    char buf[OUT_SIZ] = { 0 };
    char params[OUT_SIZ] = { 0 };

    for (int i = 0; table[i].name != 0; i++) {
	strcat(params, table[i].name);
	strcat(params, "=?");

	if (table[i + 1].name != 0) {
	    strcat(params, ",");
	}
    }

    sql_stmt *stmt;

    int len = sprintf(buf, "update %s set %s where %s=%" PRId64, tablename,
		      params, tablenameid(tablename), id);

    int err = sql_query(buf, len, &stmt);

    if (err != SQL_OK) {
	return err;
    }
    err = sql_bind_values(stmt, table);

    if (err != SQL_OK)
	return err;

    err = sql_step(stmt);

    if (err != SQL_DONE)
	return err;

    if (sql_finalize(stmt) != SQL_OK) {
	log_data("could not finalize update statement");
    }
    return err;
}

/*
void build_insert_values( field_map *table, char *names, char *values )
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

void build_update_values( field_map *table, char *values )
{
	for ( int i = 0; table[i].name != 0; i++ )
	{
		strcat( values, table[i].name );
		strcat( values, "=" );

		set_db_values( &table[i], values );
	}

	if ( values[0] != 0 )
		values[strlen( values ) - 1] = 0;
}*/

int db_begin_transaction()
{
    if (sql_exec("BEGIN") != SQL_OK) {
	log_data("unable to begin transaction.");
	return 0;
    }
    return 1;

}

int db_end_transaction()
{
    if (sql_exec("END") != SQL_OK) {
	log_data("unable to end transaction.");
	return 0;
    }
    return 1;
}
