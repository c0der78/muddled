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
/*!
 * @header Structure, methods and constants related to accounts @ignore
 * __AFFECT_H_
 */
#ifndef MUDDLED_AFFECT_H
#define MUDDLED_AFFECT_H

#include "typedef.h"

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

BEGIN_DECL

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

END_DECL

/*!
 * @constant AffectCallbacks the table used to lookup affect callbacks
 */
extern const Lookup affect_callbacks[];
/*!
 * @constant affect_flags flags used for affects
 */
extern const Lookup affect_flags[];

/*!
 * @group Callbacks
 */
AffectCallback affect_apply_resists;
AffectCallback affect_apply_str;
AffectCallback affect_apply_int;
AffectCallback affect_apply_wis;
AffectCallback affect_apply_dex;
AffectCallback affect_apply_con;
AffectCallback affect_apply_luck;
AffectCallback affect_apply_sex;
AffectCallback affect_apply_level;
AffectCallback affect_apply_mana;
AffectCallback affect_apply_move;
AffectCallback affect_apply_hit;
AffectCallback affect_apply_size;
AffectCallback affect_apply_align;
/*!
 * @group Callbacks
 */

#endif
