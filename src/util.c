
/******************************************************************************
 *                                       _     _ _          _                 *
 *                   _ __ ___  _   _  __| | __| | | ___  __| |                *
 *                  | '_ ` _ \| | | |/ _` |/ _` | |/ _ \/ _` |                *
 *                  | | | | | | |_| | (_| | (_| | |  __/ (_| |                *
 *                  |_| |_| |_|\__,_|\__,_|\__,_|_|\___|\__,_|                *
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include "character.h"
#include "nonplayer.h"
#include "room.h"
#include "object.h"
#include "engine.h"
#include "string.h"
#include "macro.h"
#include "flag.h"
#include "player.h"
#include "log.h"
#include "util.h"
#include "explored.h"
#include "lookup.h"
#include "affect.h"
#include <ctype.h>

long number_mm(void)
{

    return rand() >> 6;

}

long number_range(long from, long to)
{

    int power;

    long number;

    if (from == 0 && to == 0)
        return 0;

    if ((to = to - from + 1) <= 1)
        return from;

    for (power = 2; power < to; power <<= 1) ;

    while ((number = number_mm() & (power - 1)) >= to) ;

    return from + number;

}

int number_percent(void)
{

    long percent;

    while ((percent = number_mm() & (128 - 1)) > 99) ;

    return (int)(1 + percent);

}

long number_bits(int width)
{

    return number_mm() & ((1 << width) - 1);

}

/*
 * Roll some dice.
 */
long dice(int number, int size)
{

    int idice;

    long sum;

    switch (size)
    {

    case 0:
        return 0;

    case 1:
        return number;

    }

    for (idice = 0, sum = 0; idice < number; idice++)
        sum += number_range(1, size);

    return sum;

}

long number_fuzzy(long num, int fuzziness)
{

    return number_range(num - fuzziness, num + fuzziness);

}

double diminishing_returns(double val, double scale)
{

    if (val < 0)
        return -diminishing_returns(-val, scale);

    double mult = val / scale;

    double trinum = (sqrt(8.0f * mult + 1.0f) - 1.0f) / 2.0f;

    return trinum * scale;

}

double factorial(int n)
{

    if (n < 0)
        return 0;

    double f = 1;

    while (n > 1)
        f *= --n;

    return f;

}

double combination(int n, int k)
{

    log_trace("f(%d)=%f", n, factorial(n));

    log_trace("f(%d)=%f", k, factorial(k));

    log_trace("f(%d-%d)=%f", n, k, factorial(n - k));

    double total = factorial(n) / factorial(k) * factorial(n - k);

    log_trace("c(%d,%d)=%f", n, k, total);

    return total;

}

/*
 * Simple linear interpolation.
 */
int interpolate(int level, int value_00, int value_32)
{

    return value_00 + level * (value_32 - value_00) / 32;

}

bool can_see(const Character *ch, const Character *ch2)
{

    return true;

}

bool can_see_room(const Character *ch, const Room *room)
{

    return true;

}

bool can_see_obj(const Character *ch, const Object *obj)
{

    return true;

}

void char_from_room(Character *ch)
{

    if (ch->inRoom)
    {

        UNLINK(ch->inRoom->characters, Character, ch, next_in_room);

    }
    ch->inRoom = 0;

    ch->next_in_room = 0;

}

void char_to_room(Character *ch, Room *room)
{

    ch->inRoom = room;

    if (!ch->inRoom)
        return;

    if (is_explorable(room) && ch->pc)
        set_bit(ch->pc->explored, room->id);

    LINK(ch->inRoom->characters, ch, next_in_room);

}

void obj_to_room(Object *obj, Room *room)
{

    obj->inRoom = room;

    LINK(room->objects, obj, next_content);

    obj->carriedBy = 0;

    obj->inObj = 0;

}

void obj_to_obj(Object *obj, Object *container)
{

    LINK(container->contains, obj, next_content);

    obj->inObj = container;

    obj->inRoom = 0;

    obj->carriedBy = 0;

}

void obj_from_room(Object *obj)
{

    Room *room;

    if ((room = obj->inRoom) == 0)
    {

        log_error("obj_from_room: NULL");

        return;

    }
    UNLINK(room->objects, Object, obj, next_content);

    obj->next_content = 0;

    obj->inRoom = 0;

}

void obj_to_char(Object *obj, Character *ch)
{

    LINK(ch->carrying, obj, next_content);

    obj->inRoom = 0;

    obj->carriedBy = ch;

    obj->inObj = 0;

}

