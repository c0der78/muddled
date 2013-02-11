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
#include <stdlib.h>
#include <stdio.h>
#include <muddyengine/hashmap.h>

static int check = 6;

void hashmap_foreach(void *data)
{
    int d = *(int*) data;

    fail_if(d +1 != check);

    check--;
}
START_TEST(test_foreach)
{
    hashmap *map = new_hashmap(0);

    check = 6;

    hm_insert(map, (void *) 5, 5);
    hm_insert(map, (void *) 4, 4);
    hm_insert(map, (void *) 3, 3);

    hm_foreach(map, hashmap_foreach);

    destroy_hashmap(map);
}
END_TEST

START_TEST(test_iterate)
{
    hashmap *map = new_hashmap(0);

    hm_insert(map, (void *) 5, 5);
    hm_insert(map, (void *) 4, 4);
    hm_insert(map, (void *) 3, 3);

    check = 5;

    for(void * data = hm_start(map); hm_hasnext(map); data = hm_next(map))
    {
        fail_if(*(int*)data != check);

        check--;
    }
    destroy_hashmap(map);
}
END_TEST

Suite *hashmap_suite (void)
{
    Suite *s = suite_create ("Hashmap");

    /* Core test case */
    TCase *tc_core = tcase_create ("Core");
    tcase_add_test (tc_core, test_iterate);
    tcase_add_test (tc_core, test_foreach);
    suite_add_tcase (s, tc_core);

    return s;
}

