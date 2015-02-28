/******************************************************************************
 *         __  __           _     _         ____  _       _                   *
 *        |  \/  |_   _  __| | __| |_   _  |  _ \| | __ _(_)_ __  ___         *
 *        | |\/| | | | |/ _` |/ _` | | | | | |_) | |/ _` | | '_ \/ __|        *
 *        | |  | | |_| | (_| | (_| | |_| | |  __/| | (_| | | | | \__ \        *
 *        |_|  |_|\__,_|\__,_|\__,_|\__, | |_|   |_|\__,_|_|_| |_|___/        *
 *                                  |___/                                     *
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
#include <check.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "engine.h"
#include "character.h"
#include "connection.h"
#include "nonplayer.h"
#include "player.h"
#include "str.h"

static Character *tch = 0;
char buffer[BUF_SIZ] = {0};

void test_xwriteln(Connection *conn, const char *fmt)
{
    strcpy(buffer, fmt);
}

void char_write_setup()
{
    tch = new_char();
    Connection *c = (Connection *) malloc(sizeof(Connection));
    c->writeln = test_xwriteln;
    tch->npc = new_npc();
    tch->npc->shortDescr = str_dup("testChar");
    tch->pc = new_player(c);
    LINK(first_character, tch, next);
    LINK(first_player, tch, next_player);
}

void char_write_teardown()
{
    if (tch->pc->conn)
        free_mem(tch->pc->conn);

    extract_char(tch, true);

    tch = 0;
}


START_TEST(test_char_xwriteln)
{
    fail_if(tch == 0, "tch was not initialized");

    xwriteln(tch, "test");

    fail_if(strcmp(buffer, "test"), "xwriteln buffer was incorrect");
}
END_TEST

void parse_format_string(Character *to, char *orig, ...)
{
    va_list args;
    va_start(args, orig);
    char buf[OUT_SIZ] = {0};
    Character *vict;
    char *out = buf;

    for (const char *pstr = orig; *pstr; pstr++)
    {
        if (*pstr != '%')
        {
            *out++ = *pstr;
            continue;
        }

        const char *i;

        switch (*++pstr)
        {
        case 'N':
        case 'n':
            vict = va_arg(args, Character *);
            fail_if(vict == 0, "'N' is null");
            i = vict->npc->shortDescr;
            while ( ( *out = *i) != 0)
                out++, i++;

            break;
        }
    }

    va_end(args);

    xwriteln(to, buf);
}

START_TEST(test_char_format)
{
    parse_format_string(tch, "test %N test", tch);

    fail_if(strcmp(buffer, "test testChar test"), "parse_format_string was incorrect");
}
END_TEST


Suite *character_suite (void)
{
    Suite *s = suite_create ("Characters");

    TCase *tc_writing = tcase_create("Writing");
    tcase_add_checked_fixture (tc_writing, char_write_setup, char_write_teardown);
    tcase_add_test (tc_writing, test_char_xwriteln);
    tcase_add_test (tc_writing, test_char_format);
    suite_add_tcase(s, tc_writing);

    return s;
}

