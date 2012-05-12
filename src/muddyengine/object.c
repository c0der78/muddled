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
#include <muddyengine/db.h>
#include <muddyengine/log.h>
#include <muddyengine/flag.h>
#include <muddyengine/object.h>
#include <muddyengine/string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <muddyengine/area.h>
#include <muddyengine/engine.h>
#include <muddyengine/macro.h>
#include <muddyengine/character.h>
#include <muddyengine/util.h>
#include <muddyengine/buffer.h>
#include <muddyengine/class.h>
#include <muddyengine/player.h>
#include <muddyengine/room.h>

Object *first_object = 0;

static bool canUseFloat(Character * ch)
{
	return class_table[*ch->classes].fMana;
}

static bool canUseShield(Character * ch)
{

	return !class_table[*ch->classes].fMana;
}

const wear_table_type wear_table[] = {
	{WEAR_NONE, WEAR_NONE, "not worn", 0},
	{WEAR_HEAD, WEAR_HEAD, "worn on head", 0},
	{WEAR_NECK, WEAR_NECK, "worn around neck", 0},
	{WEAR_EYES, WEAR_EYES, "worn on eyes", 0},
	{WEAR_SHOULDERS, WEAR_SHOULDERS, "worn on shoulders", 0},
	{WEAR_BACK, WEAR_BACK, "worn on back", 0},
	{WEAR_ARMS, WEAR_ARMS, "worn on arms", 0},
	{WEAR_HANDS, WEAR_HANDS, "worn on hands", 0},
	{WEAR_WRIST, WEAR_WRIST, "worn on left wrist", 0},
	{WEAR_WRIST, WEAR_WRIST_2, "worn on right wrist", 0},
	{WEAR_FINGER, WEAR_FINGER, "worn on left finger", 0},
	{WEAR_FINGER, WEAR_FINGER_2, "worn on right finger", 0},
	{WEAR_TORSO, WEAR_TORSO, "worn on torso", 0},
	{WEAR_WAIST, WEAR_WAIST, "worn on waist", 0},
	{WEAR_LEGS, WEAR_LEGS, "worn on legs", 0},
	{WEAR_FEET, WEAR_FEET, "worn on feet", 0},
	{WEAR_ABOUT, WEAR_ABOUT, "worn about body", 0},
	{WEAR_WIELD, WEAR_WIELD, "wielded", 0},
	{WEAR_DUAL, WEAR_DUAL, "wielded", 0},
	{WEAR_SHIELD, WEAR_SHIELD, "worn as shield", canUseShield},
	{WEAR_LIGHT, WEAR_LIGHT, "used as light", 0},
	{WEAR_HOLD, WEAR_HOLD, "holding", 0},
	{WEAR_FLOAT, WEAR_FLOAT, "floating nearby", canUseFloat},
	{0, 0, 0}
};

const Lookup object_types[] = {
	{"light", OBJ_LIGHT},
	{"scroll", OBJ_SCROLL},
	{"wand", OBJ_WAND},
	{"staff", OBJ_STAFF},
	{"weapon", OBJ_WEAPON},
	{"treasure", OBJ_TREASURE},
	{"armor", OBJ_ARMOR},
	{"potion", OBJ_POTION},
	{"furniture", OBJ_FURNITURE},
	{"trash", OBJ_TRASH},
	{"container", OBJ_CONTAINER},
	{"drink_cont", OBJ_DRINK_CON},
	{"key", OBJ_KEY},
	{"food", OBJ_FOOD},
	{"money", OBJ_MONEY},
	{"boat", OBJ_BOAT},
	{"corpse_npc", OBJ_CORPSE_NPC},
	{"corpse_pc", OBJ_CORPSE_PC},
	{"fountain", OBJ_FOUNTAIN},
	{"pill", OBJ_PILL},
	{"portal", OBJ_PORTAL},
	{"warp_stone", OBJ_WARP_STONE},
	{"room_key", OBJ_ROOM_KEY},
	{"jewelry", OBJ_JEWELRY},
	{0, 0}
};

