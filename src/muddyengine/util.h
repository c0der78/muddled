
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

#ifndef UTIL_H
#define UTIL_H

#include <muddyengine/character.h>
#include <stdbool.h>
#include <time.h>

struct timezone_type {

    const char *name;		/* Name of the time zone */

    const char *zone;		/* Cities or Zones in zone crossing */

    int gmt_offset;		/* Difference in hours from Greenwich
				 * Mean Time */

    int dst_offset;		/* Day Light Savings Time offset */

};

long number_bits(int);

long number_range(long, long);

long number_fuzzy(long, int);

bool can_see(const Character *, const Character *);

bool can_see_room(const Character *, const Room *);

bool can_see_obj(const Character *, const Object *);

void char_from_room(Character *);

void char_to_room(Character *, Room *);

void obj_from_room(Object *);

void obj_from_char(Object *);

void obj_from_obj(Object *);

void equip_char(Character *, Object *, int);

void obj_to_room(Object *, Room *);

void obj_to_char(Object *, Character *);

void obj_to_obj(Object *, Object *);

Character *get_char_room(Character *, const char *);

void
actf(int, const Character *, const void *, const void *,
     const char *, ...) __attribute__ ((format(printf, 5, 6)));

void act_pos(int, position_t, const Character *, const void *,
	     const void *, const char *);
void actf_pos(int, position_t, const Character *, const void *,
	      const void *, const char *, ...)
    __attribute__ ((format(printf, 6, 7)));

void act(int, const Character *, const void *, const void *, const char *);

const char *chview(const Character *, const Character *);

const char *objview(const Object *, const Character *);

long dice(int, int);

extern const struct timezone_type timezones[];

int timezone_lookup(const char *);

const char *str_time(time_t, int, const char *);

void char_from_room(Character *);

int gold(money_t);

int silver(money_t);

Object *get_eq_char(Character *, int);

void unequip_char(Character *, Object *);

int getCurrStat(const Character *, int);

int getMaxTrain(const Character *);

int interpolate(int, int, int);

Room *find_location(Character *, const char *);

double diminishing_returns(double, double);

double factorial(int);

double combination(int, int);

#endif				/* //  #ifndef UTIL_H */
