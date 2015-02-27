
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "character.h"
#include "util.h"
#include "room.h"
#include "macro.h"
#include "nonplayer.h"
#include "player.h"
#include "skill.h"
#include "log.h"
#include "fight.h"
#include "class.h"
#include "channel.h"

const Lookup dam_types[] =
{
    {"bash", DAM_BASH},
    {"pierce", DAM_PIERCE},
    {"slash", DAM_SLASH},
    {"acid", DAM_ACID},
    {"cold", DAM_COLD},
    {"air", DAM_AIR},
    {"disease", DAM_DISEASE},
    {"earth", DAM_EARTH},
    {"energy", DAM_ENERGY},
    {"fire", DAM_FIRE},
    {"holy", DAM_HOLY},
    {"light", DAM_LIGHT},
    {"mental", DAM_MENTAL},
    {"electric", DAM_ELECTRIC},
    {"water", DAM_WATER},
    {"poison", DAM_POISON},
    {"sonic", DAM_SONIC},
    {"shadow", DAM_SHADOW},
    {0, 0}
};

void
dam_message(Character *ch, Character *victim, long dam, int dt, dam_t type)
{

    const char *attack = (dt >= 0
                          && dt <
                          max_skill) ? skill_table[dt].damage : "attack";

    if (dam <= 0)
    {

        act(TO_CHAR, ch, attack, victim, "Your $t misses $N!");

        act(TO_VICT, ch, attack, victim, "$n's $t misses you!");

        act(TO_NOTVICT, ch, attack, victim, "$n's $t misses $N!");

    }
    else
    {

        actf(TO_CHAR, ch, attack, victim,
             "Your $t hits $N for %ld damage!", dam);

        actf(TO_VICT, ch, attack, victim,
             "$n's $t hits you for %ld damage!", dam);

        actf(TO_NOTVICT, ch, attack, victim,
             "$n's $t hits $N for %ld damage!", dam);

    }

}