const Lookup wear_types[] = {
	{"none", WEAR_NONE},
	{"head", WEAR_HEAD},
	{"neck", WEAR_NECK},
	{"eyes", WEAR_EYES},
	{"shoulders", WEAR_SHOULDERS},
	{"back", WEAR_BACK},
	{"arms", WEAR_ARMS},
	{"hands", WEAR_HANDS},
	{"lwrist", WEAR_WRIST},
	{"rwrist", WEAR_WRIST_2},
	{"rfinger", WEAR_FINGER},
	{"lfinger", WEAR_FINGER_2},
	{"torso", WEAR_TORSO},
	{"waist", WEAR_WAIST},
	{"legs", WEAR_LEGS},
	{"feet", WEAR_FEET},
	{"about", WEAR_ABOUT},
	{"wield", WEAR_WIELD},
	{"dual", WEAR_DUAL},
	{"shield", WEAR_SHIELD},
	{"light", WEAR_LIGHT},
	{"hold", WEAR_HOLD},
	{"float", WEAR_FLOAT},
	{0, 0}
};

const Lookup wear_flags[] = {
	{"none", WEAR_NONE},
	{"head", WEAR_HEAD},
	{"neck", WEAR_NECK},
	{"eyes", WEAR_EYES},
	{"shoulders", WEAR_SHOULDERS},
	{"back", WEAR_BACK},
	{"arms", WEAR_ARMS},
	{"hands", WEAR_HANDS},
	{"wrist", WEAR_WRIST},
	{"finger", WEAR_FINGER},
	{"torso", WEAR_TORSO},
	{"waist", WEAR_WAIST},
	{"legs", WEAR_LEGS},
	{"feet", WEAR_FEET},
	{"about", WEAR_ABOUT},
	{"wield", WEAR_WIELD},
	{"dual", WEAR_DUAL},
	{"shield", WEAR_SHIELD},
	{"light", WEAR_LIGHT},
	{"hold", WEAR_HOLD},
	{"float", WEAR_FLOAT},
	{0, 0}
};

const Lookup weapon_types[] = {
	{"exotic", WEAPON_EXOTIC},
	{"sword", WEAPON_SWORD},
	{"dagger", WEAPON_DAGGER},
	{"spear", WEAPON_SPEAR},
	{"mace", WEAPON_MACE},
	{"axe", WEAPON_AXE},
	{"flail", WEAPON_FLAIL},
	{"whip", WEAPON_WHIP},
	{"polearm", WEAPON_POLEARM},
	{"longrange", WEAPON_LONGRANGE},
	{0, 0}
};

const Lookup weapon_flags[] = {

	{"flaming", WEAPON_FLAMING},
	{"freezing", WEAPON_FROST},
	{"vampiric", WEAPON_VAMPIRIC},
	{"sharp", WEAPON_SHARP},
	{"vorpal", WEAPON_VORPAL},
	{"two_handed", WEAPON_TWO_HANDS},
	{"shocking", WEAPON_SHOCKING},
	{"poison", WEAPON_POISON},
	{0, 0}
};

const Lookup container_flags[] = {
	{"closeable", CONT_CLOSEABLE},
	{"pickproof", CONT_PICKPROOF},
	{"closed", CONT_CLOSED},
	{"locked", CONT_LOCKED},
	{"put_on", CONT_PUT_ON},
	{0, 0}
};

const Lookup object_flags[] = {
	{"glow", ITEM_GLOW},
	{"hum", ITEM_HUM},
	{"dark", ITEM_DARK},
	{"evil", ITEM_EVIL},
	{"invis", ITEM_INVIS},
	{"magic", ITEM_MAGIC},
	{"nodrop", ITEM_NODROP},
	{"bless", ITEM_BLESS},
	{"anti-good", ITEM_ANTI_GOOD},
	{"anti-evil", ITEM_ANTI_EVIL},
	{"anti-neutral", ITEM_ANTI_NEUTRAL},
	{"noremove", ITEM_NOREMOVE},
	{"inventory", ITEM_INVENTORY},
	{"nopurge", ITEM_NOPURGE},
	{"rot_death", ITEM_ROT_DEATH},
	{"burnproof", ITEM_BURN_PROOF},
	{0, 0}
};

