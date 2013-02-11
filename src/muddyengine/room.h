
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

#ifndef ROOM_H
#define ROOM_H

typedef struct room Room;

#include <muddyengine/character.h>
#include <muddyengine/area.h>
#include <muddyengine/engine.h>
#include <muddyengine/hashmap.h>
#include <muddyengine/exit.h>
#include <muddyengine/object.h>

typedef enum
{
    SECT_INSIDE,
    SECT_CITY,
    SECT_FIELD,
    SECT_FOREST,
    SECT_HILLS,
    SECT_MOUNTAIN,
    SECT_WATER_SWIM,
    SECT_WATER_NOSWIM,
    SECT_AIR,
    SECT_DESERT,
    SECT_JUNGLE,
    SECT_ROAD,
    SECT_PATH,
    SECT_CAVE,
    SECT_SWAMP,
    SECT_UNKNOWN,
    SECT_MAX
} sector_t;

struct room
{
    Room *next;
    Room *next_in_area;
    identifier_t id;
    const char *name;
    const char *description;
    Character *characters;
    hashmap extraDescr;
    Area *area;
    Exit *exits[MAX_DIR];
    const char *reset;
    Object *objects;
    sector_t sector;
    Flag *flags;
};

enum
{
    ROOM_SAFE,
    ROOM_NO_RECALL,

    ROOM_NOEXPLORE
};

extern Room *room_hash[ID_HASH];

extern int max_room;
extern int max_explorable_room;

extern const Lookup sector_table[];
extern const Lookup room_flags[];
extern const Lookup room_flags[];

#define DEFAULT_ROOM 1

Room *new_room();
void destroy_room(Room *);
int load_rooms(Area *);
Room *load_room(identifier_t);
int save_room(Room *);
int save_room_only(Room *);
Room *room_lookup(const char *);
Room *get_room_by_id(identifier_t);
Room *get_default_room();

#endif				/* // #ifndef ROOM_H */
