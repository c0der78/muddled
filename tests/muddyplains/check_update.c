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
#include <stdio.h>
#include <muddyengine/room.h>
#include <muddyengine/string.h>
#include <muddyengine/object.h>
#include <muddyengine/lookup.h>

void reset_room(Room *);

START_TEST(test_reset_room)
{

    const char *str = "npc(1,2)\nobj(3)\nequip(4,'lfinger')\nput( 5 )";

    const char *delims = "(),";

    char buf[ARG_SIZ];

    while (str && *str != 0)
    {
        str = get_line(str, buf);

        char *tmp;
        long id = 0;

        if(buf[0] == 0 || (tmp = strtok(buf, delims)) == 0)
            continue;

        if(!str_cmp("npc", tmp))
        {
            if((tmp = strtok(0, delims)) == 0 || (id = atol(tmp)) == 0)
                continue;

            int max = 1;

            if((tmp = strtok(0, delims)) != 0)
            {
                max = atoi(tmp);
            }

            fail_if(id != 1, "invalid npc id");
            fail_if(max != 2, "invalid npc max");

            continue;
        }

        if(!str_cmp("obj", tmp))
        {
            if((tmp = strtok(0, delims)) == 0 || (id = atol(tmp)) == 0)
                continue;

            int max = 1;

            if((tmp = strtok(0, delims)) != 0)
            {
                max = atoi(tmp);
            }

            fail_if(id != 3, "invalid obj id");
            fail_if(max != 1, "invalid obj max");

            continue;
        }

        if(!str_cmp("give", tmp))
        {


            if((tmp = strtok(0, delims)) == 0 || (id = atol(tmp)) == 0)
                continue;

            int max = 1;

            if((tmp = strtok(0, delims)) != 0)
            {
                max = atoi(tmp);
            }

            continue;
        }

        if(!str_cmp("equip", tmp))
        {

            if((tmp = strtok(0, delims)) == 0 || (id = atol(tmp)) == 0)
                continue;

            one_argument(strtok(0, delims), buf);

            int type = value_lookup(wear_types, buf);

            fail_if(id != 4, "invalid equip id");
            fail_if(type != WEAR_FINGER_2, "invalid equip wear type");

            continue;
        }

        if (!str_cmp("put", tmp))
        {
            if((tmp = strtok(0, delims)) == 0 || (id = atol(tmp)) == 0)
                continue;

            int max = 1;

            if((tmp = strtok(0, delims)) != 0)
            {
                max = atoi(tmp);
            }

            fail_if(id != 5, "invalid put id");

            continue;
        }
    }

}
END_TEST


Suite *update_suite (void)
{
    Suite *s = suite_create ("Updates");

    /* Core test case */
    TCase *tc_core = tcase_create ("Core");
    tcase_add_test (tc_core, test_reset_room);
    suite_add_tcase (s, tc_core);

    return s;
}

