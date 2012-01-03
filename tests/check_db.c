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

void test_db_setup() {
	init_sqlite3();
}

void test_db_teardown() {
	close_sqlite3();
}

START_TEST(test_escape_db_str)
{
	const char *str = "test'ing";
	
	const char *estr = escape_db_str(str);
	
	fail_unless(!str_cmp(estr, "test''ing"), "quotation was not escaped");
}
END_TEST

START_TEST(test_get_rowid_in_transaction)
{
	db_begin_transaction();

	if ( sqlite3_exec( sqlite3_instance, "create table if not exists test(id integer primary key, value integer default null)", NULL, 0, 0 ) != SQLITE_OK )
	{
		fail( "could note create test table" );
	}
	
	if ( sqlite3_exec( sqlite3_instance, "insert into test(value) values(5)", NULL, 0, 0) != SQLITE_OK)
	{
		fail( "could not insert to test table" );
	}
	
	int id = sqlite3_last_insert_rowid( sqlite3_instance );

	fail_if(id == 0);

	sqlite3_exec( sqlite3_instance, "drop table if exists test", NULL, 0, 0);

	db_end_transaction();
}
END_TEST

Suite *database_suite ()
{
  Suite *s = suite_create ("Database");

  /* Core test case */
  TCase *tc_core = tcase_create ("Core");

  tcase_add_checked_fixture(tc_core, test_db_setup, test_db_teardown);

  tcase_add_test (tc_core, test_escape_db_str);

  tcase_add_test (tc_core, test_get_rowid_in_transaction);

  suite_add_tcase (s, tc_core);


  return s;
}

