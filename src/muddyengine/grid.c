
/******************************************************************************
 *         __  __           _     _         ____  _       _                   *
 *        |  \/  |_   _  __| | __| |_   _  |  _ \| | __ _(_)_ __  ___         *
 *        | |\/| | | | |/ _` |/ _` | | | | | |_) | |/ _` | | '_ \/ __|        *
 *        | |  | | |_| | (_| | (_| | |_| | |  __/| | (_| | | | | \__ \        *
 *        |_|  |_|\__,_|\__,_|\__,_|\__, | |_|   |_|\__,_|_|_| |_|___/        *
 *                                  |___/                                     *
 *                                                                            *
 *         (C) 2010 by Ryan Jennings <c0der78@gmail.com> www.arg3.com         *
 *	               Many thanks to creators of muds before me.                 *
 *                                                                            *
 *        In order to use any part of this Mud, you must comply with the      *
 *     license in 'license.txt'.  In particular, you may not remove either    *
 *                        of these copyright notices.                         *
 *                                                                            *
 *       Much time and thought has gone into this software and you are        *
 *     benefitting.  I hope that you share your changes too.  What goes       *
 *                            around, comes around.                           *
 ******************************************************************************/

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <muddyengine/character.h>
#include <muddyengine/string.h>
#include <muddyengine/grid.h>
#include <muddyengine/buffer.h>

void grid_init_border(Border * border)
{

    border->flags = BORDER_NONE;

    memset(border->color, 0, sizeof(border->color));

    border->vertical = ' ';

    border->horizontal = ' ';

}

static void grid_init_row(GridRow * row)
{

    for (int i = 0; i < row->columns; i++)
    {

        GridCell *cell = &row->cells[i];

        memset(cell->value, 0, sizeof(cell->value));

        cell->span = 1;

        memset(cell->color, 0, sizeof(cell->color));

        grid_init_border(&cell->border);

    }

}

Grid *new_grid(int width, int columns)
{

    Grid *grid = (Grid *) alloc_mem(1, sizeof(Grid));

    grid->width = width;

    grid->rowcount = 1;

    grid->defaultcolumns = columns;

    grid->rows = (GridRow *) alloc_mem(grid->rowcount, sizeof(GridRow));

    GridRow *row = &grid->rows[0];

    row->columns = columns;

    row->columnsize = grid->width / columns;

    row->cells = (GridCell *) alloc_mem(row->columns, sizeof(GridCell));

    grid_init_row(row);

    return grid;

}

void destroy_grid(Grid * grid)
{

    for (int i = 0; i < grid->rowcount; i++)
    {

        free_mem(grid->rows[i].cells);

    }
    free_mem(grid->rows);

    free(grid);

}

static void next_row(Grid * grid)
{

    grid->x = 0;

    if (++grid->y >= grid->rowcount)
    {

        grid->rowcount++;

        grid->rows =
            (GridRow *) realloc(grid->rows,
                                grid->rowcount * sizeof(GridRow));

        GridRow *row = &grid->rows[grid->rowcount - 1];

        row->columns = grid->defaultcolumns;

        row->columnsize = grid->width / row->columns;

        row->cells =
            (GridCell *) alloc_mem(row->columns, sizeof(GridCell));

        grid_init_row(row);

    }
}

void grid_set_next_row(Grid * grid, int columns)
{

    if (grid->x >= grid->rows[grid->y].columns)
        next_row(grid);

    GridRow *row = &grid->rows[grid->y];

    row->columns = columns;

    row->columnsize = grid->width / columns;

    row->cells =
        (GridCell *) realloc(row->cells, columns * sizeof(GridCell));

    for (int i = grid->x; i < row->columns; i++)
    {

        GridCell *cell = &row->cells[i];

        memset(cell->value, 0, sizeof(cell->value));

        cell->span = 1;

        memset(cell->color, 0, sizeof(cell->color));

        grid_init_border(&cell->border);

    }

}

int grid_add_default(Grid * grid, const char *value)
{

    return grid_add(grid, ALIGN_LEFT, 1, 0, 0, 1, 1, value);

}

int grid_add_border(Grid * grid, Border * border, const char *value)
{

    return grid_add(grid, ALIGN_LEFT, 1, border, 0, 1, 1, value);

}

