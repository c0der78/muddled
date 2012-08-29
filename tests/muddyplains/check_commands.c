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
#include "muddyplains/command.h"
#include <stdlib.h>
#include <stdarg.h>
#include <muddyengine/character.h>
#include <muddyengine/player.h>
#include <muddyengine/string.h>
#include <muddyengine/connection.h>
#include <muddyengine/account.h>
#include <muddyengine/engine.h>

static Character *tch = 0;

void dummy_writeln(Connection *conn, const char *fmt)
{
}

void commands_core_setup()
{
	tch = new_char();
	Connection *c = (Connection *)alloc_mem(1, sizeof(Connection));
	Account *a = new_account(c);
	c->writeln = dummy_writeln;
	tch->pc = new_player(c);
	tch->pc->account = a;
	LINK(first_character, tch, next);
	LINK(first_player, tch, next_player);
}

void commands_core_teardown()
{
	free_mem(tch->pc->conn);
	tch->pc->conn = 0;
	
	destroy_account(tch->pc->account);
	
	extract_char(tch, true);
}

START_TEST(test_dofun_color)
{	
	
	set_bit(tch->pc->account->flags, PLR_COLOR_OFF);
	
	do_color("color", tch, str_empty);
	
	fail_if(is_set(tch->pc->account->flags, PLR_COLOR_OFF), "plr color was not set");
	
	do_color("color", tch, str_empty);
	
	fail_if(!is_set(tch->pc->account->flags, PLR_COLOR_OFF), "plr color was not removed");
}
END_TEST

Suite *commands_suite (void)
{
  Suite *s = suite_create ("Commands");

  /* Core test case */
  TCase *tc_core = tcase_create ("Core");
  tcase_add_checked_fixture(tc_core, commands_core_setup, commands_core_teardown);
  tcase_add_test (tc_core, test_dofun_color);
  suite_add_tcase (s, tc_core);
  
  return s;
}

