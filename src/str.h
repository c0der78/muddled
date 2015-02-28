
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
#ifndef MUDDLED_STRING_H
#define MUDDLED_STRING_H

#include "cdecl.h"
#include "typedef.h"

typedef enum
{
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT,
	ALIGN_INDENT
} align_t;

extern const char str_empty[];

BEGIN_DECL

const char *capitalize(const char *);
char *trim(char *);
bool nullstr(const char *);
const char *align_string(align_t, int, const char *, const char *,
                         const char *);
const char *valign_string(align_t, int, const char *, const char *,
                          const char *, va_list);
bool str_suffix(const char *, const char *);
bool str_infix(const char *, const char *);
bool str_prefix(const char *, const char *);
bool str_cmp(const char *, const char *);
int is_number(const char *);
bool is_valid_email(const char *);
const char *first_arg(const char *, char *, bool);
const char *one_argument(const char *, char *);
const char *ordinal_string(int);
int match(const char *, const char *);

void free_str(const char *);
void free_str_dup(const char **, const char *);
const char *str_dup(const char *);
int strlen_color(const char *);
const char *fillstr(const char *, size_t);
const char *strip_color(const char *);
bool is_name(const char *, const char *);
bool is_exact_name(const char *, const char *);

const char *strupper(const char *);

const char *str_replace(const char *, const char *, const char *);
const char *str_replace_all(const char *, const char *, const char *);
const char *str_ireplace(const char *, const char *, const char *);
const char *str_ireplace_all(const char *, const char *, const char *);

const char *stristr(const char *, const char *);
const char *get_line(const char *, char *);
int count_lines(const char *);

long number_argument(const char *, char *);
long multi_argument(const char *, char *);

int strpos(const char *, int);
const char *formatf(const char *, ...)
__attribute__ ((format(printf, 1, 2)));

END_DECL

#endif