long xp_compute(Character *gch, Character *victim, int total_levels)
{

    long xp;

    int base_exp;

    int level_range;

    level_range = victim->level - gch->level;

    /*
     * compute the base exp
     */
    switch (level_range)
    {

    default:
        base_exp = 0;
        break;

    case -9:
        base_exp = 1;
        break;

    case -8:
        base_exp = 2;
        break;

    case -7:
        base_exp = 5;
        break;

    case -6:
        base_exp = 9;
        break;

    case -5:
        base_exp = 11;
        break;

    case -4:
        base_exp = 22;
        break;

    case -3:
        base_exp = 33;
        break;

    case -2:
        base_exp = 50;
        break;

    case -1:
        base_exp = 66;
        break;

    case 0:
        base_exp = 83;
        break;

    case 1:
        base_exp = 99;
        break;

    case 2:
        base_exp = 121;
        break;

    case 3:
        base_exp = 143;
        break;

    case 4:
        base_exp = 165;
        break;

    }

    if (level_range > 4)
        base_exp = 160 + 20 * (level_range - 4);

    /*
     * do alignment computations
     */

    int align = victim->alignment - gch->alignment;

    int change = 0;

    /*
     * if (IS_SET(victim->act,ACT_NOALIGN)) { }
     *
     * else
     */

    /*
     * monster is more good than slayer
     */
    if (align > 500)
    {

        change =
            (align - 500) * base_exp / 500 * gch->level / total_levels;

        change = UMAX(1, change);

        gch->alignment = UMAX(-MAX_ALIGN, gch->alignment - change);

    }
    /*
     * monster is more evil than slayer
     */
    else if (align < -500)
    {

        change =
            (-1 * align -
             500) * base_exp / 500 * gch->level / total_levels;

        change = UMAX(1, change);

        gch->alignment = UMIN(MAX_ALIGN, gch->alignment + change);

    }
    /*
     * //improve this someday
     */
    else
    {

        change =
            gch->alignment * base_exp / 500 * gch->level / total_levels;

        gch->alignment -= change;

    }

    /*
     * if (IS_SET(victim->act,ACT_NOALIGN)) xp = base_exp;
     *
     * else */

    /*
     * calculate exp multiplier
     */
    /*
     * for goodie two shoes
     */
    if (gch->alignment > 500)
    {

        if (victim->alignment < -750)
            xp = (base_exp * 4) / 3;

        else if (victim->alignment < -500)
            xp = (base_exp * 5) / 4;

        else if (victim->alignment > 750)
            xp = base_exp / 4;

        else if (victim->alignment > 500)
            xp = base_exp / 2;

        else if (victim->alignment > 250)
            xp = (base_exp * 3) / 4;

        else
            xp = base_exp;

    }

    /*
     * for baddies
     */

    else if (gch->alignment < -500)
    {
        if (victim->alignment > 750)
            xp = (base_exp * 5) / 4;

        else if (victim->alignment > 500)
            xp = (base_exp * 11) / 10;

        else if (victim->alignment < -750)
            xp = base_exp / 2;

        else if (victim->alignment < -500)
            xp = (base_exp * 3) / 4;

        else if (victim->alignment < -250)
            xp = (base_exp * 9) / 10;

        else
            xp = base_exp;

    }

    /*
     * a little good
     */
    else if (gch->alignment > 200)
    {

        if (victim->alignment < -500)
            xp = (base_exp * 6) / 5;

        else if (victim->alignment > 750)
            xp = base_exp / 2;

        else if (victim->alignment > 0)
            xp = (base_exp * 3) / 4;

        else
            xp = base_exp;

    }
    /*
     * a little bad
     */
    else if (gch->alignment < -200)
    {

        if (victim->alignment > 500)
            xp = (base_exp * 6) / 5;

        else if (victim->alignment < -750)
            xp = base_exp / 2;

        else if (victim->alignment < 0)
            xp = (base_exp * 3) / 4;

        else
            xp = base_exp;

    }
    /*
     * neutral
     */
    else
    {

        if (victim->alignment > 500 || victim->alignment < -500)
            xp = (base_exp * 4) / 3;

        else if (victim->alignment < 200 && victim->alignment > -200)
            xp = base_exp / 2;

        else
            xp = base_exp;

    }

    xp = base_exp;

    /*
     * // more exp at the low levels if (gch->level < 6) xp = 10 * xp /
     * (gch->level + 4);
     *
     * // less at high if (gch->level > 35 ) xp = 15 * xp / (gch->level -
     * 25 );
     *
     * // reduce for playing time
     *
     * { // compute quarter-hours per level time_per_level = 4 *
     * (gch->played + (int) (current_time - gch->logon))/3600 /
     * gch->level;
     *
     * time_per_level = URANGE(2,time_per_level,12); if (gch->level < 15)
     * // make it a curve time_per_level = UMAX(time_per_level,(15 -
     * gch->level)); xp = xp * time_per_level / 12; }
     */

    int xp_group =
        (exp_table[UMIN(MAX_PLAYABLE_LEVEL, gch->level) / LEVEL_GROUPS] /
         LEVEL_GROUPS);

    xp *= xp_group;

    /*
     * randomize the rewards
     */
    xp = number_range((long)(xp * 3.0 / 4.0), (long)(xp * 5.0 / 4.0));

    /*
     * adjust for grouping
     */
    xp = xp * gch->level / (UMAX(1, total_levels - 1));

    return xp;

}

void advance_level(Character *ch, bool hide)
{

    long add_hp;

    long add_mana;

    long add_move;

    /*
     * ch->pcdata->last_level = //(ch->played + (int) (current_time -
     * ch->logon)) / 3600; //sprintf(buf, "the %s",
     * //title_table[ch->class][ch->level][ch->sex == SEX_FEMALE ? 1 :
     * 0]); //set_title(ch, buf);
     */

    add_hp =
        getCurrStat(ch,
                    STAT_CON) / exp_table[UMIN(MAX_PLAYABLE_LEVEL,
                                          ch->level) / LEVEL_GROUPS];

    add_mana =
        number_range(getCurrStat(ch, STAT_INT),
                     (2 * getCurrStat(ch, STAT_INT) +
                      getCurrStat(ch, STAT_WIS)) / 5);

    if (*ch->classes != -1 && !class_table[*ch->classes].fMana)
        add_mana /= 2;

    add_move =
        number_range(getCurrStat(ch, STAT_DEX), (getCurrStat(ch, STAT_CON)
                     + getCurrStat(ch,
                                   STAT_DEX)) /
                     6);

    add_hp = ch->level * 0.4f;

    add_mana = ch->level * 0.4f;

    add_move = ch->level * 0.4f;

    add_hp = add_hp * 9 / 10;

    add_mana = add_mana * 9 / 10;

    add_move = add_move * 9 / 10;

    add_hp = UMAX(2, add_hp);

    add_mana = UMAX(2, add_mana);

    add_move = UMAX(6, add_move);

    ch->maxHit += add_hp;

    ch->maxMana += add_mana;

    ch->maxMove += add_move;

    ch->pc->permHit += add_hp;

    ch->pc->permMana += add_mana;

    ch->pc->permMove += add_move;

    if (!hide)
    {

        writelnf(ch,
                 "You gain %d hit point%s, %d mana, %d move.",
                 add_hp, add_hp == 1 ? "" : "s", add_mana, add_move);

    }
    return;

}

