
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

#ifndef MUDDLED_FIGHT_H
#define MUDDLED_FIGHT_H

#include "cdecl.h"

typedef enum
{
    DAM_UNDEFINED = -1,
    DAM_BASH,
    DAM_PIERCE,
    DAM_SLASH,
    DAM_ACID,
    DAM_COLD,
    DAM_AIR,
    DAM_DISEASE,
    DAM_EARTH,
    DAM_ENERGY,
    DAM_FIRE,
    DAM_HOLY,
    DAM_LIGHT,
    DAM_SHADOW,
    DAM_MENTAL,
    DAM_ELECTRIC,
    DAM_WATER,
    DAM_POISON,
    DAM_SONIC,
    MAX_DAM
} dam_t;

BEGIN_DECL

void multi_hit(Character *, Character *, int, dam_t);

bool damage(Character *, Character *, long, int, dam_t);

END_DECL

extern const Lookup dam_types[];

#endif              /* // #ifndef FIGHT_H */
