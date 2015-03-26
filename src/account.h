/******************************************************************************
 *                                       _     _ _          _                 *
 *                   _ __ ___  _   _  __| | __| | | ___  __| |                *
 *                  | '_ ` _ \| | | |/ _` |/ _` | |/ _ \/ _` |                *
 *                  | | | | | | |_| | (_| | (_| | |  __/ (_| |                *
 *                  |_| |_| |_|\__,_|\__,_|\__,_|_|\___|\__,_|                *
 *                                                                            *
 *         (C) 2010 by Ryan Jennings <c0der78@gmail.com> www.arg3.com         *
 *             Many thanks to creators of muds before me.                     *
 *                                                                            *
 *        In order to use any part of this Mud, you must comply with the      *
 *     license in 'license.txt'.  In particular, you may not remove either    *
 *                        of these copyright notices.                         *
 *                                                                            *
 *       Much time and thought has gone into this software and you are        *
 *     benefitting.  I hope that you share your changes too.  What goes       *
 *                            around, comes around.                           *
 ******************************************************************************/

/*
 * ! @header Structure, methods and constants related to accounts @ignore
 * __ACCOUNT_H_
 */
#ifndef MUDDLED_ACCOUNT_H
#define MUDDLED_ACCOUNT_H

#include <time.h>
#include "lookup.h"
#include "typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	ACC_COLOR_OFF
};

/*!
 * @const account_flags @abstract the table used for account flag /
 * string conversion
 */
extern const Lookup account_flags[];

/*!
 * @group Memory Management
 */

/*!
 * creates a new account @return a newly allocated account
 */
Account *new_account(Connection *);
/*!
 * create a account player link and performs initialization @return a
 * newly allocated account player link
 */
AccountPlayer *new_account_player();
/*!
 * performs cleanup and destroys a created account @param account the
 * allocated account to destroy
 */
void free_account(Account *);
/*!
 * performs cleanup and destroys a created account player link @param
 * accountPlayer the allocated account player link to destroy
 */
void free_account_player(AccountPlayer *);

/*!
 * loads the players for an account @param account the allocated account
 * to load players for
 */
int load_account_players(Account *);

/*!
 * loads the forum data for an account @param account the allocated
 * account to load forum data for
 */
int load_account_forums(Account *);

/*!
 * loads the account for the login @param account the allocated account
 * to load into @param login the login id of the account to load @return 1
 * if successfull otherwise 0
 */
int load_account(Account *, const char *);
/*!
 * deletes an account record from saved data @param account the account
 * to delete @return 1 if successfull otherwise 0
 */
int delete_account(Account *);
/*!
 * saves an account to records @param account the account to save
 * @return 1 if successfull otherwise 0
 */
int save_account(Account *);

/*!
 * @group Data Management
 */

/*!
 * @param account the account to get value from @return the last read
 * note for the current forum
 */
time_t account_forum_last_note(Account *);

/*!
 * @param account the account to get value from @return true if account
 * is subscribed to current forum
 */
bool account_forum_is_subscribed(Account *);

/*!
 * sets the last note timestamp for the current forum @param account the
 * account to set value for @param time the created time of the last note
 */
void account_forum_set_last_note(Account *, time_t);

#ifdef __cplusplus
}
#endif

#endif
