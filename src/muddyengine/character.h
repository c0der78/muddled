
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


#ifndef CHARACTER_H
#define CHARACTER_H

typedef struct Character Character;

typedef enum
{
	SIZE_TINY = 2,
	SIZE_SMALL = 4,
	SIZE_AVERAGE = 6,
	SIZE_LARGE = 8,
	SIZE_HUGE = 10
} size_type;

typedef enum
{
	POS_FIGHTING,
	POS_STANDING,
	POS_SITTING,
	POS_RESTING,
	POS_STUNNED,
	POS_SLEEPING,
	POS_INCAPICATED,
	POS_MORTAL,
	POS_DEAD
} position_t;

typedef enum
{
	SEX_NEUTRAL,
	SEX_MALE,
	SEX_FEMALE,
	SEX_HERMAPHRODITE
} sex_t;

typedef enum
{
	COND_HUNGER,
	COND_THIRST,
	COND_DRUNK,
	COND_SANITY,
	MAX_COND
} condition_t;

#include <muddyengine/flag.h>
#include <muddyengine/affect.h>
#include <muddyengine/player.h>
#include <muddyengine/nonplayer.h>
#include <muddyengine/race.h>
#include <muddyengine/fight.h>
#include <muddyengine/room.h>

struct Character
{
	identifier_t id;
	Character *next;
	Character *next_in_area;
	Character *next_in_room;
	Character *next_player;
	sex_t sex;
	const char *name;
	Affect *affects;
	Player *pc;
	NPC *npc;
	Race *race;
	int *classes;
	short level;
	long hit;
	short version;
	long maxHit;
	long mana;
	long maxMana;
	long move;
	long maxMove;
	int stats[MAX_STAT];
	int statMods[MAX_STAT];
	int alignment;
	money_t gold;
	float size;
	int resists[MAX_DAM];
	const char *description;
	Character *fighting;
	position_t position;
	Object *carrying;
	Room *inRoom;
	Flag *flags;
	Flag *affectedBy;
	void ( *writeln ) ( const Character *, const char * );
	void ( *writelnf ) ( const Character *, const char *, ... );
	void ( *write ) ( const Character *, const char * );
	void ( *writef ) ( const Character *, const char *, ... );
	void ( *page ) ( Character *, const char * );
	void ( *titlef ) ( const Character *, const char *, ... );
	void ( *title ) ( const Character *, const char * );
};

extern const Lookup sex_table[];

extern const Lookup position_table[];
extern Character *first_character;
Character *new_char(  );
void destroy_char( Character * );
short scrwidth( const Character * );
short scrheight( const Character * );
int load_char_column( Character *, db_stmt *, const char *, int );
int save_character( Character *, const Lookup * );
int delete_character( Character * );
int load_char_objs( Character * );
int save_char_objs( Character * );
bool is_immortal( const Character * );
bool is_implementor( const Character * );
void extract_char( Character *, bool );
int load_char_affects( Character * );
int save_char_affects( Character * );
bool is_playing( const Character * );
bool is_player( const Character * );
Character *get_char_world( Character *, const char * );

#define MAX_ALIGN	2500

#endif							//  #ifndef CHARACTER_H
