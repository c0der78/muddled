
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
#include <unistd.h>
#include <inttypes.h>
#include <muddyengine/nonplayer.h>
#include <muddyengine/character.h>
#include <muddyengine/area.h>
#include <muddyengine/object.h>
#include <muddyengine/player.h>
#include <muddyengine/affect.h>
#include <muddyengine/macro.h>
#include <muddyengine/string.h>
#include <muddyengine/log.h>
#include <muddyengine/room.h>
#include <muddyengine/util.h>
#include <muddyengine/hint.h>
#include <muddyengine/skill.h>
#include <muddyengine/fight.h>
#include <muddyengine/lookup.h>
#include <muddyengine/account.h>
#include "update.h"
#include "client.h"

void reset_room(Room * room)
{
    static const char *delims = "(),";

    void wear_obj(Character *, Object *, bool);

    if (room == 0 || room->reset == 0)
	return;

    const char *str = room->reset;

    char buf[ARG_SIZ];

    Character *lastNPC = 0;
    Object *lastOBJ = 0;

    while (str && *str != 0) {
	str = get_line(str, buf);

	char *tmp;
	identifier_t id = 0;

	if (buf[0] == 0 || (tmp = strtok(buf, delims)) == 0)
	    continue;

	tmp = trim(tmp);

	if (!str_cmp("npc", tmp)) {
	    if ((tmp = strtok(0, delims)) == 0
		|| (id = atol(tmp)) == 0
		|| (lastNPC = load_npc(id)) == 0) {
		log_error("npc reset - could not load npc %" PRId64
			  " (room %" PRId64 ")", id, room->id);
		continue;
	    }
	    int max = 1;

	    if ((tmp = strtok(0, delims)) != 0) {
		max = atoi(tmp);
	    }
	    int count = 0;

	    for (Character * rch = room->characters; rch;
		 rch = rch->next_in_room)
		if (rch->id == id)
		    count++;

	    if (count >= max)
		continue;

	    char_to_room(lastNPC, room);

	    continue;
	}
	if (!str_cmp("obj", tmp)) {
	    if ((tmp = strtok(0, delims)) == 0
		|| (id = atol(tmp)) == 0
		|| (lastOBJ = load_object(id)) == 0) {
		log_error("obj reset - could not load obj %" PRId64
			  " (room %" PRId64 ")", id, room->id);
		continue;
	    }
	    int max = 1;

	    if ((tmp = strtok(0, delims)) != 0) {
		max = atoi(tmp);
	    }
	    int count = 0;

	    for (Object * obj = room->objects; obj;
		 obj = obj->next_content)
		if (obj->id == id)
		    count++;

	    if (count >= max)
		continue;

	    obj_to_room(lastOBJ, room);
	    continue;
	}
	if (!str_cmp("give", tmp)) {
	    if (lastNPC == 0) {
		log_error("give reset - no last npc (room %" PRId64 ")",
			  room->id);
		continue;
	    }
	    if ((tmp = strtok(0, delims)) == 0
		|| (id = atol(tmp)) == 0
		|| (lastOBJ = load_object(id)) == 0) {
		log_error("give reset - could not load obj %" PRId64
			  " (room %" PRId64 ")", id, room->id);
		continue;
	    }
	    int max = 1;

	    if ((tmp = strtok(0, delims)) != 0) {
		max = atoi(tmp);
	    }
	    int count = 0;

	    for (Object * obj = lastNPC->carrying; obj;
		 obj = obj->next_content)
		if (obj->id == id)
		    count++;

	    if (count >= max)
		continue;

	    obj_to_char(lastOBJ, lastNPC);
	    continue;
	}
	if (!str_cmp("equip", tmp)) {
	    if (lastNPC == 0) {
		log_error("equip reset - no last npc (room %" PRId64 ")",
			  room->id);
		continue;
	    }
	    if ((tmp = strtok(0, delims)) == 0
		|| (id = atol(tmp)) == 0
		|| (lastOBJ = load_object(id)) == 0) {
		log_error("equip_reset - could not load obj %" PRId64
			  " (room %" PRId64 ")", id, room->id);
		continue;
	    }
	    obj_to_char(lastOBJ, lastNPC);

	    if ((tmp = strtok(0, delims)) != 0) {
		one_argument(tmp, buf);

		wear_type type = (wear_type) value_lookup(wear_types, buf);

		equip_char(lastNPC, lastOBJ, type);
	    } else {
		wear_obj(lastNPC, lastOBJ, false);
	    }

	    continue;
	}
	if (!str_cmp("put", tmp)) {
	    if (lastOBJ == 0) {
		log_error("put reset - no last obj (room %" PRId64 ")",
			  room->id);
		continue;
	    }
	    Object *obj;

	    if ((tmp = strtok(0, delims)) == 0
		|| (id = atol(tmp)) == 0 || (obj = load_object(id)) == 0) {
		log_error("put reset - could not load obj %" PRId64
			  " ( room %" PRId64 ")", id, room->id);
		continue;
	    }
	    int max = 1;

	    if ((tmp = strtok(0, delims)) != 0) {
		max = atoi(tmp);
	    }
	    obj_to_obj(obj, lastOBJ);
	    continue;
	}
    }
}