int
grid_add(Grid * grid, align_t align, int colspan, Border * border,
         const char *color, int lm, int rm, const char *value)
{

    if (grid->x >= grid->rows[grid->y].columns)
        next_row(grid);

    grid_set(grid, grid->x, grid->y, align, colspan, border, color, lm, rm);

    int rval = grid->x;

    strncpy(grid->rows[grid->y].cells[grid->x].value,
            value ? value : str_empty, BUF_SIZ - 1);

    grid->x += colspan;

    return rval;

}

void
grid_set(Grid * grid, int x, int y, align_t align, int colspan,
         Border * border, const char *color, int left_mar, int right_mar)
{

    if (y < 0 || y > grid->rowcount || x < 0
            || x > grid->rows[grid->y].columns)
        return;

    GridCell *cell = &grid->rows[y].cells[x];

    cell->span = UMAX(1, colspan);

    if (border)
    {

        cell->border.flags = border->flags;

        cell->border.horizontal = border->horizontal;

        cell->border.vertical = border->vertical;

        strncpy(cell->border.color, border->color,
                sizeof(cell->border.color));

    }
    if (color)
    {

        strncpy(cell->color, color, sizeof(cell->color));

    }
    cell->left_margin = UMAX(0, left_mar);

    cell->right_margin = UMAX(0, right_mar);

    cell->align = align;

}

int grid_addf_border(Grid * grid, Border * border, const char *fmt, ...)
{

    va_list args;

    char buf[BUF_SIZ] = { 0 };

    va_start(args, fmt);

    vsnprintf(buf, sizeof(buf), fmt, args);

    va_end(args);

    return grid_add(grid, ALIGN_LEFT, 1, border, 0, 1, 1, buf);

}

int grid_addf_default(Grid * grid, const char *fmt, ...)
{

    va_list args;

    char buf[BUF_SIZ] = { 0 };

    va_start(args, fmt);

    vsnprintf(buf, sizeof(buf), fmt, args);

    va_end(args);

    return grid_add(grid, ALIGN_LEFT, 1, 0, 0, 1, 1, buf);

}

int
grid_addf(Grid * grid, align_t align, int colspan, Border * border,
          const char *color, int lm, int rm, const char *fmt, ...)
{

    va_list args;

    char buf[BUF_SIZ] = { 0 };

    va_start(args, fmt);

    vsnprintf(buf, sizeof(buf), fmt, args);

    va_end(args);

    return grid_add(grid, align, colspan, border, color, lm, rm, buf);

}

void grid_print(Grid * grid, Character * ch)
{

    Buffer *buf = new_buf();

    for (int i = 0; i < grid->rowcount; i++)
    {

        for (int j = 0; j < grid->rows[i].columns;
                j += grid->rows[i].cells[j].span)
        {

            GridRow *row = &grid->rows[i];

            GridCell *cell = &row->cells[j];

            Border *border = &cell->border;

            int width_adjust =
                cell->left_margin + cell->right_margin;

            if (border->flags & BORDER_LEFT)
            {

                writef(buf, "%s%c~x", border->color,
                       border->vertical);

                width_adjust++;

            }
            if (cell->left_margin > 0)
            {

                writef(buf, "%*s", cell->left_margin, " ");

            }
            if (border->flags & BORDER_RIGHT)
            {

                width_adjust++;

            }
            int colwidth =
                (row->columnsize * cell->span) - (width_adjust);

            if (j + cell->span >= row->columns)
            {

                int loss =
                    (grid->width -
                     (row->columnsize * row->columns));

                colwidth += loss;

            }
            write(buf,
                  align_string(cell->align, colwidth, cell->color,
                               (border->flags & BORDER_INLINE) ?
                               &border->horizontal : 0,
                               cell->value));

            if (cell->right_margin > 0)
            {

                writef(buf, "%*s", cell->right_margin, " ");

            }
            if (border->flags & BORDER_RIGHT)
            {

                writef(buf, "%s%c~x", border->color,
                       border->vertical);

            }
        }

        writeln(buf, "");

    }

    ch->page(ch, buf_string(buf));

    destroy_buf(buf);

}