int wear_type_to_flag(int type)
{
	if (type == WEAR_WRIST_2)
		return WEAR_WRIST;

	if (type == WEAR_FINGER_2)
		return WEAR_FINGER;

	return type;
}

Object *new_object()
{
	Object *obj = (Object *) alloc_mem(1, sizeof(Object));

	obj->name = str_empty;
	obj->shortDescr = str_empty;
	obj->longDescr = str_empty;
	obj->description = str_empty;
	obj->condition = 100.0f;
	obj->weight = 0.0f;
	obj->cost = 0.0f;
	obj->wearLoc = WEAR_NONE;
	obj->wearFlags = WEAR_NONE;
	obj->flags = new_flag();

	return obj;
}

void destroy_object(Object * obj)
{
	free_str(obj->name);
	free_str(obj->shortDescr);
	free_str(obj->longDescr);
	free_str(obj->description);
	destroy_flags(obj->flags);

	free_mem(obj);
}

int load_obj_columns(Object * obj, sql_stmt * stmt)
{
	int count = sql_column_count(stmt);

	for (int i = 0; i < count; i++) {
		const char *colname = sql_column_name(stmt, i);

		if (!str_cmp(colname, "name")) {
			obj->name = str_dup(sql_column_str(stmt, i));
		} else if (!str_cmp(colname, "shortDescr")) {
			obj->shortDescr = str_dup(sql_column_str(stmt, i));
		} else if (!str_cmp(colname, "longDescr")) {
			obj->longDescr = str_dup(sql_column_str(stmt, i));
		} else if (!str_cmp(colname, "description")) {
			obj->description = str_dup(sql_column_str(stmt, i));
		} else if (!str_cmp(colname, "objectId")) {
			obj->id = sql_column_int(stmt, i);
		} else if (!str_cmp(colname, "level")) {
			obj->level = sql_column_int(stmt, i);
		} else if (!str_cmp(colname, "areaId")) {
			if (obj->area
			    && obj->area->id != sql_column_int64(stmt, i))
				log_error("sql returned invalid room for area");
			else
				obj->area =
				    get_area_by_id(sql_column_int(stmt, i));
		} else if (!str_cmp(colname, "type")) {
			obj->type = (object_type) sql_column_int(stmt, i);
		} else if (!str_cmp(colname, "inObjId")) {
			obj->inObj = get_obj_by_id(sql_column_int(stmt, i));
		} else if (!str_cmp(colname, "wearFlags")) {
			obj->wearFlags = (wear_type) sql_column_int(stmt, i);
		} else if (!str_cmp(colname, "weight")) {
			obj->weight = (float)sqlite3_column_double(stmt, i);
		} else if (!str_cmp(colname, "cost")) {
			obj->cost = sqlite3_column_double(stmt, i);
		} else if (!str_cmp(colname, "condition")) {
			obj->condition = (float)sqlite3_column_double(stmt, i);
		} else if (!str_cmp(colname, "carriedById")) {
			if (obj->carriedBy
			    && obj->carriedBy->id != sql_column_int(stmt, i))
				log_error
				    ("sql returned invalid character for object");
		} else {
			log_warn("unknown room column '%s'", colname);
		}
	}

	return count;
}

Object *load_object(identifier_t id)
{
	char buf[400];
	sql_stmt *stmt;
	Object *obj = 0;

	int len =
	    sprintf(buf, "select * from object where objectId=%" PRId64, id);

	if (sql_query(buf, len, &stmt) != SQL_OK) {
		log_data("could not prepare statement");
		return 0;
	}
	if (sql_step(stmt) != SQL_DONE) {

		obj = new_object();

		load_obj_columns(obj, stmt);

		LINK(first_object, obj, next);
		LINK(obj->area->objects, obj, next_in_area);
	}
	if (sql_finalize(stmt) != SQL_OK) {
		log_data("could not finalize statement");
	}
	return obj;
}

