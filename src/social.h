
/******************************************************************************
 *                                       _     _ _          _                 *
 *                   _ __ ___  _   _  __| | __| | | ___  __| |                *
 *                  | '_ ` _ \| | | |/ _` |/ _` | |/ _ \/ _` |                *
 *                  | | | | | | |_| | (_| | (_| | |  __/ (_| |                *
 *                  |_| |_| |_|\__,_|\__,_|\__,_|_|\___|\__,_|                *
 *                                                                            *
 *         (C) 2010 by Ryan Jennings <c0der78@gmail.com> www.arg3.com         *
 *                 Many thanks to creators of muds before me.                 *
 *                                                                            *
 *        In order to use any part of this Mud, you must comply with the      *
 *     license in 'license.txt'.  In particular, you may not remove either    *
 *                        of these copyright notices.                         *
 *                                                                            *
 *       Much time and thought has gone into this software and you are        *
 *     benefitting.  I hope that you share your changes too.  What goes       *
 *                            around, comes around.                           *
 ******************************************************************************/
#ifndef MUDDLED_SOCIAL_H
#define MUDDLED_SOCIAL_H

#include "cdecl.h"
#include "typedef.h"

struct social
{
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

BEGIN_DECL

Social *new_social();
void destroy_social(Social *);
int load_socials();
int save_social(Social *);
void save_socials();
int interpret_social(Character *, const char *, const char *);
Social *social_lookup(const char *);

END_DECL

#endif              /* // #ifndef SOCIAL_H */
