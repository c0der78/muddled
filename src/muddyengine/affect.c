
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
#include <muddyengine/affect.h>
#include <muddyengine/engine.h>
#include <muddyengine/log.h>
#include <muddyengine/character.h>
#include <muddyengine/nonplayer.h>
#include <muddyengine/object.h>
#include <muddyengine/room.h>
#include <stdio.h>
#include <inttypes.h>
#include <muddyengine/db.h>
#include <muddyengine/string.h>
#include <muddyengine/lookup.h>
#include <assert.h>

const Lookup affect_flags[] = {
	{"blindness", AFF_BLIND},
	{"invisibility", AFF_INVISIBLE},
	{"detect_evil", AFF_DETECT_EVIL},
	{"detect_invis", AFF_DETECT_INVIS},
	{"detect_magic", AFF_DETECT_MAGIC},
	{"detect_hidden", AFF_DETECT_HIDDEN},
	{"detect_good", AFF_DETECT_GOOD},
	{"sactuary", AFF_SANCTUARY},
	{"faerie_fire", AFF_FAERIE_FIRE},
	{"infrared", AFF_INFRARED},
	{"curse", AFF_CURSE},
	{"poison", AFF_POISON},
	{"protect_evil", AFF_PROTECT_EVIL},
	{"protect_good", AFF_PROTECT_GOOD},
	{"sneak", AFF_SNEAK},
	{"hide", AFF_HIDE},
	{"sleep", AFF_SLEEP},
	{"charm", AFF_CHARM},
	{"flying", AFF_FLYING},
	{"pass_door", AFF_PASS_DOOR},
	{"haste", AFF_HASTE},
	{"calm", AFF_CALM},
	{"plague", AFF_PLAGUE},
	{"weaken", AFF_WEAKEN},
	{"dark_vision", AFF_DARK_VISION},
	{"berserk", AFF_BERSERK},
	{"regeneration", AFF_REGENERATION},
	{"slow", AFF_SLOW},
	{0, 0}
};

static int affect_mod(Affect * paf, bool fRemove)
{
	int mod = paf->modifier;

	if (fRemove)
		mod = 0 - mod;

	return mod;
}

void affect_apply_stat(Affect * aff, void *obj, bool fRemove, int stat)
{
	Character *ch = (Character *) obj;

	assert(ch != 0);

	ch->statMods[stat] += affect_mod(aff, fRemove);

}

void affect_apply_resist(Affect * paf, void *obj, bool fRemove, int arg)
{
	Character *ch = (Character *) obj;

	assert(ch != 0);

	ch->resists[arg] = affect_mod(paf, fRemove);
}

void affect_apply_resists(Affect * paf, void *obj, bool fRemove)
{
	Character *ch = (Character *) obj;
	assert(ch != 0);

	for (int i = DAM_BASH; i <= DAM_SLASH; i++) {
		ch->resists[i] += affect_mod(paf, fRemove);
	}
}

void affect_apply_str(Affect * paf, void *obj, bool fRemove)
{
	affect_apply_stat(paf, obj, fRemove, STAT_STR);
}

void affect_apply_dex(Affect * aff, void *obj, bool fRemove)
{
	affect_apply_stat(aff, obj, fRemove, STAT_DEX);
}

void affect_apply_int(Affect * paf, void *obj, bool fRemove)
{
	affect_apply_stat(paf, obj, fRemove, STAT_INT);
}

void affect_apply_wis(Affect * paf, void *obj, bool fRemove)
{
	affect_apply_stat(paf, obj, fRemove, STAT_WIS);
}

void affect_apply_con(Affect * paf, void *obj, bool fRemove)
{
	affect_apply_stat(paf, obj, fRemove, STAT_CON);
}

void affect_apply_luck(Affect * paf, void *obj, bool fRemove)
{
	affect_apply_stat(paf, obj, fRemove, STAT_LUCK);
}