int load_objects(Area * area)
{
	char buf[400];
	sql_stmt *stmt;
	int total = 0;

	int len = sprintf(buf, "select * from object where areaId=%" PRId64,
			  area->id);

	if (sql_query(buf, len, &stmt) != SQL_OK) {
		log_data("could not prepare statement");
		return 0;
	}
	while (sql_step(stmt) != SQL_DONE) {

		Object *obj = new_object();

		obj->area = area;

		load_obj_columns(obj, stmt);

		LINK(area->objects, obj, next_in_area);
		LINK(first_object, obj, next);
		total++;
	}

	if (sql_finalize(stmt) != SQL_OK) {
		log_data("could not finalize statement");
	}
	return total;
}

int save_object(Object * obj)
{
	field_map obj_values[] = {
		{"name", &obj->name, SQL_TEXT, 0},
		{"description", &obj->description, SQL_TEXT, 0},
		{"shortDescr", &obj->shortDescr, SQL_TEXT, 0},
		{"longDescr", &obj->longDescr, SQL_TEXT, 0},
		{"weight", &obj->weight, SQL_FLOAT, 0},
		{"areaId", (obj->area ? &obj->area->id : 0), SQL_INT, 0},
		{"condition", &obj->condition, SQL_FLOAT, 0},
		{"wearFlags", &obj->wearFlags, SQL_INT, 0},
		{"level", &obj->level, SQL_INT, 0},
		{"carriedById", (obj->carriedBy ? &obj->carriedBy->id : 0),
		 SQL_INT, 0},
		{"inObjId", (obj->inObj ? &obj->inObj->id : 0), SQL_INT, 0},
		{"cost", &obj->cost, SQL_DOUBLE, 0},
		{0, 0, 0}
	};

	if (obj->id == 0) {
		if (sql_insert_query(obj_values, "object") != SQL_OK) {
			log_data("could not insert object");
			return 0;
		}
		obj->id = db_last_insert_rowid();
	} else {
		if (sql_update_query(obj_values, "object", obj->id) != SQL_OK) {
			log_data("could not update object");
			return 0;
		}
	}

	return 1;
}

int delete_object(Object * obj)
{
	char buf[BUF_SIZ];

	sprintf(buf, "delete from object where objectId=%" PRId64, obj->id);

	if (sql_exec(buf) != SQL_OK) {
		log_data("could not delete object");
		return 0;
	}
	return 1;
}

void extract_obj(Object * obj)
{
	if (obj->inRoom) {
		obj_from_room(obj);
	}
	if (obj->carriedBy) {
		obj_from_char(obj);
	}
	if (obj->inObj) {
		obj_from_obj(obj);
	}
	for (Object * obj_next, *obj_content = obj->contains; obj_content;
	     obj_content = obj_next) {
		obj_next = obj_content->next_content;
		extract_obj(obj_content);
	}

	UNLINK(first_object, Object, obj, next);

	destroy_object(obj);
}

Object *get_obj_by_id(identifier_t id)
{
	for (Object * obj = first_object; obj != 0; obj = obj->next) {
		if (obj->id == id)
			return obj;
	}
	return 0;
}

Object *object_lookup(const char *arg)
{
	if (is_number(arg)) {
		return get_obj_by_id(atoi(arg));
	} else {
		for (Object * obj = first_object; obj != 0; obj = obj->next) {
			if (is_name(arg, obj->name))
				return obj;
		}

		return 0;
	}
}

const char *format_obj_to_char(Object * obj, Character * ch, bool fShort)
{
	return can_see_obj(ch, obj) ? obj->shortDescr : "something";
}

