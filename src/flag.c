
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
#include "flag.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "log.h"
#include "engine.h"
#include "lookup.h"
#include "string.h"

Flag *new_flag()
{
    Flag *flags = (Flag *) alloc_mem(1, sizeof(Flag));
    flags->size = 0;
    flags->bits = 0;
    return flags;
}

void destroy_flags(Flag *flags)
{
    if (flags->size > 0)
        free_mem(flags->bits);

    free(flags);
}

static void resize_bits(Flag *flags, size_t size)
{
    size_t oldsize = flags->size;

    flags->size = size + 1;
    flags->bits = (int *)realloc(flags->bits, flags->size * sizeof(bit_t));
    if (flags->bits == 0)
    {
        log_error("unable to resize bit field");
        flags->size = 0;
        return;
    }
    while (oldsize < flags->size)
        flags->bits[oldsize++] = 0;
}

Flag *init_flag(int bit)
{
    Flag *flag = new_flag();
    flag->size = 1;
    resize_bits(flag, 1);
    flag->bits[0] = bit;
    return flag;
}

Flag *copy_flags(Flag *f1, Flag *f2)
{
    resize_bits(f1, f2->size);
    for (int i = 0; i < f1->size; i++)
        f1[i] = f2[i];

    return f1;
}

Flag *set_bit(Flag *flags, bit_t bit)
{
    size_t pos = (size_t) (bit / sizeof(bit_t));

    if (pos >= flags->size)
    {
        resize_bits(flags, pos);
    }
    flags->bits[pos] |= (1 << (bit % sizeof(bit_t)));

    return flags;
}

Flag *set_flags(Flag *flags, Flag *val)
{

    if (val->size > flags->size)
    {
        resize_bits(flags, val->size - 1);
    }
    for (int i = 0; i < val->size; i++)
    {
        flags->bits[i] |= val->bits[i];
    }

    return flags;
}

Flag *remove_bit(Flag *flags, bit_t bit)
{
    bit_t pos = bit / sizeof(bit_t);

    if (pos >= flags->size)
        return flags;

    flags->bits[pos] &= ~(1 << (bit % sizeof(bit_t)));

    return flags;
}

Flag *remove_flags(Flag *flags, Flag *val)
{
    for (int i = 0; i < val->size; i++)
    {
        if (i >= flags->size)
            break;

        flags->bits[i] &= val->bits[i];
    }

    return flags;
}

Flag *toggle_bit(Flag *flags, bit_t bit)
{
    size_t pos = (size_t) (bit / sizeof(bit_t));

    if (pos >= flags->size)
    {
        resize_bits(flags, pos);
    }
    flags->bits[pos] ^= (1 << (bit % sizeof(bit_t)));

    return flags;
}

Flag *toggle_flags(Flag *flags, Flag *val)
{
    if (val->size > flags->size)
    {
        resize_bits(flags, val->size - 1);
    }
    for (int i = 0; i < val->size; i++)
    {
        flags->bits[i] ^= val->bits[i];
    }

    return flags;
}

int flag_toint(Flag *flags)
{
    if (flags->size == 0)
        return 0;

    return flags->bits[0];
}

bool is_set(Flag *flags, bit_t bit)
{
    bit_t pos = bit / sizeof(bit_t);

    if (pos >= flags->size)
        return false;

    return (flags->bits[pos] & (1 << (bit % sizeof(bit_t))));
}

bool is_empty(Flag *flag)
{
    if (flag->size == 0)
    {
        return true;
    }
    bool empty = true;

    for (int i = 0; i < flag->size; i++)
        empty = empty && flag->bits[i] == 0;

    return empty;
}

bool flags_set(Flag *flags, Flag *val)
{
    bool isset = true;

    for (int i = 0; i < val->size; i++)
    {
        if (i >= flags->size)
        {
            if (val->bits[i] != 0)
                return false;

            continue;
        }
        isset = isset && (flags->bits[i] & val->bits[i]);
    }

    return isset;
}

int parse_flags_toggle(Flag *flags, const char *arglist, const Lookup *table)
{
    int res = 0;

    for (const Lookup *t = table; t->name != 0; t++)
    {
        if (is_name(t->name, arglist))
        {
            toggle_bit(flags, t->value);
            res++;
        }
    }
    return res;
}

int parse_flags(Flag *flags, const char *format, const Lookup *table)
{
    const char *name = strtok((char *)format, ",");
    int res = 0;

    while (name != 0)
    {
        long f = value_lookup(table, name);

        if (f != -1)
        {
            set_bit(flags, f);
            res++;
        }
        name = strtok(NULL, ",");
    }
    return res;
}

const char *format_flags(Flag *flags, const Lookup *table)
{
    static char buf[3][OUT_SIZ];
    static int i;

    ++i, i %= 3;
    char *res = buf[i];

    *res = 0;

    for (const Lookup *t = table; t->name != 0; t++)
    {
        if (!is_set(flags, t->value))
            continue;

        strcat(res, t->name);
        strcat(res, ",");
    }

    if (res[0] != 0)
    {
        res[strlen(res) - 1] = 0;
    }
    else
    {
        strcpy(res, "None");
    }

    return res;
}