void affect_apply_sex(Affect * paf, void *obj, bool fRemove)
{
	Character *ch = (Character *) obj;
	assert(ch != 0);
	int s = ch->sex;
	s += affect_mod(paf, fRemove);
	ch->sex = (sex_t) s;
}

void affect_apply_level(Affect * paf, void *affected, bool fRemove)
{
	Character *ch = (Character *) affected;
	ch->level += affect_mod(paf, fRemove);
}

void affect_apply_mana(Affect * paf, void *obj, bool fRemove)
{
	Character *ch = (Character *) obj;
	assert(ch != 0);
	ch->maxMana += affect_mod(paf, fRemove);
}

void affect_apply_move(Affect * paf, void *obj, bool fRemove)
{
	Character *ch = (Character *) obj;
	assert(ch != 0);
	ch->maxMove += affect_mod(paf, fRemove);
}

void affect_apply_hit(Affect * paf, void *obj, bool fRemove)
{
	Character *ch = (Character *) obj;
	assert(ch != 0);
	ch->maxHit += affect_mod(paf, fRemove);
}

void affect_apply_size(Affect * paf, void *obj, bool fRemove)
{
	Character *ch = (Character *) obj;
	assert(ch != 0);
	ch->size += URANGE(SIZE_TINY, affect_mod(paf, fRemove), SIZE_HUGE);
}

void affect_apply_align(Affect * paf, void *obj, bool fRemove)
{
	Character *ch = (Character *) obj;
	assert(ch != 0);

	ch->alignment =
	    URANGE(-MAX_ALIGN, ch->alignment + affect_mod(paf, fRemove),
		   MAX_ALIGN);
}

const Lookup affect_callbacks[] = {
	{"none", 0},
	{"strength", (uintptr_t) & affect_apply_str},
	{"dexterity", (uintptr_t) & affect_apply_dex},
	{"intelligence", (uintptr_t) & affect_apply_int},
	{"wisdom", (uintptr_t) & affect_apply_wis},
	{"constitution", (uintptr_t) & affect_apply_con},
	{"luck", (uintptr_t) & affect_apply_luck},
	{"sex", (uintptr_t) & affect_apply_sex},
	{"level", (uintptr_t) & affect_apply_level},
	{"mana", (uintptr_t) & affect_apply_mana},
	{"hit", (uintptr_t) & affect_apply_hit},
	{"move", (uintptr_t) & affect_apply_move},
	{"size", (uintptr_t) & affect_apply_size},
	{"align", (uintptr_t) & affect_apply_align},
	{0, 0}
};

Affect *new_affect()
{
	Affect *aff = (Affect *) alloc_mem(1, sizeof(Affect));

	aff->from = -1;

	return aff;
}

void destroy_affect(Affect * aff)
{
	free_mem(aff);
}

void affect_remove(Character * ch, Affect * paf)
{
	if (ch->affects == 0) {
		log_bug("no affects.");
		return;
	}
	affect_modify(ch, paf, false);

	UNLINK(ch->affects, Affect, paf, next);
}

void affect_remove_obj(Object * obj, Affect * paf)
{
	if (obj->affects == 0) {
		log_bug("no affects on object.");
		return;
	}
	if (obj->carriedBy != NULL && obj->wearLoc != WEAR_NONE)
		(*paf->callback) (paf, obj->carriedBy, true);

	(*paf->callback) (paf, obj, true);

	UNLINK(obj->affects, Affect, paf, next);
}

const char *affect_name(Affect * paf)
{
	return lookup_name(affect_callbacks, (uintptr_t) paf->callback);
}

