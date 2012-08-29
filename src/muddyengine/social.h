
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
#ifndef SOCIAL_H
#define SOCIAL_H

typedef struct Social Social;

#include <stdlib.h>
#include <muddyengine/string.h>
#include <muddyengine/character.h>

struct Social {
	identifier_t id;
	Social *next;
	position_t minPosition;
	const char *name;
	const char *charNoArg;
	const char *othersNoArg;
	const char *charFound;
	const char *othersFound;
	const char *victFound;
	const char *charNotFound;
	const char *charAuto;
	const char *othersAuto;
	const char *charObjFound;
	const char *othersObjFound;
};

extern Social *first_social;
Social *new_social();
void destroy_social(Social *);
int load_socials();
int save_social(Social *);
void save_socials();
int interpret_social(Character *, const char *, const char *);
Social *social_lookup(const char *);

#endif				/* // #ifndef SOCIAL_H */
