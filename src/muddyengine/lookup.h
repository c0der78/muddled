
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

#ifndef LOOKUP_H
#define LOOKUP_H

#include <stdint.h>

typedef struct Lookup Lookup;

struct Lookup {
	const char *name;
	uintptr_t value;
};

int index_lookup(const Lookup *, const char *);

uintptr_t value_lookup(const Lookup *, const char *);

const char *values_tostring(const Lookup *, uintptr_t *);
void values_fromstring(const Lookup *, const char *, uintptr_t *);

const char *lookup_names(const Lookup *);
const char *lookup_name(const Lookup *, uintptr_t);

void synchronize_tables();

#endif				/* // #ifndef LOOKUP_H */
