
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
#ifndef SKILL_H
#define SKILL_H

typedef struct Skill Skill;

#include <muddyengine/character.h>

typedef void SpellFun(int, Character *);

#define SPELL(spell) void spell_##spell(int sn, Character *ch)

#include <muddyengine/flag.h>


struct Skill {
    identifier_t id;
    const char *name;
    int *levels;
    money_t cost;
    position_t minPos;
    SpellFun *spellfun;
    int *pgsn;
    int mana;
    int wait;
    const char *damage;
    const char *msgOff;
    const char *msgObj;
    Flag flags;
};

struct gsn_type {
    int *pgsn;
    const char *name;
};

struct spellfun_type {
    SpellFun *fun;
    const char *name;
};

Skill *new_skill();
void destroy_skill(Skill *);

extern const struct gsn_type gsn_table[];
extern const struct spellfun_type spellfun_table[];
extern Skill *skill_table;
extern int max_skill;
SpellFun *spellfun_lookup(const char *);
const char *spellfun_name(SpellFun *);
int *gsn_lookup(const char *);
const char *gsn_name(int *);
Skill *skill_lookup(const char *);
bool valid_skill(identifier_t id);
int load_skills();
int save_skill(Skill *);

extern const Lookup skill_flags[];
#define GSN_UNDEFINED -1

#endif				/* //  #ifndef SKILL_H */
