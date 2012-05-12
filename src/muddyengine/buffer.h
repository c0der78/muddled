
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

#ifndef BUFFER_H
#define BUFFER_H

typedef struct Buffer Buffer;

struct Buffer {

	short state;		/* error state of the buffer */

	int size;		/* size in k */

	char *string;		/* buffer's string */

	 bool(*write) (Buffer *, const char *);

	 bool(*writeln) (Buffer *, const char *);

	 bool(*writef) (Buffer *, const char *, ...);

	 bool(*writelnf) (Buffer *, const char *, ...);

};

Buffer *new_buf();

void destroy_buf(Buffer *);

bool buf_add(Buffer *, const char *);

bool buf_add_len(Buffer *, const char *, size_t);

bool buf_addln(Buffer *, const char *);

bool buf_addf(Buffer *, const char *, ...)
    __attribute__ ((format(printf, 2, 3)));

bool buf_addlnf(Buffer *, const char *, ...)
    __attribute__ ((format(printf, 2, 3)));

void clear_buf(Buffer *);

char *buf_string(Buffer *);

#endif				/* // #ifndef BUFFER_H */
