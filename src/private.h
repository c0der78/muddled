#ifndef MUDDLED_PRIVATE_H
#define MUDDLED_PRIVATE_H

#include <time.h>
#include "typedef.h"

struct account
{
    identifier_t id;
    const char *login;
    const char *email;
    const char *password;
    AccountPlayer *players;
    int timezone;
    identifier_t autologinId;
    Flag *flags;
    Forum *forum;
    Note *inProgress;
    Connection *conn;
    Character *playing;
    AccountForum *forumData;
};

struct account_player
{
    AccountPlayer *next;
    identifier_t charId;
    const char *name;
    short level;
};

struct account_forum
{
    identifier_t forumId;
    identifier_t id;
    time_t lastNote;
    bool unsubscribed;
    const char *draft;
};

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
    AffectCallback *callback;
};

struct area
{
    identifier_t id;
    Area *next;
    const char *name;
    Character *npcs;
    Room *rooms;
    Object *objects;
    Flag *flags;
};

struct buffer
{

    short state;        /* error state of the buffer */

    int size;       /* size in k */

    char *string;       /* buffer's string */

    bool(*write) (Buffer *, const char *);

    bool(*writeln) (Buffer *, const char *);

    bool(*writef) (Buffer *, const char *, ...);

    bool(*writelnf) (Buffer *, const char *, ...);

};

struct character
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
    void (*writeln) (const Character *, const char *);
    void (*writelnf) (const Character *, const char *, ...);
    void (*write) (const Character *, const char *);
    void (*writef) (const Character *, const char *, ...);
    void (*page) (Character *, const char *);
    void (*titlef) (const Character *, const char *, ...);
    void (*title) (const Character *, const char *);
};


struct forum
{
    identifier_t id;
    const char *name;
    const char *description;
    short readLevel;
    short writeLevel;
    const char *defaultTo;
    forum_t type;
    int purgeDays;
    Note *notes;
    int flags;
};

struct note
{
    Note *next;
    identifier_t id;
    const char *from;
    time_t date;
    const char *toList;
    const char *subject;
    const char *text;
    time_t expire;
};

#endif

