
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

#ifndef __GRID_H_
#define __GRID_H_

#include <muddyengine/engine.h>
#include <muddyengine/string.h>

typedef struct gridcell GridCell;

typedef struct gridrow GridRow;

typedef struct grid Grid;

typedef struct border Border;

struct border
{

	char color[4];

	char vertical;

	char horizontal;

	int flags;

};

struct gridcell
{

	char value[BUF_SIZ];

	int span;

	int left_margin;

	int right_margin;

	int align;

	Border border;

	char color[4];

};

struct gridrow
{

	int columns;

	int columnsize;

	struct gridcell *cells;

};

struct grid
{

	struct gridrow *rows;

	int rowcount;

	int defaultcolumns;

	int x;

	int y;

	int width;

};

enum
{
	BORDER_NONE = 0,
	BORDER_LEFT = ( 1 << 0 ),
	BORDER_RIGHT = ( 1 << 1 ),
	BORDER_INLINE = ( 1 << 2 )
};

Grid *new_grid( int, int );

void destroy_grid( Grid * );

void grid_init_border( Border * );

int grid_add( Grid *, align_t, int, Border *, const char *, int, int,
			  const char * );

void grid_set( Grid *, int, int, align_t, int, Border *, const char *, int,
			   int );

int grid_addf( Grid *, align_t, int, Border *, const char *, int, int,
			   const char *, ... ) __attribute__ ( ( format( printf, 8, 9 ) ) );

int grid_add_default( Grid *, const char * );

int grid_addf_default( Grid *, const char *, ... )
	__attribute__ ( ( format( printf, 2, 3 ) ) );

int grid_add_border( Grid *, Border *, const char * );

int grid_addf_border( Grid *, Border *, const char *, ... )
	__attribute__ ( ( format( printf, 3, 4 ) ) );

void grid_print( Grid *, Character * );

void grid_set_next_row( Grid *, int );

#endif							/* 
								 */
