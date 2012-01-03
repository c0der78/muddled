
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


#ifndef __ACCOUNT_H_
#define __ACCOUNT_H_

typedef struct Account Account;
typedef struct AccountPlayer AccountPlayer;

#include <muddyengine/flag.h>
#include <muddyengine/forum.h>
#include <muddyengine/connection.h>
#include <time.h>

struct Account
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
	int unsubscribed;
	time_t *lastNote;
	Connection *conn;
	Character *playing;
};

struct AccountPlayer
{
	AccountPlayer *next;
	identifier_t charId;
	const char *name;
	short level;
};

enum
{
	ACC_COLOR_OFF
};
extern const Lookup account_flags[];
Account *new_account( Connection * );
AccountPlayer *new_account_player(  );
void destroy_account( Account * );
void destroy_account_player( AccountPlayer * );
int load_account_players( Account * );
int load_account( Account *, const char * );
int delete_account( Account * );
int save_account( Account * );

#endif