void gain_exp(Character *ch, long gain)
{

    if (!ch->pc || is_immortal(ch))
        return;

    long tolvl = exp_to_level(ch);

    int oldlevel = ch->level;

    ch->pc->experience = UMAX(tolvl, ch->pc->experience + gain);

    while (ch->level < MAX_PLAYABLE_LEVEL && ch->pc->experience >= tolvl)
    {

        writeln(ch, "You raise a level!!  ");

        ch->level += 1;

        log_info("%s gained level %d", ch->name, ch->level);

        advance_level(ch, false);

        save_player(ch);

    }

    if (ch->level != oldlevel)
    {

        announce(ch, INFO_LEVEL, "$n has attained level %d!",
                 ch->level);

    }
}

bool damage(Character *ch, Character *victim, long dam, int dt, dam_t type)
{

    dam_message(ch, victim, dam, dt, type);

    if (dam == 0)
        return false;

    victim->hit -= dam;

    if (victim->hit <= 0)
    {

        act(TO_CHAR, ch, NULL, victim, "~R$N is DEAD!~x");

        act(TO_VICT, ch, NULL, victim, "~RYou are DEAD!~x");

        act(TO_NOTVICT, ch, NULL, victim, "~R$N is DEAD!~x");

        ch->fighting = 0;

        long xp = xp_compute(ch, victim, ch->level);

        writelnf(ch, "~GYou gain %d experience!~x", xp);

        gain_exp(ch, xp);

        if (victim->pc == 0)
        {

            char_from_room(victim);

            extract_char(victim, true);

        }
        else
        {

            victim->fighting = 0;

            announce(victim, INFO_DEATH,
                     "Alas, $n has been killed by %s.", NAME(ch));

        }

        return true;

    }
    return true;

}

void one_hit(Character *ch, Character *victim, int dt, dam_t type)
{

    if (victim == ch || !ch || !victim)
        return;

    if (victim->position == POS_DEAD || ch->inRoom != victim->inRoom)
        return;

    int thac0 = interpolate(ch->level, class_table[*ch->classes].thac0,
                            class_table[*ch->classes].thac32);

    if (thac0 < 0)
        thac0 = thac0 / 2;

    if (thac0 < -5)
        thac0 = -5 + (thac0 + 5) / 2;

    int victim_ac =
        type == DAM_UNDEFINED ? DAM_UNDEFINED : victim->resists[type];

    long diceroll;

    if (victim_ac < -15)
        victim_ac = (victim_ac + 15) / 5 - 15;

    while ((diceroll = number_bits(5)) >= 20) ;

    if (diceroll == 0 || (diceroll != 19 && diceroll < thac0 - victim_ac))
    {

        damage(ch, victim, 0, dt, type);

        return;

    }
    long dam = number_range(0, ch->level - victim->level);

    dam +=
        number_range(getCurrStat(ch, STAT_STR) / 2,
                     getCurrStat(ch, STAT_STR));

    damage(ch, victim, dam, dt, type);

}

void multi_hit(Character *ch, Character *victim, int dt, dam_t type)
{

    one_hit(ch, victim, dt, type);

    if (ch->fighting != victim)
        return;

    if (number_range(1, 100) < 50)
        one_hit(ch, victim, dt, type);

}