/*
 * Take an obj from its character.
 */
void obj_from_char(Object *obj)
{

    Character *ch;

    if ((ch = obj->carriedBy) == NULL)
    {

        log_error("obj_from_char: null ch.");

        return;

    }
    /*
     * if ( obj->wear_loc != WEAR_NONE ) unequip_char( ch, obj );
     */

    UNLINK(ch->carrying, Object, obj, next_content);

    obj->carriedBy = NULL;

    obj->next_content = NULL;

    /*
     * ch->carry_number -= get_obj_number( obj ); ch->carry_weight -=
     * get_obj_weight( obj );
     */
    return;

}

void obj_from_obj(Object *obj)
{

    Object *obj_from;

    if ((obj_from = obj->inObj) == NULL)
    {

        log_error("obj_from_obj: null obj_from.");

        return;

    }
    UNLINK(obj_from->contains, Object, obj, next_content);

    obj->next_content = NULL;

    obj->inObj = NULL;

    /*
     * for ( ; obj_from != NULL; obj_from = obj_from->inObj ) { if (
     * obj_from->carried_by != NULL ) { obj_from->carried_by->carry_number
     * -= get_obj_number( obj ); obj_from->carried_by->carry_weight -=
     * get_obj_weight( obj ) * WEIGHT_MULT(obj_from) / 100; }
     */

    return;

}

Object *get_eq_char(Character *ch, int iWear)
{

    if (ch == NULL)
        return NULL;

    for (Object *obj = ch->carrying; obj != NULL; obj = obj->next_content)
    {

        if (obj->wearLoc == iWear)
            return obj;

    }

    return NULL;

}

void equip_char(Character *ch, Object *obj, int iWear)
{

    // int i;

    if (get_eq_char(ch, iWear) != NULL)
    {

        log_bug("char %" PRId64 " already equipped (obj %" PRId64
                ") in room %" PRId64 " (%s).", ch->id, obj->id,
                ch->inRoom ? ch->inRoom->id : 0,
                wear_types[iWear].name);

        return;

    }
    /*
     * if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch) ) || (
     * IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch) ) || (
     * IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) ) { act(
     * "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR ); act(
     * "$n is zapped by $p and drops it.", ch, obj, NULL, TO_ROOM );
     * obj_from_char( obj ); obj_to_room( obj, ch->inRoom ); return; }
     *
     * for (i = 0; i < 4; i++) ch->armor[i] -= apply_ac( obj, iWear,i );
     */
    obj->wearLoc = iWear;

    for (Affect *paf = obj->affects; paf; paf = paf->next)
    {

        affect_modify(ch, paf, true);

    }

    /*
     * if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 &&
     * ch->inRoom != NULL ) ++ch->inRoom->light;
     */

    return;

}

void unequip_char(Character *ch, Object *obj)
{

    if (obj->wearLoc == WEAR_NONE)
    {

        log_bug("Unequip_char: already unequipped.");

        return;

    }
    for (Affect *paf = obj->affects; paf; paf = paf->next)
    {

        affect_modify(ch, paf, false);

    }
    obj->wearLoc = WEAR_NONE;

    return;

}

Character *get_char_room(Character *ch, const char *argument)
{

    char arg[BUF_SIZ];

    Character *rch;

    int count;

    long number = number_argument(argument, arg);

    count = 0;

    if (!str_cmp(arg, "self"))
        return ch;

    for (rch = ch->inRoom->characters; rch != NULL; rch = rch->next_in_room)
    {

        if (!can_see(ch, rch) || !is_name(arg, rch->name))
            continue;

        if (++count == number)
            return rch;

    }

    return NULL;

}

const char *chview(const Character *ch, const Character *looker)
{

    static char buf[5][100];

    static int index;

    ++index, index %= 5;

    /*
     * if (viewer can't see ch) { strcpy(buf[index], "someone"); } else
     */

    strcpy(buf[index], NAME(ch));

    return buf[index];

}

const char *objview(const Object *obj, const Character *looker)
{

    static char buf[5][100];

    static int index;

    ++index, index %= 5;

    /*
     * if (viewer can't see obj) { strcpy(buf[index], "something"); } else
     */

    strcpy(buf[index], obj->shortDescr);

    return buf[index];

}

void act(int type, const Character *ch, const void *arg1, const void *arg2,
         const char *orig)
{
    act_pos(type, POS_RESTING, ch, arg1, arg2, orig);
}

