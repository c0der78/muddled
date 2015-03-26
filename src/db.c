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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include "engine.h"
#include "db.h"
#include "flag.h"
#include "str.h"
#include "log.h"

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

    for (int i = 0; tablename[i] != '\0'; ++i)
    {
        if (tablename[i] == '_')
        {
            if (tablename[++i] == '\0')
                break;

            result[c++] = UPPER(tablename[i]);
        }
        else
        {
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
    if (sqlite3_close(enginedb()) != SQLITE_OK)
    {
        log_data("unable to close db");
    }
}

int db_open(const char *name, const char *root_path)
{
    if (root_path && *root_path)
        return sqlite3_open(formatf("%s/share/muddled/%s", root_path, name),
                            &engine_info.db);
    else
        return sqlite3_open(formatf("resources/%s", name), &engine_info.db);
}

int sql_query(const char *buf, int len, sql_stmt **stmt)
{
    return sqlite3_prepare(enginedb(), buf, len, stmt, 0);
}

int sql_col_int(sql_stmt *stmt, const char *column)
{
    return sqlite3_column_int(stmt, sql_column_index(stmt, column));
}

const char *sql_column_str(sql_stmt *stmt, int column)
{
    return (const char *)sqlite3_column_text(stmt, column);
}

int sql_column_index(sql_stmt *stmt, const char *name)
{
    for (int i = 0, size = sql_column_count(stmt); i < size; ++i)
    {
        if (!strcmp(sql_column_name(stmt, i), name))
        {
            return i;
        }
    }
    return -1;
}

void fm_set_int(const field_map *field, int value)
{
    *((int *)field->value) = value;
}

int fm_int(const field_map *field)
{
    return *((int *)field->value);
}

void fm_set_float(const field_map *field, float value)
{
    *((float *)field->value) = value;
}

float fm_float(const field_map *field)
{
    return *((float *)field->value);
}

Flag *fm_flag(const field_map *field)
{
    return *((Flag **) field->value);
}

void fm_parse_flags(const field_map *field, const char *value)
{
    parse_flags(fm_flag(field), value, (const Lookup *)field->arg1);
}

void fm_set_double(const field_map *field, double value)
{
    *((double *)field->value) = value;
}

double fm_double(const field_map *field)
{
    return *((double *)field->value);
}

void fm_set_str(const field_map *field, const char *value)
{
    *((const char **)field->value) = str_dup(value);
}

const char *fm_str(const field_map *field)
{
    return *((const char **)field->value);
}

void fm_custom(field_map *field, sql_stmt *stmt, int i)
{
    custom_field_t func =  (custom_field_t) field->funk;
    assert(func != 0);
    (*func) (stmt, i, field);
}

void fm_lookup(field_map *field, sql_stmt *stmt, int i)
{
    const Lookup *lookup = (const Lookup *)field->arg1;
    assert(lookup != 0);
    (*(int *)field->value) = value_lookup(lookup, sql_column_str(stmt, i));
}

const char *escape_sql_str(const char *pstr)
{
    static char buf[OUT_SIZ * 3];

    char *pbuf = buf;

    while (pstr && *pstr)
    {
        if (*pstr == '\'')
            *pbuf++ = '\'';

        *pbuf++ = *pstr++;
    }

    *pbuf = 0;

    return buf;
}

identifier_t db_save(field_map *table, const char *tableName, identifier_t id)
{
    if (id == 0)
    {
        if (sql_insert_query(table, tableName) != SQL_OK)
            log_data("could not insert to %s", tableName);
        return db_last_insert_rowid();
    }
    else
    {
        if (sql_update_query(table, tableName, id) != SQL_OK)
            log_data("could not update %s", tableName);

        return id;
    }
}

int db_load_all(const char *tableName, sql_callback_t callback, const char *constraints, ...)
{
    char buf[OUT_SIZ] = {0};

    if (constraints)
    {
        va_list args;

        va_start(args, constraints);

        vsnprintf(buf, OUT_SIZ, constraints, args);

        va_end(args);
    }

    if (sql_select_query(0, tableName, callback, buf) != SQL_OK)
    {
        log_data("could not load from %s", tableName);
        return 0;
    }

    return 1;
}

int db_load_by_id(field_map *table, const char *tablename, identifier_t id)
{
    char buf[1024] = {0};

    sprintf(buf, "where %s='%"PRId64"'", tablenameid(tablename), id);

    if (sql_select_query(table, tablename, 0, buf) != SQL_OK)
    {
        fprintf(stderr, "could not load from %s", tablename);
        return 0;
    }

    return 1;
}

int sql_bind_str(sql_stmt *stmt, int index, const char *str)
{
    return sql_bind_text(stmt, index, str, strlen(str), 0);
}

int sql_bind_table_value(sql_stmt *stmt, int index, field_map *field)
{

    //log_trace("binding %s...", field->name);

    if (field->value == 0)
    {
        return sql_bind_null(stmt, index);
    }
    switch (field->type)
    {
    case SQL_INT:
        return sql_bind_int(stmt, index, fm_int(field));
    case SQL_TEXT:
        return sql_bind_str(stmt, index, fm_str(field));
    case SQL_DOUBLE:
        return sql_bind_double(stmt, index, fm_double(field));
    case SQL_FLOAT:
        return sql_bind_float(stmt, index, fm_float(field));
    case SQL_FLAG:
    {
        Flag *value = fm_flag(field);
        if (value == 0 || is_empty(value))
            return sql_bind_null(stmt, index);

        return sql_bind_str(stmt, index, format_flags(value, (const Lookup *)field->arg1));
    }
    case SQL_LOOKUP:
    {
        int value = fm_int(field);
        const Lookup *lookup = (const Lookup *)field->arg1;
        assert(lookup != 0);
        const char *name = lookup_name(lookup, value);
        if (name == 0)
            return sql_bind_null(stmt, index);
        return sql_bind_str(stmt, index, name);
    }
    case SQL_CUSTOM:
    {
        custom_field_t func = (custom_field_t) (field->funk);
        assert(func != 0);
        return (*func) (stmt, index, field);
    }
    default:
        log_bug("unknown save type for field %s", field->name);
        return SQL_NONTYPE;
    }
}

static int sql_bind_values(sql_stmt *stmt, field_map *table, const char *idname)
{

    for (int i = 0, p = 1; table[i].name != 0; i++)
    {
        if (!strcmp(table[i].name, idname))
            continue;

        int err = sql_bind_table_value(stmt, p++, &table[i]);

        if (err != SQL_OK)
            return err;
    }

    return SQL_OK;
}

int db_save_int_array(sql_stmt *stmt, int index, const field_map *table)
{
    int *values = (int *)table->value;
    size_t max = *((size_t*) table->arg2);

    static char buf[BUF_SIZ];
    int len = 0;

    for (int i = 0; i < max; i++)
    {
        len += sprintf(&buf[len], "%d,", (values)[i]);
    }

    if (len > 0)
        buf[len - 1] = 0;

    return sql_bind_text(stmt, index, buf, len, 0);
}

void db_read_int_array(int max, void *arg, sql_stmt *stmt, int i)
{
    int *values = (int *)arg;
    const char *value = strtok((char *)sql_column_str(stmt, i), ",");
    int index = 0;

    while (value)
    {
        if (index >= max)
            break;

        (values)[index++] = atoi(value);

        value = strtok(NULL, ",");
    }
}

int db_delete(const char *tablename, identifier_t id)
{
    char buf[BUF_SIZ] = {0};

    int len = sprintf(buf, "delete from %s where %s=?", tablename, tablenameid(tablename));

    //log_trace("%s", buf);

    sql_stmt *stmt;

    if (sql_query(buf, len, &stmt) != SQL_OK)
        return sql_finalize(stmt);

    if (sql_bind_int(stmt, 1, id) != SQL_OK)
        return sql_finalize(stmt);

    sql_step(stmt);

    return sql_finalize(stmt);
}

int sql_insert_query(field_map *table, const char *tablename)
{
    char buf[OUT_SIZ] = { 0 };
    char columns[OUT_SIZ] = { 0 };
    char params[OUT_SIZ] = { 0 };

    const char *idname = tablenameid(tablename);

    for (int i = 0; table[i].name != 0; i++)
    {
        if (!strcmp(table[i].name, idname))
            continue;

        strcat(columns, table[i].name);
        strcat(params, "?");

        if (table[i + 1].name != 0)
        {
            strcat(columns, ",");
            strcat(params, ",");
        }
    }

    sql_stmt *stmt;

    int len = sprintf(buf, "insert into %s (%s) values(%s)", tablename, columns,
                      params);

    //log_trace("%s", buf);

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {
        return sql_finalize(stmt);
    }

    if (sql_bind_values(stmt, table, idname) != SQL_OK)
    {
        return sql_finalize(stmt);
    }

    sql_step(stmt);

    return sql_finalize(stmt);
}

int sql_update_query(field_map *table, const char *tablename, identifier_t id)
{
    char buf[OUT_SIZ] = { 0 };
    char params[OUT_SIZ] = { 0 };

    const char *idname = tablenameid(tablename);

    for (int i = 0; table[i].name != 0; i++)
    {

        if (!strcmp(table[i].name, idname))
            continue;

        strcat(params, table[i].name);
        strcat(params, "=?");

        if (table[i + 1].name != 0)
        {
            strcat(params, ",");
        }
    }

    sql_stmt *stmt;

    int len = sprintf(buf, "update %s set %s where %s=%" PRId64, tablename,
                      params, idname, id);

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {
        return sql_finalize(stmt);
    }

    if (sql_bind_values(stmt, table, idname) != SQL_OK)
        return sql_finalize(stmt);

    sql_step(stmt);

    return sql_finalize(stmt);
}

/**
 * returns the numver of columns read
 */
int sql_load_columns(sql_stmt *stmt, field_map *table)
{
    int i;

    for (i = 0; table && table[i].name != 0; i++)
    {

        field_map *field = &table[i];

        //log_trace("loading %s...", field->name);

        switch (field->type)
        {
        case SQL_INT:
            fm_set_int(field, sql_column_int(stmt, i));
            break;
        case SQL_TEXT:
            fm_set_str(field, sql_column_str(stmt, i));
            break;
        case SQL_FLOAT:
            fm_set_float(field, sql_column_float(stmt, i));
            break;
        case SQL_DOUBLE:
            fm_set_double(field, sql_column_double(stmt, i));
            break;
        case SQL_FLAG:
            fm_parse_flags(field, sql_column_str(stmt, i));
            break;
        case SQL_CUSTOM:
            fm_custom(field, stmt, i);
            break;
        case SQL_LOOKUP:
            fm_lookup(field, stmt, i);
            break;
        }
    }
    return i;
}

int sql_select_query(field_map *table, const char *tablename, sql_callback_t callback, const char *constraints)
{
    char buf[OUT_SIZ] = { 0 };
    char columns[OUT_SIZ] = { 0 };

    for (int i = 0; table && table[i].name != 0; i++)
    {
        strcat(columns, table[i].name);

        if (table[i + 1].name != 0)
        {
            strcat(columns, ",");
        }
    }

    if (columns[0] == 0)
    {
        sprintf(columns, "%s.*", tablename);
    }

    sql_stmt *stmt;

    int len = sprintf(buf, "select %s from %s %s",
                      columns,
                      tablename,
                      constraints ? constraints : "");

    //log_trace("%s", buf);

    if (sql_query(buf, len, &stmt) != SQL_OK)
    {
        return sql_finalize(stmt);
    }
    for (int res = sql_step(stmt); res != SQL_DONE; res = sql_step(stmt))
    {
        if (table)
        {
            sql_load_columns(stmt, table);
        }

        if (callback)
        {
            callback(stmt);
        }

    }
    return sql_finalize(stmt);
}

int db_begin_transaction()
{
    if (sql_exec("BEGIN") != SQL_OK)
    {
        log_data("unable to begin transaction.");
        return 0;
    }
    return 1;

}

int db_end_transaction()
{
    if (sql_exec("END") != SQL_OK)
    {
        log_data("unable to end transaction.");
        return 0;
    }
    return 1;
}
