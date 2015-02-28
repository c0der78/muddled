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
#include "area.h"

START_TEST(test_area_new)
{
    Area *area = new_area();

    fail_unless(area != 0, "null value created");

    fail_unless(area->id == 0, "account id was not zero on initialization");

    fail_unless(area->next == 0, "account player was not zero initialized");

    fail_unless(area->npcs == 0, "area first_npc was not zero initialized");

    fail_unless(area->rooms == 0, "area first_room was not zero initialized");

    fail_unless(area->objects == 0, "area first_object was not zero initialized");

    destroy_area(area);
}
END_TEST


Suite *area_suite (void)
{
    Suite *s = suite_create ("Areas");

    /* Core test case */
    TCase *tc_core = tcase_create ("Core");
    tcase_add_test (tc_core, test_area_new);
    suite_add_tcase (s, tc_core);

    return s;
}

