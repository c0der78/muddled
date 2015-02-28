#ifndef MUDDLED_TYPEDEF_H
#define MUDDLED_TYPEDEF_H

/*!
 * @typedef Account
 */
typedef struct account Account;
/*!
 * @typedef AccountPlayer
 */
typedef struct account_player AccountPlayer;
/*!
 * @typedef AccountForum
 */
typedef struct account_forum AccountForum;


/*!
 * @typedef Area
 */
typedef struct area Area;

/*!
 * @typedef Affect
 */
typedef struct affect Affect;

typedef void AffectCallback(Affect *, void *, bool);


typedef struct buffer Buffer;


typedef struct connection Connection;

typedef struct character Character;

typedef struct forum Forum;

typedef struct note Note;

typedef struct engine Engine;

typedef double money_t;

typedef __int64_t identifier_t;

typedef struct room Room;

typedef struct object Object;

typedef struct npc NPC;

typedef struct player Player;

typedef struct race Race;

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

typedef enum
{
    DEF_NORMAL,
    DEF_INCLUDE,
    DEF_EXCLUDE,
    DEF_READONLY
} forum_t;

#endif
