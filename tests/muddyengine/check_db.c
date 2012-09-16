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
#include <check.h>
#include <muddyengine/db.h>
#include <muddyengine/string.h>
#include <muddyengine/macro.h>
#include <muddyengine/engine.h>
#include <muddyengine/util.h>
#include <muddyengine/log.h>
#include <inttypes.h>

#define DBNAME "test"

void test_db_setup() {
	db_open(Stringify(ROOT_DIR), "muddytest");

	char buf[BUF_SIZ];

	sprintf(buf, "create table if not exists "DBNAME"(testId integer not null primary key autoincrement,"
		"name varchar, intval integer)");

	if(sql_exec(buf) != SQL_OK) {
		fail("Could not create "DBNAME" table");
	}

	if(sql_exec("delete from " DBNAME) != SQL_OK)
		fail("could not delete from table");
}

void test_db_teardown() {
	db_close();

	system("rm -rf muddytest.db3");
}

START_TEST(test_escape_sql_str)
{
	const char *str = "test'ing";
	
	const char *estr = escape_sql_str(str);
	
	fail_unless(!str_cmp(estr, "test''ing"), "quotation was not escaped");
}
END_TEST

START_TEST(test_get_rowid_in_transaction)
{
	db_begin_transaction();

	if ( sql_exec( "insert into "DBNAME"(name,intval) values('testA', '5')") != SQLITE_OK)
	{
		fail( "could not insert to test table" );
	}
	
	int id = db_last_insert_rowid( );

	fail_if(id == 0);

	db_end_transaction();
}
END_TEST

START_TEST(test_field_map)
	
	struct test
	{
		sql_int64 id;
		const char *name;
		int value;
	};

	struct test T;

	T.id = 0;
	T.name = str_dup("testA");
	T.value = number_percent();

	field_map table[] = {
		{"name", &T.name, SQL_TEXT},
		{"intval", &T.value, SQL_INT},
		{0}
	};

	fail_if(T.value != fm_int(&table[1]));

	T.id = db_save(table, DBNAME, T.id);

	int check = number_percent();

	char buf[BUF_SIZ];

	sprintf(buf, "update "DBNAME" set intval=%d where %s=%"PRId64" and name='%s' and intval='%d'", check, tablenameid(DBNAME), T.id, T.name, T.value);

	if(sql_exec(buf) != SQL_OK) {
		fail("could not update saved data entry");
	}

	db_load_by_id(table, DBNAME, T.id);

	fail_if(T.value != check);

END_TEST

Suite *database_suite ()
{
  Suite *s = suite_create ("Database");

  /* Core test case */
  TCase *tc_core = tcase_create ("Core");

  tcase_add_checked_fixture(tc_core, test_db_setup, test_db_teardown);

  tcase_add_test (tc_core, test_escape_sql_str);

  tcase_add_test (tc_core, test_get_rowid_in_transaction);

  tcase_add_test (tc_core, test_field_map);

  suite_add_tcase (s, tc_core);


  return s;
}