void act_pos(int type, position_t minPos, const Character *ch,
             const void *arg1, const void *arg2, const char *orig)
{

    Character *vch = (Character *) arg2;

    Object *obj1 = (Object *) arg1;

    Object *obj2 = (Object *) arg2;

    const char *str, *i = NULL;

    char *point;

    char buf[BUF_SIZ];

    Character *to;

    const char *he_she[] = { "it", "he", "she", "it" };

    const char *him_her[] = { "it", "him", "her", "it" };

    const char *his_her[] = { "its", "his", "hers", "its" };

    if (ch == 0 || ch->inRoom == 0 || !orig || !*orig)
        return;

    to = ch->inRoom->characters;

    if (type & TO_VICT)
    {

        if (vch == NULL)
        {

            log_warn("Act: null vch with TO_VICT.");

            return;

        }
        if (vch->inRoom == NULL)
            return;

        to = vch->inRoom->characters;

    }
    if (type & TO_WORLD)
    {

        to = first_player;

    }
    for (; to != NULL;
            to = (type & TO_WORLD) ? to->next_player : to->next_in_room)
    {

        if (to->position > minPos)
            continue;

        if ((type & TO_CHAR) && to != ch)
            continue;

        if ((type & TO_VICT) && (to != vch || to == ch))
            continue;

        if ((type & (TO_ROOM | TO_WORLD)) && to == ch)
            continue;

        if ((type & TO_NOTVICT) && (to == ch || to == vch))
            continue;

        point = buf;

        str = orig;

        while (*str != 0)
        {

            if (*str != '$')
            {

                *point++ = *str++;

                continue;

            }
            ++str;

            i = "<@@@>";

            if (!arg2 && isupper((int)*str))
            {

                log_error("act: missing arg2 for code %d.",
                          *str);

            }
            else
            {

                switch (*str)
                {

                default:

                    log_error("act: bad code %c.", *str);

                    break;

                case 't':

                    i = (const char *)arg1;

                    break;

                case 'T':

                    i = (const char *)arg2;

                    break;

                case 'n':

                    i = chview(ch, to);

                    break;

                case 'N':

                    i = chview(vch, to);

                    break;

                case 'e':

                    i = he_she[ch->sex];

                    break;

                case 'E':

                    i = he_she[vch->sex];

                    break;

                case 'm':

                    i = him_her[ch->sex];

                    break;

                case 'M':

                    i = him_her[vch->sex];

                    break;

                case 's':

                    i = his_her[ch->sex];

                    break;

                case 'S':

                    i = his_her[vch->sex];

                    break;

                case 'o':

                case 'p':
                    // rom compatibility
                    i = objview(obj1, to);

                    break;

                case 'O':

                case 'P':
                    // rom compatibility
                    i = objview(obj2, to);

                    break;

                }

            }

            ++str;

            while ((*point = *i) != 0)
                ++point, ++i;

        }

        *point = 0;

        buf[0] = UPPER(buf[0]);

        if (to->pc)
        {

            writeln(to, buf);

        }
    }

}

void
actf(int type, const Character *ch,
     const void *arg1, const void *arg2, const char *fmt, ...)
{

    va_list args;

    char buf[BUF_SIZ];

    va_start(args, fmt);

    vsnprintf(buf, sizeof(buf), fmt, args);

    va_end(args);

    act_pos(type, POS_RESTING, ch, arg1, arg2, buf);

}

void
actf_pos(int type, position_t minPos, const Character *ch,
         const void *arg1, const void *arg2, const char *fmt, ...)
{
    va_list args;

    char buf[BUF_SIZ];

    va_start(args, fmt);

    vsnprintf(buf, sizeof(buf), fmt, args);

    va_end(args);

    act_pos(type, minPos, ch, arg1, arg2, buf);
}

