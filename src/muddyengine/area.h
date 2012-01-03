
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

#ifndef __AREA_H_
#define __AREA_H_

typedef struct Area Area;

#include <muddyengine/character.h>
#include <muddyengine/room.h>
#include <muddyengine/flag.h>
#include <muddyengine/object.h>

struct Area
{
	identifier_t id;
	Area *next;
	const char *name;
	Character *npcs;
	Room *rooms;
	Object *objects;
	Flag *flags;
};

enum
{
	AREA_NOEXPLORE,
	AREA_CHANGED
};

Area *new_area(  );
void destroy_area( Area * );

int load_areas(  );
Area *get_area_by_id( identifier_t );
Area *load_area( identifier_t );
int save_area( Area * );
Area *area_lookup( const char * );
int save_area_only( Area * );
Area *get_default_area(  );
extern Area *first_area;
extern int max_area;

extern const Lookup area_flags[];
#endif							//  #ifndef AREA_H
