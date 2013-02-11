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
/*!
 * @header Structure, methods and constants related to accounts @ignore
 * __AFFECT_H_
 */
#ifndef __AFFECT_H_
#define __AFFECT_H_

/*!
 * @typedef Affect
 */
typedef struct affect Affect;

#include <muddyengine/character.h>
#include <muddyengine/engine.h>
#include <stdbool.h>

/*!
 * @typedef affect_callback @param affect the affect being called from
 * @param affected the object being affected @param remove true if the affect
 * should be removed @param arg an extra argument if needed
 */
typedef void affect_callback(Affect *, void *, bool);

/*!
 * @typedef Affect @abstract Represents an affect that can be applied to
 * things in the world @field type a skill id if the affect comes from a
 * skill @field level the level of the affect @field duration the current
 * duration of the affect @field perm_duration the duration when the affect
 * was first created @field modifier the modifier value of the affect @field
 * callback the callback method when the affect is applied
 */
struct affect
{
    Affect *next;
    identifier_t id;
    int from;
    int level;
    int duration;
    int perm_duration;
    int modifier;
    Flag *flags;
    affect_callback *callback;
};

/*!
 * @enum Affect Flags @abstract @constant AFF_BLIND causes blindness
 * @constant AFF_INVISIBLE makes affected invisible @constant AFF_DETECT_EVIL
 * makes affected able to recognize evil @constant AFF_DETECT_INVIS makes
 * affected able to see invisible @constant AFF_DETECT_MAGIC makes affected
 * able to detect magic @constant AFF_DETECT_HIDDEN makes affected able to
 * see hidden things @constant AFF_DETECT_GOOD makes affected able to
 * recognize good @constant AFF_SANCTUARY applies a protective barrier to the
 * affected @constant AFF_FAERIE_FIRE causes weakness aura around affected
 * @constant AFF_INFRARED allows affected to see in the dark @constant
 * AFF_CURSE affected is cursed @constant AFF_POISION affected is poisoned
 * @constant AFF_PROTECT_EVIL affected is protected from evil @constant
 * AFF_PROTECT_GOOD affected is protected from good @constant AFF_SNEAK
 * affected is sneaking @constant AFF_HIDE affected is hiding @constant
 * AFF_SLEEP affected is sleeping @constant AFF_CHARM affected is charmed
 * @constant AFF_FLYING affected is flying @constant AFF_PASS_DOOR affected
 * is able to pass through doors @constant AFF_HASTE affected is moving
 * hastily @constant AFF_CALM affected is calmed @constant AFF_PLAGUE
 * affected by the plague @constant AFF_WEAKEN affected is weakened @constant
 * AFF_DARK_VISION affected can see in the dark @constant AFF_BERSERK
 * affected by rage @constant AFF_REGENERATION affected heals more quickly
 * @constant AFF_SLOW affected is moving slowly @discussion
 */
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

/*!
 * @group Memory Management
 */

/*!
 * creates a new affect @return the newly allocated affect
 */
Affect *new_affect();

/*!
 * destroys an affect @param affect the allocated affect to destroy
 */
void destroy_affect(Affect *);

/*!
 * @group Memory Management
 */

/*!
 * @group Data Management
 */

/*!
 * allocates and loads an affect by id @param id the affect id to load
 * @return the new allocated affect loaded from the id
 */
Affect *load_affect_by_id(identifier_t);

/*!
 * saves an affect to records @param affect the affect to save @return 1 if
 * successful, zero otherwise
 */
int save_affect(Affect *);

/*!
 * @group Data Management
 */

void affect_to_char(Character *, Affect *);
void affect_modify(Character *, Affect *, bool);
void affect_remove(Character *, Affect *);
const char *affect_name(Affect *);
bool is_affected(Character *, identifier_t);

/*!
 * @constant affect_callbacks the table used to lookup affect callbacks
 */
extern const Lookup affect_callbacks[];
/*!
 * @constant affect_flags flags used for affects
 */
extern const Lookup affect_flags[];

/*!
 * @group Callbacks
 */
affect_callback affect_apply_resists;
affect_callback affect_apply_str;
affect_callback affect_apply_int;
affect_callback affect_apply_wis;
affect_callback affect_apply_dex;
affect_callback affect_apply_con;
affect_callback affect_apply_luck;
affect_callback affect_apply_sex;
affect_callback affect_apply_level;
affect_callback affect_apply_mana;
affect_callback affect_apply_move;
affect_callback affect_apply_hit;
affect_callback affect_apply_size;
affect_callback affect_apply_align;
/*!
 * @group Callbacks
 */

#endif
