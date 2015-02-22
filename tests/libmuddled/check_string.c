/******************************************************************************
 *         __  __           _     _         ____  _       _                   *
 *        |  \/  |_   _  __| | __| |_   _  |  _ \| | __ _(_)_ __  ___         *
 *        | |\/| | | | |/ _` |/ _` | | | | | |_) | |/ _` | | '_ \/ __|        *
 *        | |  | | |_| | (_| | (_| | |_| | |  __/| | (_| | | | | \__ \        *
 *        |_|  |_|\__,_|\__,_|\__,_|\__, | |_|   |_|\__,_|_|_| |_|___/        *
 *                                  |___/                                     *
 *                                                                            *
 *         (C) 2010 by Ryan Jennings <c0der78@gmail.com> www.arg3.com         *
 *	               Many thanks to creators of muds before me.                 *
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
#include <muddyengine/string.h>
#include <muddyengine/engine.h>

START_TEST(test_str_cmp)
{
    const char *arg = "test";

    fail_if(str_cmp(arg, "test"));

    fail_if(!str_cmp(arg, "blah"));
}
END_TEST


START_TEST(test_str_prefix)
{
    const char *arg = "this is a test";

    fail_if(str_prefix("this is", arg));

    fail_if(!str_prefix("a test", arg));
}
END_TEST

START_TEST(test_str_infix)
{
    const char *arg = "foobar123";

    fail_if(str_infix("bar", arg));

    fail_if(!str_infix("blah", arg));
}
END_TEST


START_TEST(test_str_suffix)
{
    const char *arg = "this is a test";

    fail_if(str_suffix("test", arg));

    fail_if(!str_suffix("this", arg));
}
END_TEST

START_TEST(test_is_name)
{
    const char *arg = "foo bar 123";

    fail_if(!is_name("bar", arg));

    fail_if(is_name("blah", arg));

    fail_if(!is_name("12", arg));
}
END_TEST

START_TEST(test_is_exact_name)
{
    const char *arg = "foo bar 123";

    fail_if(!is_exact_name("bar", arg));

    fail_if(is_exact_name("12", arg));
}
END_TEST

START_TEST(test_stristr)
{
    const char *arg = "foobar123";

    fail_if(stristr(arg, "OBaR1") == 0);

    fail_if(stristr(arg, "blah") != 0);
}
END_TEST

START_TEST(test_str_replace)
{
    const char *arg = "this is a test";

    const char *arg2 = str_replace(arg, "test", "foobar");

    fail_if(strcmp(arg2, "this is a foobar"));

    const char *arg3 = str_replace(arg, "blah", "foobar");

    fail_if(strcmp(arg3, "this is a test"));

    const char *arg4 = "this is a test, this is a test";

    const char *arg5 = str_replace(arg4, "test", "foobar");

    fail_if(strcmp(arg5, "this is a foobar, this is a test"));
}
END_TEST

START_TEST(test_str_replace_all)
{
    const char *arg = "this is a test, this is a test";
    const char *arg2 = str_replace_all(arg, "test", "foobar");

    fail_if(strcmp(arg2, "this is a foobar, this is a foobar"));

    const char *arg3 = str_replace_all(arg, "foo", "blah");

    fail_if(strcmp(arg3, "this is a test, this is a test"));
}
END_TEST


START_TEST(test_str_ireplace)
{
    const char *arg = "this is a test";

    const char *arg2 = str_ireplace(arg, "TEsT", "foobar");

    fail_if(strcmp(arg2, "this is a foobar"));

    const char *arg3 = str_ireplace(arg, "blAH", "foobar");

    fail_if(strcmp(arg3, "this is a test"));

    const char *arg4 = "this is a test, this is a test";

    const char *arg5 = str_ireplace(arg4, "TeST", "foobar");

    fail_if(strcmp(arg5, "this is a foobar, this is a test"));
}
END_TEST

START_TEST(test_str_ireplace_all)
{
    const char *arg = "this is a test, this is a test";
    const char *arg2 = str_ireplace_all(arg, "TeST", "foobar");

    fail_if(strcmp(arg2, "this is a foobar, this is a foobar"));

    const char *arg3 = str_ireplace_all(arg, "fOO", "blah");

    fail_if(strcmp(arg3, "this is a test, this is a test"));
}
END_TEST

START_TEST(test_capitalize)
{
    const char *arg = "this IS a TEST";

    const char *arg2 = capitalize(arg);

    fail_if(str_cmp(arg2, "This is a test"));
}
END_TEST

START_TEST(test_is_valid_email)
{
    fail_if(is_valid_email("this is not an email"));

    fail_if(!is_valid_email("me@foo.bar"));

    fail_if(is_valid_email("me@foo"));

    fail_if(is_valid_email("me.foo.bar"));
}
END_TEST

START_TEST(test_is_number)
{
    fail_if(!is_number("123"));

    fail_if(!is_number("-123"));

    fail_if(!is_number("12.3"));

    fail_if(!is_number("-12.3"));

    fail_if(is_number("foobar"));
}
END_TEST

START_TEST(test_get_line)
{
    char buf[ARG_SIZ];

    const char *arg = "this is a test\ntest this is";

    const char *arg2 = get_line(arg, buf);

    fail_if(strcmp(arg2, "test this is"));

    fail_if(strcmp(buf, "this is a test"));
}
END_TEST

START_TEST(test_fillstr)
{
    const char *arg = fillstr(0, 10);

    fail_if(strcmp(arg, "----------"));

    arg = fillstr("~W-", 5);

    fail_if(strcmp(arg, "~W-~W-~W-~W-~W-"));
}
END_TEST

START_TEST(test_count_lines)
{
    const char *arg = "this is a test\ntest this is\n\ra test this is\r";

    fail_if(count_lines(arg) != 2);

}
END_TEST

START_TEST(test_one_argument)
{
    const char *buf = "This Is a Test";

    char arg[ARG_SIZ];

    buf = one_argument(buf, arg);

    fail_if(strcmp(arg, "This"));

    fail_if(strcmp(buf, "Is a Test"));

    buf = "'This Is' a Test";

    buf = one_argument(buf, arg);

    fail_if(strcmp(arg, "This Is"));

    fail_if(strcmp(buf, "a Test"));
}
END_TEST

START_TEST(test_strip_color)
{
    const char *arg = "~WThis is a ~MTest~x";

    const char *res = strip_color(arg);

    fail_if(strcmp(res, "This is a Test"));
}
END_TEST

START_TEST(test_strlen_color)
{
    const char *arg = "~Wfoo~Mbar~x";

    fail_if(strlen_color(arg) != 6);
}
END_TEST

START_TEST(test_align_string)
{
    const char *arg = align_string(ALIGN_CENTER, 5, "~W", "-", "T");

    fail_if(strcmp(arg, "~W--T--~x"));
}
END_TEST

START_TEST(test_strpos)
{
    const char *arg = "~Wfoo~Mbar~G123~x";

    fail_if(strpos(arg, 5) != 9);
}
END_TEST

START_TEST(test_ordinal_string)
{
    fail_if(strcmp(ordinal_string(5), "5th"));

    fail_if(strcmp(ordinal_string(3), "third"));

    fail_if(strcmp(ordinal_string(22), "22nd"));

    fail_if(strcmp(ordinal_string(31), "31st"));

    fail_if(strcmp(ordinal_string(1), "first"));
}
END_TEST

START_TEST(test_strupper)
{
    fail_if(strcmp(strupper("test"), "TEST"));

    fail_if(strcmp(strupper("'this is' a test."), "'THIS IS' A TEST."));
}
END_TEST

START_TEST(test_number_argument)
{
    const char *arg = "18.foobar";

    char buf[ARG_SIZ];

    int num = number_argument(arg, buf);

    fail_if(num != 18);

    fail_if(strcmp(buf, "foobar"));
}
END_TEST

START_TEST(test_multi_argument)
{
    const char *arg = "18*foobar";

    char buf[ARG_SIZ];

    int num = multi_argument(arg, buf);

    fail_if(num != 18);
    fail_if(strcmp(buf, "foobar"));
}
END_TEST

Suite *string_suite (void)
{
    Suite *s = suite_create ("String");

    /* Core test case */
    TCase *tc_core = tcase_create ("Core");
    tcase_add_test (tc_core, test_str_cmp);
    tcase_add_test(tc_core, test_str_prefix);
    tcase_add_test(tc_core, test_str_infix);
    tcase_add_test(tc_core, test_str_suffix);
    tcase_add_test(tc_core, test_is_name);
    tcase_add_test(tc_core, test_is_exact_name);
    tcase_add_test(tc_core, test_stristr);
    tcase_add_test(tc_core, test_str_replace);
    tcase_add_test(tc_core, test_str_ireplace);
    tcase_add_test(tc_core, test_str_replace_all);
    tcase_add_test(tc_core, test_str_ireplace_all);
    tcase_add_test(tc_core, test_capitalize);
    tcase_add_test(tc_core, test_is_valid_email);
    tcase_add_test(tc_core, test_is_number);
    tcase_add_test(tc_core, test_get_line);
    tcase_add_test(tc_core, test_one_argument);
    tcase_add_test(tc_core, test_strip_color);
    tcase_add_test(tc_core, test_strlen_color);
    tcase_add_test(tc_core, test_count_lines);
    tcase_add_test(tc_core, test_fillstr);
    tcase_add_test(tc_core, test_align_string);
    tcase_add_test(tc_core, test_strpos);
    tcase_add_test(tc_core, test_ordinal_string);
    tcase_add_test(tc_core, test_strupper);
    tcase_add_test(tc_core, test_number_argument);
    tcase_add_test(tc_core, test_multi_argument);

    suite_add_tcase (s, tc_core);

    return s;
}