void reset_area(Area * area)
{
    for (Room * room = area->rooms; room != 0; room = room->next_in_area) {
	reset_room(room);
    }
}

void area_update()
{
    for (Area * area = first_area; area != 0; area = area->next) {
	reset_area(area);
    }

}

long hit_gain(Character * ch)
{
    long gain;
    //int           number;

    if (ch->inRoom == NULL)
	return 0;

    if (!ch->pc) {
	gain = 10 + ch->level;
	//if (IS_AFFECTED(ch, AFF_REGENERATION))
	//gain *= 2;

	switch (ch->position) {
	default:
	    gain /= 2;
	    break;
	case POS_SLEEPING:
	    gain = 3 * gain / 2;
	    break;
	case POS_RESTING:
	    break;
	case POS_FIGHTING:
	    gain /= 3;
	    break;
	}

    } else {
	gain = UMAX(3, getCurrStat(ch, STAT_CON) - 3 + ch->level);
	//gain += bonus(ch) * 2;
	/*
	 * number = number_percent(); if (number <
	 * ch->get_skill(gsn_fast_healing)) { gain += number * gain /
	 * 100; gain += ch->hit * number / 100; if (ch->hit <
	 * ch->max_hit) check_improve(ch, gsn_fast_healing, true, 8);
	 * }
	 */

	switch (ch->position) {
	default:
	    gain /= 4;
	    break;
	case POS_SLEEPING:
	    break;
	case POS_RESTING:
	    gain /= 2;
	    break;
	case POS_FIGHTING:
	    gain /= 6;
	    break;
	}

	if (ch->pc->condition[COND_HUNGER] >= 20)
	    gain /= 2;

	if (ch->pc->condition[COND_THIRST] >= 20)
	    gain /= 2;

    }

    //gain = gain * ch->in_room->heal_rate / 100;

    //if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
    //gain = gain * ch->on->value[3] / 100;

    /*
     * if (IS_AFFECTED(ch, AFF_POISON)) gain /= 4;
     *
     * if (IS_AFFECTED(ch, AFF_PLAGUE)) gain /= 8;
     *
     * if (IS_AFFECTED(ch, AFF_HASTE) || IS_AFFECTED(ch, AFF_SLOW)) gain /=
     * 2;
     *
     * if (ch->race == race_lookup("vampire") && time_info.hour < 7 &&
     * time_info.hour > 19) gain *= 3;
     */
    return UMIN(gain, ch->maxHit - ch->hit);
}

