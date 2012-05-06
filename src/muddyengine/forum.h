
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

#ifndef FORUM_H
#define FORUM_H

typedef struct Forum Forum;

typedef struct Note Note;

#include <muddyengine/character.h>
#include <muddyengine/engine.h>
#include <time.h>

typedef enum
{
	DEF_NORMAL,
	DEF_INCLUDE,
	DEF_EXCLUDE,
	DEF_READONLY
} forum_t;

struct Forum
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

struct Note
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

extern Forum *forum_table;
extern int max_forum;

enum
{
	FORUM_CHANGED = ( 1 << 0 ),
	FORUM_NOUNSUB = ( 1 << 1 )
};

#define FORUM_ERROR -1

Note *new_note(  );
void destroy_note( Note * );
int lookup_forum_by_name( const char * );
int lookup_forum_by_id(identifier_t);
void finish_note( Forum *, Note * );
bool is_note_to( Character *, Note * );
void show_note_to_char( Character *, Note *, int );
void next_forum( Character * );
Note *find_note( Character *, Forum *, int );
int unread_notes( Character *, Forum * );
void check_notes( Forum * );
bool is_subscribed( Account *, int );
void set_subscribed( Account *, int );
void remove_subscribed( Account *, int );
void note_check( Character * );
void show_forum( Character *, bool );
void save_forums(  );
int load_forums(  );
Note *last_note( Character *, Forum * );
int delete_note( Forum *, Note * );

#endif							//  #ifndef FORUM_H
