
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

#ifndef MUDDLED_FLAG_H
#define MUDDLED_FLAG_H

typedef long long bit_t;
typedef struct flag Flag;

#include "lookup.h"
#include <stdbool.h>
#include <stddef.h>

/*!
 * A bitvector structure with unlimited number of bits
 */
struct flag
{
    int *bits;
    size_t size;
};

Flag *new_flag();
Flag *init_flag(int);
void destroy_flags(Flag *);
Flag *copy_flags(Flag *, Flag *);
Flag *set_bit(Flag *, bit_t);
Flag *set_flags(Flag *, Flag *);
Flag *toggle_bit(Flag *, bit_t);
Flag *toggle_flags(Flag *, Flag *);
Flag *remove_bit(Flag *, bit_t);
Flag *remove_flags(Flag *, Flag *);
bool is_set(Flag *, bit_t);
bool is_empty(Flag *);

/*!
 * checks if all bits in a flag are set in another flag
 */
bool flags_set(Flag *, Flag *);
/*!
 * parses the flags from a string  using 'toggle' instead of 'set'
 *
 * @return the number of flags parsed
 */
int parse_flags_toggle(Flag *, const char *, const Lookup *);
/*!
 * formats the flags into a string
 */
const char *format_flags(Flag *, const Lookup *);
/*!
 * Parses flags into a string
 *
 * @return the number of flags parsed
 */
int parse_flags(Flag *, const char *, const Lookup *);
/*!
 * @return the flag as an integer
 */
int flag_toint(Flag *);

#endif              /* // #ifndef FLAG_H */