const struct timezone_type timezones[] =
{
    {"GMT-12", "Eniwetok", -12, 0},
    {"GMT-11", "Samoa", -11, 0},
    {"GMT-10", "Hawaii", -10, 0},
    {"GMT-9", "Alaska", -9, 0},
    {"GMT-8", "PST, Pacific US", -8, -7},
    {"GMT-7", "MST, Mountain US", -7, -6},
    {"GMT-6", "CST, Central US", -6, -5},
    {"GMT-5", "EST, Eastern US", -5, -4},
    {"GMT-4", "Atlantic, Canada", -4, 0},
    {"GMT-3", "Brazilia, Buenos Aries", -3, 0},
    {"GMT-2", "Mid-Atlantic", -2, 0},
    {"GMT-1", "Cape Verdes", -1, 0},
    {"GMT", "Greenwich Mean Time, Greenwich", 0, 0},
    {"GMT+1", "Berlin, Rome", 1, 0},
    {"GMT+2", "Israel, Cairo", 2, 0},
    {"GMT+3", "Moscow, Kuwait", 3, 0},
    {"GMT+4", "Abu Dhabi, Muscat", 4, 0},
    {"GMT+5", "Islamabad, Karachi", 5, 0},
    {"GMT+6", "Almaty, Dhaka", 6, 0},
    {"GMT+7", "Bangkok, Jakarta", 7, 0},
    {"GMT+8", "Hong Kong, Beijing", 8, 0},
    {"GMT+9", "Tokyo, Osaka", 9, 0},
    {"GMT+10", "Sydney, Melbourne, Guam", 10, 0},
    {"GMT+11", "Magadan, Soloman Is.", 11, 0},
    {"GMT+12", "Fiji, Wellington, Auckland", 12, 0},
    {0, 0, 0, 0}
};

int timezone_lookup(const char *arg)
{

    if (!arg || !*arg)
        return -1;

    for (int i = 0; timezones[i].name != 0; i++)
    {

        if (!str_cmp(arg, timezones[i].name))
            return i;

    }

    return -1;

}

extern long timezone;

const char *str_time(time_t timet, int tz, const char *format)
{

    static char buf_new[5][100];

    static int i;

    char *result;

    // rotate buffers
    ++i, i %= 5;

    result = buf_new[i];

    if (timet <= 0)
    {

        timet = time(0);

    }
    if (tz > -1 && tz < (sizeof(timezones) / sizeof(timezones[0])) - 1)
    {

        timet += timezone;  /* timezone external variable in time.h
                     * holds the difference in seconds to GMT.
                     */

        timet += (60 * 60 * timezones[tz].gmt_offset);  /* Add the offset
                                 * hours */

    }
    strftime(result, 100,
             format != NULL ? format : "%a %b %d %I:%M:%S %p %Y",
             localtime(&timet));

    return result;

}


time_t iso8601_parse(const char *str)
{
    int y, M, d, h, m;
    float s;
    int tzh, tzm;
    struct tm time;

    if (8 > sscanf(str, "%d-%d-%dT%d:%d:%f%d:%dZ", &y, &M, &d, &h, &m, &s, &tzh, &tzm))
    {
        sscanf(str, "%d-%d-%dT%d:%d:%fZ", &y, &M, &d, &h, &m, &s);
    }

    time.tm_year = y - 1900; /* Year since 1900 */
    time.tm_mon = M - 1;     /* 0-11 */
    time.tm_mday = d;        /* 1-31 */
    time.tm_hour = h;        /* 0-23 */
    time.tm_min = m;         /* 0-59 */
    time.tm_sec = (int) s;    /* 0-61 */
    time.tm_isdst = 0;     /* auto check daylight savings time */

    return mktime(&time) - timezone;
}

char *iso8601_format(time_t time, char *buf, size_t bufsiz)
{
    strftime(buf, bufsiz, "%FT%TZ", gmtime(&current_time));
    return buf;
}


int gold(money_t value)
{

    return (int)value;

}

int silver(money_t value)
{

    return (int)((float)(value - (int)value) * 100.0);

}

int getMaxTrain(const Character *ch)
{

    if (is_immortal(ch))
        return MAX_STAT_VALUE;

    int max = MAX_PLAYABLE_LEVEL / 4;

    if (ch->level < MAX_PLAYABLE_LEVEL / 4)
        return max;

    if (ch->level > MAX_PLAYABLE_LEVEL / 3)
        max += (ch->level - (MAX_PLAYABLE_LEVEL / 3));

    if (ch->level > MAX_PLAYABLE_LEVEL / 2)
        max += (ch->level - (MAX_PLAYABLE_LEVEL / 2));

    return max;

}

int getCurrStat(const Character *ch, int stat)
{

    int max = getMaxTrain(ch);

    return URANGE(3, ch->stats[stat] + ch->statMods[stat], max);

}

Room *find_location(Character *ch, const char *arg)
{

    Character *victim;

    Object *obj;

    Area *area;

    if (is_number(arg) && !is_player(ch))
        return get_room_by_id(atoi(arg));

    if ((victim = get_char_world(ch, arg)) != NULL)
        return victim->inRoom;

    if ((area = area_lookup(arg)) != NULL)
        return area->rooms;

    if ((obj = get_obj_world(ch, arg)) != NULL)
        return obj->inRoom;

    return NULL;

}
