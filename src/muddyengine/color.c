
/******************************************************************************
 *         __  __           _     _         ____  _       _                   *
 *        |  \/  |_   _  __| | __| |_   _  |  _ \| | __ _(_)_ __  ___         *
 *        | |\/| | | | |/ _` |/ _` | | | | | |_) | |/ _` | | '_ \/ __|        *
 *        | |  | | |_| | (_| | (_| | |_| | |  __/| | (_| | | | | \__ \        *
 *        |_|  |_|\__,_|\__,_|\__,_|\__, | |_|   |_|\__,_|_|_| |_|___/        *
 *                                  |___/                                     *
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

#include <muddyengine/color.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <muddyengine/flag.h>
#include <muddyengine/character.h>
#include <muddyengine/macro.h>
#include <muddyengine/util.h>
#include <muddyengine/buffer.h>

static bool is_valid_attr(short a)
{

    return (a == BOLD) || (a == NORMAL) || (a == BLINK);

}

static bool is_valid_color(short c)
{

    return (c >= BLACK && c <= WHITE) ||
           (c >= (BLACK + BG_MOD) && c <= (WHITE + BG_MOD));

}

const char *make_terminal_color(color_t *color)
{

    char *buf = get_temp_buf();

    int len = snprintf(buf, BUFSIZ, "\x1b[0;");

    if (is_valid_attr(color->attr))
        len += sprintf(&buf[len], "%d;", color->attr);

    if (is_valid_color(color->value))
        sprintf(&buf[len], "%d", color->value);

    else
        buf[--len] = 0;

    strcat(buf, "m");

    return buf;

}

const char *make_html_color(color_t *color)
{
    char *buf = get_temp_buf();

    int len = snprintf(buf, BUFSIZ, "<span class=\"");

    if (is_valid_attr(color->attr) && color->attr)
    {
        len += snprintf(&buf[len], BUFSIZ, "bold");
    }

    if (is_valid_color(color->value))
    {
        switch (color->value)
        {
        case RED:
            len += snprintf(&buf[len] , BUFSIZ, "red");
            break;
        case GREEN:
            len += snprintf(&buf[len], BUFSIZ,  "green");
            break;
        case YELLOW:
            len += snprintf(&buf[len], BUFSIZ,  "yellow");
            break;
        case BLUE:
            len += snprintf(&buf[len], BUFSIZ,  "blue");
            break;
        case CYAN:
            len += snprintf(&buf[len], BUFSIZ,  "cyan");
            break;
        case MAGENTA:
            len += snprintf(&buf[len], BUFSIZ,  "magenta");
            break;
        case WHITE:
            len += snprintf(&buf[len], BUFSIZ,  "white");
            break;
        case BLACK:
            len += snprintf(&buf[len], BUFSIZ,  "black");
            break;
        }
    }

    len += snprintf(&buf[len], BUFSIZ, "\">");

    return buf;
}

const char *finish_html_color()
{
    char *buf = get_temp_buf();

    strncpy(buf, "</span>", BUFSIZ);

    return buf;
}

const char *convert_color_code(const char *pstr, color_t *color)
{

    assert(pstr && *pstr == COLOR_CODE);

    pstr++;

    bool bg = *pstr == '!';

    if (bg)
        pstr++;

    if (color == 0 || *pstr == COLOR_CODE)
        return pstr;

    color->value = bg ? BG_MOD : 0;

    if (islower((int)*pstr))
    {

        color->attr = 0;

    }
    else if (isupper((int)*pstr))
    {

        color->attr = 1;

    }
    else
    {

        color->attr = -1;

    }

    // add to the color value so the background mod will be
    // taken into account
    switch (UPPER(*pstr))
    {

    case '\0':

        break;

    case 'X':

        color->attr = 0;

        color->value = 0;

        break;

    case '?':

        color->attr = number_range(0, 1);

        color->value += number_range(BLACK + 1, WHITE);

        break;

    case 'B':

        color->value += BLUE;

        break;

    case 'G':

        color->value += GREEN;

        break;

    case 'R':

        color->value += RED;

        break;

    case 'Y':

        color->value += YELLOW;

        break;

    case 'M':

        color->value += MAGENTA;

        break;

    case 'W':

        color->value += WHITE;

        break;

    case 'C':

        color->value += CYAN;

        break;

    case 'D':

        color->value += BLACK;

        break;

    }

    return pstr;

}
