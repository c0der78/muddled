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
#include <check.h>
#include "../src/flag.h"
#include "../src/explored.h"
#include "../src/room.h"
#include "../src/area.h"
#include "../src/log.h"
#include <stdio.h>

int bitcount(int);

START_TEST(test_bitcount)
{
    int c = 0;

    c |= (1 << 4);
    c |= (1 << 6);
    c |= (1 << 10);

    fail_if(bitcount(c) != 3);
}
END_TEST

START_TEST(test_roomcount)
{
    Flag *explored = new_flag();

    set_bit(explored, 1);
    set_bit(explored, 2);
    set_bit(explored, 10);

    fail_if(roomcount(explored) != 3);

    destroy_flags(explored);
}
END_TEST

START_TEST(test_explored_rle)
{
    Flag *explored = new_flag();

    Room *room = new_room();
    room->id = 1;
    LINK(room_hash[room->id % ID_HASH], room, next);

    set_bit(explored, room->id);

    const char *pstr = get_explored_rle(explored);

    remove_bit(explored, room->id);

    convert_explored_rle(explored, pstr);

    fail_if(roomcount(explored) != 1);

    destroy_flags(explored);

    UNLINK(room_hash[room->id % ID_HASH], Room, room, next);

    destroy_room(room);

}
END_TEST

Suite *explored_suite (void)
{
    Suite *s = suite_create ("Explored");

    /* Core test case */
    TCase *tc_core = tcase_create ("Core");
    tcase_add_test (tc_core, test_bitcount);
    tcase_add_test (tc_core, test_roomcount);
    tcase_add_test (tc_core, test_explored_rle);
    suite_add_tcase (s, tc_core);

    return s;
}
