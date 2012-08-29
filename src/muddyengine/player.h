
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

#ifndef PLAYER_H
#define PLAYER_H

typedef struct Player Player;

#include <muddyengine/account.h>
#include <muddyengine/character.h>
#include <muddyengine/connection.h>
#include <time.h>

struct Player {
	Account *account;
	Connection *conn;
	const char *title;
	Flag *explored;
	Flag *channels;
	const char *prompt;
	const char *battlePrompt;
	int condition[MAX_COND];
	long experience;
	long permHit;
	long permMana;
	long permMove;
	time_t created;
};

enum {
	PLR_COLOR_OFF,
	PLR_TICKS_OFF,
	PLR_BRIEF,
	PLR_HINTS,
	PLR_AUTOMAP_OFF,
	PLR_NOEMOTEVERBS
};

extern Character *first_player;
Player *new_player(Connection *);
void destroy_player(Player *);
extern const Lookup plr_flags[];
Character *load_player_by_id(Connection *, identifier_t);
Character *player_lookup(const char *);
Character *load_player_by_name(Connection *, const char *);
int save_player(Character *);
int delete_player(Character *);
bool is_drunk(Character *);
long exp_to_level(const Character *);

#endif				/* // #ifndef PLAYER_H */