void affect_modify(Character * ch, Affect * paf, bool fAdd)
{
	if (ch && paf && paf->callback) {

		(*paf->callback) (paf, ch, !fAdd);
	}
	/*
	 * if (ch->pc && (wield = get_eq_char(ch, WEAR_WIELD)) != NULL &&
	 * get_obj_weight(wield) > (str_app[get_curr_stat(ch,
	 * STAT_STR)].wield * 10) && !IS_WEAPON_STAT(wield, WEAPON_NODISARM))
	 * { static int depth;
	 *
	 * if (depth == 0) { depth++; act("You drop $p.", ch, wield, NULL,
	 * TO_CHAR); act("$n drops $p.", ch, wield, NULL, TO_ROOM);
	 * obj_from_char(wield); obj_to_room(wield, ch->in_room); depth--; }
	 * }
	 */

	return;
}

Affect *affect_find(Affect * paf, identifier_t sn)
{

	for (Affect * paf_find = paf; paf_find; paf_find = paf_find->next) {
		if (paf_find->from == sn)
			return paf_find;
	}

	return 0;
}

void affect_to_char(Character * ch, Affect * paf)
{
	LINK(ch->affects, paf, next);

	affect_modify(ch, paf, true);

	return;
}

void affect_to_obj(Object * obj, Affect * paf)
{
	LINK(obj->affects, paf, next);

	if (obj->carriedBy && obj->wearLoc != WEAR_NONE)
		affect_modify(obj->carriedBy, paf, true);
}

bool is_affected(Character * ch, identifier_t sn)
{
	if (!ch)
		return false;

	return affect_find(ch->affects, sn) != NULL;
}

Affect *load_affect_by_id(identifier_t id)
{
	char buf[400];
	sql_stmt *stmt;

	int len =
	    sprintf(buf, "select * from affect where affectId=%" PRId64, id);

	if (sql_query(buf, len, &stmt) != SQL_OK) {
		log_data("could not prepare sql statement");
		return 0;
	}
	Affect *paf = 0;

	if (sql_step(stmt) != SQL_DONE) {
		paf = new_affect();
		paf->id = id;

		int count = sql_column_count(stmt);

		for (int i = 0; i < count; i++) {
			const char *colname = sql_column_name(stmt, i);

			if (!str_cmp(colname, "affectId")) {
				if (id != sql_column_int(stmt, i))
					log_error
					    ("sql statement did not return correct affect");
			} else if (!str_cmp(colname, "from")) {
				paf->from = sql_column_int(stmt, i);
			} else if (!str_cmp(colname, "level")) {
				paf->level = sql_column_int(stmt, i);
			} else if (!str_cmp(colname, "duration")) {
				paf->duration = paf->perm_duration =
				    sql_column_int(stmt, i);
			} else if (!str_cmp(colname, "modifier")) {
				paf->modifier = sql_column_int(stmt, i);
			} else if (!str_cmp(colname, "flags")) {
				parse_flags(paf->flags,
					    sql_column_str(stmt, i),
					    affect_flags);
			} else if (!str_cmp(colname, "type")) {

				paf->callback =
				    (affect_callback *)
				    value_lookup(affect_callbacks,
						 sql_column_str(stmt, i));

			} else {
				log_warn("unknown coloumn for affect '%s'",
					 colname);
			}
		}
	}
	if (sql_finalize(stmt) != SQL_OK) {
		log_data("unable to finalize statement");
	}
	return paf;
}

int save_affect(Affect * paf)
{
	field_map affect_values[] = {
		{"duration", &paf->duration, SQL_INT},
		{"perm_duration", &paf->perm_duration, SQL_INT},
		{"modifier", &paf->modifier, SQL_INT},
		{"level", &paf->level, SQL_INT},
		{"flags", &paf->flags, SQL_FLAG, affect_flags},
		{"type", &paf->callback, SQL_FLAG, affect_callbacks},
		{0}
	};

	if (paf->id == 0) {
		if (sql_insert_query(affect_values, "affect") != SQL_OK) {
			log_data("could not insert affect");
			return 0;
		}
		paf->id = db_last_insert_rowid();

	} else {
		if (sql_update_query(affect_values, "affect", paf->id) !=
		    SQL_OK) {
			log_data("could not update affect");
			return 0;
		}
	}

	return 1;
}