long mana_gain(Character * ch)
{
    long gain;
    //int           number;

    if (ch->inRoom == NULL)
	return 0;

    if (!ch->pc) {
	gain = 10 + ch->level;
	switch (ch->position) {
	default:
	    gain /= 2;
	    break;
	case POS_SLEEPING:
	    gain = 3 * gain / 2;
	    break;
	case POS_RESTING:
	    break;
	case POS_FIGHTING:
	    gain /= 3;
	    break;
	}
    } else {
	gain =
	    (getCurrStat(ch, STAT_WIS) + getCurrStat(ch, STAT_INT) +
	     ch->level);
	//gain += bonus(ch) * 2;
	/*
	 * number = number_percent(); if (number <
	 * ch->get_skill(gsn_meditation)) { gain += number * gain /
	 * 100; if (ch->mana < ch->max_mana) check_improve(ch,
	 * gsn_meditation, true, 8); } if (!has_spells(ch)) gain /=
	 * 2;
	 */
	switch (ch->position) {
	default:
	    gain /= 4;
	    break;
	case POS_SLEEPING:
	    break;
	case POS_RESTING:
	    gain /= 2;
	    break;
	case POS_FIGHTING:
	    gain /= 6;
	    break;
	}

	if (ch->pc->condition[COND_HUNGER] >= 20)
	    gain /= 2;

	if (ch->pc->condition[COND_THIRST] >= 20)
	    gain /= 2;

    }

    /*
       gain = gain * ch->in_room->mana_rate / 100;

       if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
       gain = gain * ch->on->value[4] / 100;

       if (IS_AFFECTED(ch, AFF_POISON))
       gain /= 4;

       if (IS_AFFECTED(ch, AFF_PLAGUE))
       gain /= 8;

       if (IS_AFFECTED(ch, AFF_HASTE) || IS_AFFECTED(ch, AFF_SLOW))
       gain /= 2;

       if (ch->race == race_lookup("vampire") && time_info.hour < 7 && time_info.hour > 19)
       gain *= 3;
     */

    return UMIN(gain, ch->maxMana - ch->mana);
}

long move_gain(Character * ch)
{
    long gain;

    if (ch->inRoom == NULL)
	return 0;

    if (!ch->pc) {
	gain = ch->level;
    } else {
	gain = UMAX(15, ch->level + getCurrStat(ch, STAT_DEX));

	switch (ch->position) {
	case POS_SLEEPING:
	    gain += getCurrStat(ch, STAT_DEX);
	    break;
	case POS_RESTING:
	    gain += getCurrStat(ch, STAT_DEX) / 2;
	    break;
	default:
	    break;
	}

	if (ch->pc->condition[COND_HUNGER] >= 20)
	    gain /= 2;

	if (ch->pc->condition[COND_THIRST] >= 20)
	    gain /= 2;
    }

    /*
       gain = gain * ch->in_room->heal_rate / 100;

       if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
       gain = gain * ch->on->value[3] / 100;

       if (IS_AFFECTED(ch, AFF_POISON))
       gain /= 4;

       if (IS_AFFECTED(ch, AFF_PLAGUE))
       gain /= 8;

       if (IS_AFFECTED(ch, AFF_HASTE) || IS_AFFECTED(ch, AFF_SLOW))
       gain /= 2;

       if (ch->race == race_lookup("vampire") && time_info.hour < 7 && time_info.hour > 19)
       gain *= 3;
     */
    return UMIN(gain, ch->maxMove - ch->move);
}

void gain_condition(Character * ch, int iCond, int value)
{
    if (value == 0 || !ch || !ch->pc || ch->level >= LEVEL_IMMORTAL)
	return;

    int condition = ch->pc->condition[iCond];

    if (condition < -50)
	return;

    if (iCond == COND_THIRST && value < 0 && ch->inRoom
	&& ch->inRoom->sector == SECT_DESERT) {
	value *= 2;
	writeln(ch, "You feel the ~Ydesert ~Rheat~x...");
    }
    if (condition < 0)
	value /= 2;

    ch->pc->condition[iCond] =
	URANGE(iCond == COND_DRUNK ? 0 : -50, condition + value, 75);

    if ((condition = ch->pc->condition[iCond]) <= 0) {
	switch (iCond) {
	case COND_HUNGER:
	    if (condition < -42 && value < 0) {
		writeln(ch, "You are starving!");
		act(TO_ROOM, ch, 0, 0, "$n is starving!");
	    } else if (condition < -30)
		writeln(ch, "You are dying of starvation.");
	    else if (condition < -20)
		writeln(ch, "You are extremely hungry.");
	    else if (condition < -10)
		writeln(ch, "You are really hungry.");
	    else
		writeln(ch, "You are hungry.");
	    if (ch->position == POS_SLEEPING)
		return;

	    break;

	case COND_THIRST:
	    if (condition < -40 && value < 0) {
		writeln(ch, "You are dying of thirst!");
		act(TO_ROOM, ch, 0, 0, "$n is dying of thirst!");
	    } else if (condition < -30)
		writeln(ch, "You are dying of dehydration.");
	    else if (condition < -20)
		writeln(ch, "You are extremely dehydrated.");
	    else if (condition < -10)
		writeln(ch, "You are dehydrating.");
	    else
		writeln(ch, "You are thirsty.");
	    if (ch->position == POS_SLEEPING)
		return;
	    break;

	case COND_DRUNK:
	    if (condition != 0)
		writeln(ch, "You are sober.");
	    break;
	}
    }
    return;
}

