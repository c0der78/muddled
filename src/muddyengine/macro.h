
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
#ifndef MACRO_H
#define MACRO_H

#include <stdlib.h>
#include <stdbool.h>

#define UPPER(c) (toupper((int)(c)))
#define LOWER(c) (tolower((int)(c)))
#define UMIN(a, b) ((a) < (b) ? (a) : (b))
#define UMAX(a, b) ((a) > (b) ? (a) : (b))
#define URANGE(a, b, c) ((b) < (a) ? (a) : ((b) > (c) ? ((c) > (a) ? (c) : (a)) : (b)))

#define percent(a, b)	(((a) * 100) / b)

#define write(to, arg) 		(to)->write((to), (arg))
#define writeln(to, arg) 	(to)->writeln((to), (arg))
#define writef(to, arg, ...)	(to)->writef((to), (arg), __VA_ARGS__)
#define writelnf(to, arg, ...)	(to)->writelnf((to), (arg), __VA_ARGS__)

#define NAME(ch) ((ch->npc != 0) ? (ch->npc->shortDescr) : (ch->name))

#define UNLINK(plist, ptype, pdata, pnext) \
do { \
	if (plist == pdata) \
		plist = pdata->pnext; \
        else { \
            ptype *_prev; \
            for (_prev = plist; _prev != NULL; _prev = _prev->pnext) { \
                if (_prev->pnext == pdata) { \
                    _prev->pnext = pdata->pnext; \
                    break; \
                } \
            } \
        } \
} while(0)

#define LINK(plist, pdata, pnext) \
do { \
	if((pdata) == NULL) \
		break; \
	(pdata)->pnext = (plist); \
	(plist) = (pdata); \
} while(0)

#define LINK_LAST(plist, ptype, pdata, pnext) \
do { \
	if(plist == NULL) { \
		(plist) = (pdata); \
		break; \
	} \
	ptype *p; \
	for (p = plist; p->pnext; p = p->pnext ) ; \
	p->pnext = pdata; \
} while(0)

#endif
