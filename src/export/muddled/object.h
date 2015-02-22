
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

#ifndef OBJECT_H
#define OBJECT_H

typedef struct object Object;

#include <muddled/variant.h>
#include <muddled/hashmap.h>
#include <muddled/db.h>
#include <muddled/affect.h>

#define MAX_OBJ_VAL	4

typedef enum
{
    WEAPON_EXOTIC,
    WEAPON_SWORD,
    WEAPON_DAGGER,
    WEAPON_SPEAR,
    WEAPON_MACE,
    WEAPON_AXE,
    WEAPON_FLAIL,
    WEAPON_WHIP,
    WEAPON_POLEARM,
    WEAPON_LONGRANGE
} weapon_type;

typedef enum
{
    OBJ_LIGHT,
    OBJ_SCROLL,
    OBJ_WAND,
    OBJ_STAFF,
    OBJ_WEAPON,
    OBJ_TREASURE,
    OBJ_ARMOR,
    OBJ_POTION,
    OBJ_FURNITURE,
    OBJ_TRASH,
    OBJ_CONTAINER,
    OBJ_DRINK_CON,
    OBJ_KEY,
    OBJ_FOOD,
    OBJ_MONEY,
    OBJ_BOAT,
    OBJ_CORPSE_NPC,
    OBJ_CORPSE_PC,
    OBJ_FOUNTAIN,
    OBJ_PILL,
    OBJ_PORTAL,
    OBJ_WARP_STONE,
    OBJ_ROOM_KEY,
    OBJ_JEWELRY
} object_type;

typedef enum
{
    WEAR_NONE,
    WEAR_HEAD,
    WEAR_NECK,
    WEAR_EYES,
    WEAR_SHOULDERS,
    WEAR_BACK,
    WEAR_ARMS,
    WEAR_HANDS,
    WEAR_WRIST,
    WEAR_WRIST_2,
    WEAR_FINGER,
    WEAR_FINGER_2,
    WEAR_TORSO,
    WEAR_WAIST,
    WEAR_LEGS,
    WEAR_FEET,
    WEAR_ABOUT,
    WEAR_WIELD,
    WEAR_DUAL,
    WEAR_SHIELD,
    WEAR_LIGHT,
    WEAR_HOLD,
    WEAR_FLOAT,
    MAX_WEAR
} wear_type;

struct object
{
    Object *next;
    Object *next_in_area;
    Object *next_content;
    Object *contains;
    Object *inObj;
    Affect *affects;
    hashmap extraDescr;
    Character *carriedBy;
    Room *inRoom;
    identifier_t id;
    Area *area;
    const char *name;
    const char *shortDescr;
    const char *longDescr;
    const char *description;
    float weight;
    float condition;
    money_t cost;
    wear_type wearLoc;
    wear_type wearFlags;
    Flag *flags;
    object_type type;
    short level;
    variant_t value[MAX_OBJ_VAL];
};

extern Object *first_object;
Object *new_object();
void destroy_object(Object *);

Object *load_object(identifier_t);
int load_obj_columns(Object *, sql_stmt *);
int load_objects(Area *);
int save_object(Object *);
int delete_object(Object *);
void extract_obj(Object *);
Object *get_obj_by_id(identifier_t);
Object *object_lookup(const char *);
Object *get_obj_world(const Character *, const char *);
Object *get_obj_carry(const Character *, const char *, const Character *);
Object *get_obj_wear(const Character *, const char *);
Object *get_obj_here(const Character *, const char *);

Object *get_obj_list(const Character *, const char *, Object *);

const char *format_obj_to_char(Object *, Character *, bool);
void show_list_to_char(Object *, Character *, bool, bool);

int wear_type_to_flag(int);

bool can_wear(Object *, wear_type);

enum
{
    WEAPON_FLAMING,
    WEAPON_FROST,
    WEAPON_VAMPIRIC,
    WEAPON_SHARP,
    WEAPON_VORPAL,
    WEAPON_TWO_HANDS,
    WEAPON_SHOCKING,
    WEAPON_POISON
};

enum
{
    CONT_CLOSEABLE,
    CONT_PICKPROOF,
    CONT_CLOSED,
    CONT_LOCKED,
    CONT_PUT_ON
};

enum
{
    ITEM_GLOW,
    ITEM_HUM,
    ITEM_DARK,
    ITEM_EVIL,
    ITEM_INVIS,
    ITEM_MAGIC,
    ITEM_NODROP,
    ITEM_BLESS,
    ITEM_ANTI_GOOD,
    ITEM_ANTI_EVIL,
    ITEM_ANTI_NEUTRAL,
    ITEM_NOREMOVE,
    ITEM_INVENTORY,
    ITEM_NOPURGE,
    ITEM_ROT_DEATH,
    ITEM_MELT_DROP,
    ITEM_BURN_PROOF
};
typedef struct
{
    wear_type loc;
    wear_type flags;
    const char *display;
    bool(*canUse) (Character *);
} wear_table_type;

extern const wear_table_type wear_table[];
extern const wear_table_type custom_wear_table[];
extern const Lookup wear_types[];
extern const Lookup wear_flags[];
extern const Lookup object_types[];
extern const Lookup object_flags[];
extern const Lookup weapon_types[];
extern const Lookup weapon_flags[];
extern const Lookup container_flags[];
#endif				/* // #ifndef OBJECT_H */