void
show_list_to_char(Object * list, Character * ch, bool fShort, bool fShowNothing)
{
	Buffer *output;
	const char **prgpstrShow;
	int *prgnShow;
	const char *pstrShow;
	Object *obj;
	int nShow;
	int iShow;
	int count;
	bool fCombine;

	/*
	 * Alloc space for output lines.
	 */
	output = new_buf();

	count = 0;
	for (obj = list; obj != NULL; obj = obj->next_content)
		count++;
	prgpstrShow = alloc_mem(count, sizeof(char *));
	prgnShow = alloc_mem(count, sizeof(int));
	nShow = 0;

	/*
	 * Format the list of objects.
	 */
	for (obj = list; obj != NULL; obj = obj->next_content) {
		if (obj->wearLoc == WEAR_NONE && can_see_obj(ch, obj)) {
			pstrShow = format_obj_to_char(obj, ch, fShort);

			fCombine = false;

			/*
			 * Look for duplicates, case sensitive.
			 * Matches tend to be near end so run loop backwords.
			 */
			for (iShow = nShow - 1; iShow >= 0; iShow--) {
				if (!strcmp(prgpstrShow[iShow], pstrShow)) {
					prgnShow[iShow]++;
					fCombine = true;
					break;
				}
			}

			/*
			 * Couldn't combine, or didn't want to.
			 */
			if (!fCombine) {
				prgpstrShow[nShow] = str_dup(pstrShow);
				prgnShow[nShow] = 1;
				nShow++;
			}
		}
	}

	/*
	 * Output the formatted list.
	 */
	for (iShow = 0; iShow < nShow; iShow++) {
		if (prgpstrShow[iShow][0] == '\0') {
			free_str(prgpstrShow[iShow]);
			continue;
		}
		if (prgnShow[iShow] != 1) {
			writef(output, "(%2d) ", prgnShow[iShow]);
		} else {
			write(output, "     ");
		}

		writeln(output, prgpstrShow[iShow]);
		free_str(prgpstrShow[iShow]);
	}

	if (fShowNothing && nShow == 0) {
		writeln(ch, "     Nothing.");
	}
	ch->page(ch, buf_string(output));

	/*
	 * Clean up.
	 */
	destroy_buf(output);
	free_mem(prgpstrShow);
	free_mem(prgnShow);
}

bool can_wear(Object * obj, wear_type loc)
{

	return wear_table[loc].flags == obj->wearFlags;
}

Object *get_obj_carry(const Character * ch, const char *argument,
		      const Character * viewer)
{
	char arg[BUF_SIZ];
	Object *obj;
	long number;
	int count;

	number = number_argument(argument, arg);
	count = 0;
	for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->wearLoc == WEAR_NONE && (can_see_obj(viewer, obj))
		    && is_name(arg, obj->name)) {
			if (++count == number)
				return obj;
		}
	}

	return NULL;
}

Object *get_obj_world(const Character * ch, const char *argument)
{
	char arg[BUF_SIZ];
	long number;
	Object *obj;
	int count;

	if ((obj = get_obj_here(ch, argument)) != NULL)
		return obj;

	number = number_argument(argument, arg);
	count = 0;
	for (obj = first_object; obj != NULL; obj = obj->next) {
		if (can_see_obj(ch, obj) && is_name(arg, obj->name)) {
			if (++count == number)
				return obj;
		}
	}

	return NULL;
}

Object *get_obj_list(const Character * ch, const char *argument, Object * list)
{
	char arg[BUF_SIZ];
	Object *obj;
	long number;
	int count;

	number = number_argument(argument, arg);
	count = 0;
	for (obj = list; obj != NULL; obj = obj->next_content) {
		if (can_see_obj(ch, obj) && is_name(arg, obj->name)) {
			if (++count == number)
				return obj;
		}
	}

	return NULL;
}

/*
 * Find an obj in player's equipment.
 */
Object *get_obj_wear(const Character * ch, const char *argument)
{
	char arg[BUF_SIZ];
	Object *obj;
	long number;
	int count;

	number = number_argument(argument, arg);
	count = 0;
	for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->wearLoc != WEAR_NONE && can_see_obj(ch, obj)
		    && is_name(arg, obj->name)) {
			if (++count == number)
				return obj;
		}
	}

	return NULL;
}

Object *get_obj_here(const Character * ch, const char *argument)
{
	Object *obj;

	obj = get_obj_list(ch, argument, ch->inRoom->objects);
	if (obj != NULL)
		return obj;

	if ((obj = get_obj_carry(ch, argument, ch)) != NULL)
		return obj;

	if ((obj = get_obj_wear(ch, argument)) != NULL)
		return obj;

	return NULL;
}
