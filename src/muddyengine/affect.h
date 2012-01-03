
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

#ifndef __AFFECT_H_
#define __AFFECT_H_

typedef struct Affect Affect;

typedef enum
{
	TO_AFFECTS,
	TO_OBJECT,
	TO_RESIST,
	TO_WEAPON
} where_t;

typedef enum
{
	APPLY_NONE,
	APPLY_STR,
	APPLY_DEX,
	APPLY_INT,
	APPLY_WIS,
	APPLY_CON,
	APPLY_SEX,
	APPLY_LEVEL,
	APPLY_AGE,
	APPLY_WEIGHT,
	APPLY_MANA,
	APPLY_HIT,
	APPLY_MOVE,
	APPLY_GOLD,
	APPLY_EXP,
	APPLY_HITROLL,
	APPLY_DAMROLL,
	APPLY_RESIST,
	APPLY_LUCK,
	APPLY_SIZE,
	APPLY_ALIGN,
	APPLY_SKILL,
	APPLY_SPELL_AFFECT
} apply_t;

#include <muddyengine/character.h>
#include <muddyengine/engine.h>
#include <stdbool.h>

struct Affect
{
	Affect *next;
	identifier_t id;
	identifier_t type;
	int level;
	int duration;
	int dur_save;
	int modifier;
	Flag *flags;
	where_t where;
	apply_t location;
};

enum
{
	AFF_BLIND,
	AFF_INVISIBLE,
	AFF_DETECT_EVIL,
	AFF_DETECT_INVIS,
	AFF_DETECT_MAGIC,
	AFF_DETECT_HIDDEN,
	AFF_DETECT_GOOD,
	AFF_SANCTUARY,
	AFF_FAERIE_FIRE,
	AFF_INFRARED,
	AFF_CURSE,
	AFF_POISON,
	AFF_PROTECT_EVIL,
	AFF_PROTECT_GOOD,
	AFF_SNEAK,
	AFF_HIDE,
	AFF_SLEEP,
	AFF_CHARM,
	AFF_FLYING,
	AFF_PASS_DOOR,
	AFF_HASTE,
	AFF_CALM,
	AFF_PLAGUE,
	AFF_WEAKEN,
	AFF_DARK_VISION,
	AFF_BERSERK,
	AFF_REGENERATION,
	AFF_SLOW
};

Affect *new_affect(  );
void destroy_affect( Affect * );
void affect_to_char( Character *, Affect * );
void affect_modify( Character *, Affect *, bool );
void affect_check( Character *, where_t, Flag * );
Affect *load_affect_by_id( identifier_t );
int save_affect( Affect * );
bool is_affected( Character *, identifier_t );
void affect_remove( Character *, Affect * );
extern const Lookup apply_types[];
extern const Lookup where_types[];
extern const Lookup affect_flags[];
#endif