void character_update()
{
    for (Character * ch = first_character; ch != 0; ch = ch->next) {
	if (ch->pc) {
	    Client *conn = (Client *) ch->pc->conn;

	    if (!is_set(ch->pc->account->flags, PLR_TICKS_OFF)
		&& conn->editing == 0 && client_is_playing(conn)) {
		char *const mesg[] = {
		    "TICK", "TOCK"
		};
		if (number_range(0, 100) < 50)
		    writelnf(ch, "~?[%s] %s!~x",
			     str_time(-1,
				      ch->pc->account->timezone,
				      "%I:%M:%S"),
			     mesg[number_range(0, 10) < 5 ? 0 : 1]);
		else
		    writelnf(ch, "~?%s!~x",
			     mesg[number_range(0, 10) < 5 ? 0 : 1]);
	    }
	}
	if (ch->position < POS_STUNNED) {

	    if (ch->hit < ch->maxHit)
		ch->hit += hit_gain(ch);
	    else
		ch->hit = ch->maxHit;

	    if (ch->mana < ch->maxMana)
		ch->mana += mana_gain(ch);
	    else
		ch->mana = ch->maxMana;

	    if (ch->move < ch->maxMove)
		ch->move += move_gain(ch);
	    else
		ch->move = ch->maxMove;
	}
	gain_condition(ch, COND_DRUNK, -1);
	gain_condition(ch, COND_THIRST, -1);
	gain_condition(ch, COND_HUNGER,	/* ch->size > SIZE_MEDIUM ?
					 * -2 : */
		       -1);

	for (Affect * paf_next, *paf = ch->affects; paf; paf = paf_next) {
	    paf_next = paf->next;

	    if (paf->duration > 0) {
		paf->duration--;
		if (number_range(0, 4) == 0 && paf->level > 0)
		    paf->level--;
	    } else if (paf->duration == 0) {
		if (!paf_next || paf_next->from != paf->from
		    || paf_next->duration > 0) {
		    if (valid_skill(paf->from)
			&& !nullstr(skill_table[paf->from].msgOff)) {
			writeln(ch, skill_table[paf->from].msgOff);
		    }
		}
		affect_remove(ch, paf);
	    }
	}
    }
}

void hint_update()
{
    for (Character * ch = first_player; ch != 0; ch = ch->next_player) {
	if (!is_set(ch->pc->account->flags, PLR_HINTS))
	    continue;

	long pos = number_range(0, max_hint - 1);

	writelnf(ch, "~?%s~x", hint_table[pos].text);
    }
}

void violence_update()
{
    for (Character * ch_next, *ch = first_character; ch != 0; ch = ch_next) {
	Character *victim;

	ch_next = ch->next;

	if ((victim = ch->fighting) == 0 || ch->inRoom == 0)
	    continue;

	if (victim->inRoom == ch->inRoom) {
	    multi_hit(ch, victim, GSN_UNDEFINED, DAM_UNDEFINED);
	} else {
	    ch->fighting = 0;
	}
    }
}

void aggr_update()
{

}

int pulse_tick;

void update_handler()
{
    static int pulse_area;
    static int pulse_hint;
    static int pulse_violence;

    if (--pulse_area <= 0) {
	pulse_area = PULSE_AREA;
	area_update();
    }
    if (--pulse_tick <= 0) {
	pulse_tick = PULSE_TICK;
	character_update();
    }
    if (--pulse_hint <= 0) {
	pulse_hint = PULSE_HINT;
	hint_update();
    }
    if (--pulse_violence <= 0) {
	pulse_violence = PULSE_VIOLENCE;
	violence_update();
    }
    aggr_update();
}
