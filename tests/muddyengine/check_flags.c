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
#include <stdlib.h>
#include <muddyengine/flag.h>
#include <muddyengine/lookup.h>

const Lookup test_flags[] = {
	{ "bit1", 1 },
	{ "bit2", 25 },
	{ "bit3", 50 },
	{ "bit4", 75 },
	{ "bit5", 100 },
	{ 0, 0 }
};


START_TEST(test_set_bit)
{
	Flag *flags = new_flag();
	
	set_bit(flags, 10);

	fail_if(flags->size != (10/sizeof(bit_t))+1, "flags size was not correct");
	
	fail_unless(flags->bits[10/sizeof(bit_t)] & (1 << (10 % sizeof(bit_t))), "bit was not set");
	
	destroy_flags(flags);
	
}
END_TEST

START_TEST(test_remove_bit)
{
	Flag *flags = new_flag();
	
	set_bit(flags, 100);
	
	fail_unless(is_set(flags, 100), "unable to set the bit");
	
	remove_bit(flags, 100);
	
	fail_unless(!is_set(flags, 100), "unable to remove the bit");
	
	destroy_flags(flags);
}
END_TEST
	
START_TEST(test_format_flags)
{
	Flag *flags = new_flag();
	
	set_bit(flags, 25);
	set_bit(flags, 100);
	set_bit(flags, 75);
	
	const char *expected = "bit2,bit4,bit5";
	const char *result = format_flags(flags, test_flags);
	
	
	fail_if(strcmp(result, expected), "flag string was not what was expected");
	
	destroy_flags(flags);
}
END_TEST


START_TEST(test_parse_flags)
{
	char format[100];
	
	strcpy(format, "bit1,bit3,bit5");
	
	Flag *flags = new_flag();
	
	parse_flags(flags, format, test_flags);
	
	fail_if(!is_set(flags, 1), "bit 1 was not set");
	fail_if(!is_set(flags, 50), "bit 3 was not set");
	fail_if(!is_set(flags, 100), "bit 5 was not set");
	
	destroy_flags(flags);
	
}
END_TEST

Suite *flags_suite (void)
{
  Suite *s = suite_create ("Flags");

  /* Core test case */
  TCase *tc_core = tcase_create ("Core");
  tcase_add_test (tc_core, test_set_bit);
  tcase_add_test (tc_core, test_remove_bit);
  tcase_add_test(tc_core, test_format_flags);
  tcase_add_test(tc_core, test_parse_flags);
  suite_add_tcase (s, tc_core);
  
  return s;
}

