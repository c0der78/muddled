
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

#ifndef NONPLAYER_H
#define NONPLAYER_H


typedef struct NPC NPC;


#include <muddyengine/character.h>
#include <muddyengine/area.h>

#define DICE_NUMBER			0
#define DICE_TYPE			1
#define DICE_BONUS			2
#define MAX_DICE			3

struct NPC {
    const char *shortDescr;
    const char *longDescr;
    position_t startPosition;
    Area *area;
    NPC *next_in_area;
    int damage[MAX_DICE];
    int mana[MAX_DICE];
    int hit[MAX_DICE];
};

enum {
    NPC_SENTINEL,
    NPC_SCAVENGER,
    NPC_AGGRESSIVE,
    NPC_WIMPY,
    NPC_STAY_AREA
};

NPC *new_npc();
void destroy_npc(NPC *);

extern const Lookup npc_flags[];
Character *load_npc(identifier_t);
int load_npcs(Area *);
int save_npc(Character *);
Character *get_npc_by_id(identifier_t);
Character *npc_lookup(const char *);
int delete_npc(Character *);

#endif				/* //  #ifndef NONPLAYER_H */
