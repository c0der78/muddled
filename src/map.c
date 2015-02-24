
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

#include "muddled/character.h"
#include "muddled/player.h"
#include "muddled/room.h"
#include "muddled/exit.h"
#include "muddled/color.h"
#include "muddled/util.h"
#include "muddled/explored.h"
#include "muddled/string.h"
#include <stdio.h>

char map_chars[5] = "|-|-";

char map_chars_closed[5] = "I=I=";

char lcolor[3];

#define    MAXDEPTH  4
#define    MAPX     10
#define    MAPY      8
#define    BOUNDARY(x, y) (((x) < 0) || ((y) < 0) || ((x) > (MAPX * 2)) || ((y) > (MAPY * 2)))

typedef struct map Map;

struct map
{

    char symbol;

    int depth;

    Room *pRoom;

};

Map automap[(MAPX * 2) + 1][(MAPY * 2) + 1];

const struct sector_symbol_t
{

    sector_t sector;

    const char *color;

    char symbol;

} sector_symbols[] =
{

    {
        SECT_INSIDE, "~w", 'o'
    }, {
        SECT_CITY, "~W", 'o'
    }, {
        SECT_FIELD, "~G", '*'
    }, {
        SECT_FOREST, "~g", '*'
    }, {
        SECT_HILLS, "~y", '!'
    }, {
        SECT_MOUNTAIN, "~w", '@'
    }, {
        SECT_WATER_SWIM, "~B", '='
    }, {
        SECT_WATER_NOSWIM, "~b", '='
    }, {
        SECT_AIR, "~C", '%'
    }, {
        SECT_DESERT, "~y", '+'
    }, {
        SECT_JUNGLE, "~G", '&'
    }, {
        SECT_ROAD, "~m", ':'
    }, {
        SECT_PATH, "~M", ':'
    }, {
        SECT_CAVE, "~w", '#'
    }, {
        SECT_SWAMP, "~g", '&'
    }, {
        SECT_UNKNOWN, "~D", '?'
    }, {
        0, 0, 0
    }
};

void get_exit_dir(direction_t dir, int *x, int *y, int xorig, int yorig)
{

    switch (dir)
    {

    case DIR_NORTH:

        *x = xorig;

        *y = yorig - 1;

        break;

    case DIR_EAST:

        *x = xorig + 1;

        *y = yorig;

        break;

    case DIR_SOUTH:

        *x = xorig;

        *y = yorig + 1;

        break;

    case DIR_WEST:

        *x = xorig - 1;

        *y = yorig;

        break;

    default:

        *x = -1;

        *y = -1;

        break;

    }

}

void clear_coord(int x, int y)
{

    automap[x][y].symbol = '.';

    automap[x][y].depth = 0;

    automap[x][y].pRoom = NULL;

}

void map_exits(int depth, Character *ch, Room *pRoom, int x, int y)
{

    int door;

    int exitx = 0, exity = 0;

    int roomx = 0, roomy = 0;

    Exit *pExit;

    if (!pRoom || !can_see_room(ch, pRoom))
        return;

    automap[x][y].symbol = sector_symbols[pRoom->sector].symbol;

    automap[x][y].depth = depth;

    automap[x][y].pRoom = pRoom;

    if (depth >= MAXDEPTH)
        return;

    for (door = 0; door < MAX_DIR; door++)
    {

        if ((pExit = pRoom->exits[door]) == NULL)
            continue;

        if (pExit->to.room == NULL)
            continue;

        if (!can_see_room(ch, pExit->to.room))
            continue;

        get_exit_dir(door, &exitx, &exity, x, y);

        get_exit_dir(door, &roomx, &roomy, exitx, exity);

        if (BOUNDARY(exitx, exity) || BOUNDARY(roomx, roomy))
            continue;

        if (depth == MAXDEPTH)
            continue;

        automap[exitx][exity].depth = depth;

        if (is_set(pExit->status, EXIT_CLOSED))
            automap[exitx][exity].symbol = map_chars_closed[door];

        else
            automap[exitx][exity].symbol = map_chars[door];

        automap[exitx][exity].pRoom = pExit->to.room;

        if (door == DIR_UP || door == DIR_DOWN)
            continue;

        if ((depth < MAXDEPTH)
                && ((automap[roomx][roomy].pRoom == pExit->to.room)
                    || (automap[roomx][roomy].pRoom == NULL)))
        {

            map_exits(depth + 1, ch, pExit->to.room, roomx, roomy);

        }
    }

}

char *erase_new_lines(const char *desc)
{

    unsigned int l, m;

    static char buf[OUT_SIZ * 2];

    char temp[OUT_SIZ * 2];

    if (nullstr(desc))
        return "";

    l = 0;

    m = 0;

    buf[0] = 0;

    temp[0] = 0;

    for (m = 0; desc[m] != 0; m++)
    {

        if (desc[m] == '\n' || desc[m] == '\r')
            temp[m] = '\x14';

        else
            temp[m] = desc[m];

    }

    temp[m] = 0;

    for (m = 0; temp[m] != 0; m++)
    {

        if (temp[m] == '\x14')
        {

            buf[l++] = ' ';

            do
            {

                m++;

            }
            while (temp[m] == '\x14');

        }
        buf[l++] = temp[m];

    }

    buf[l] = 0;

    return buf;

}

unsigned int get_line_len(const char *desc, unsigned int max_len)
{

    unsigned int m;

    unsigned int l;

    if (nullstr(desc))
        return 0;

    l = 0;

    for (m = 0; desc[m] != 0; m++)
    {

        if (desc[m] == COLOR_CODE)
        {

            int k = m + 1;

            if (desc[m] == '!')
                k++;

            strncpy(lcolor, &desc[m], k + 1);

            lcolor[k + 1] = 0;

            m += k;

        }
        else if (++l == max_len)
            break;

    }

    if (l < max_len)
        return 0;

    for (l = m; l > 0; l--)
        if (desc[l] == ' ')
            break;

    if (l == 0)
        return m;

    return l + 1;

}

void show_map(Character *ch, const char *text, bool smallMap)
{

    char buf[OUT_SIZ * 2];

    int x, y, m, n, pos;

    const char *p;

    bool alldesc = false;

    int maxlen = scrwidth(ch);

    int maplen = maxlen - 15;

    if (smallMap)
    {

        m = 4;

        n = 5;

    }
    else
    {

        m = 0;

        n = 0;

    }

    pos = 0;

    p = text;

    buf[0] = 0;

    strcpy(lcolor, "~x");

    if (smallMap)
    {

        if (!ch->pc || !is_explorable(ch->inRoom))
        {

            sprintf(buf, "~R+-----------+%s ", lcolor);

        }
        else
        {

            int rcnt =
                areacount(ch->pc->explored, ch->inRoom->area);

            double rooms = (double)(arearooms(ch->inRoom->area));

            double percent = UMIN(rcnt / (rooms / 100.0), 100.0);

            sprintf(buf, "~R+-----[~x%3.0f%%~R]+%s ", percent,
                    lcolor);

        }
        if (!alldesc)
        {

            pos = get_line_len(p, maplen);

            if (pos > 0)
            {

                strncat(buf, p, pos);

                p += pos;

            }
            else
            {

                strcat(buf, p);

                alldesc = true;

            }

        }
        strcat(buf, "\n\r");

    }
    for (y = m; y <= (MAPY * 2) - m; y++)
    {

        if (smallMap)
            strcat(buf, "~R|");

        else
            strcat(buf, "~D");

        for (x = n; x <= (MAPX * 2) - n; x++)
        {

            if (automap[x][y].pRoom)
            {

                if (automap[x][y].symbol ==
                        sector_symbols[automap[x][y].pRoom->sector].
                        symbol && ch->pc
                        && is_set(ch->pc->explored,
                                  automap[x][y].pRoom->id))
                {

                    if (automap[x][y].pRoom->exits[DIR_UP]
                            && automap[x][y].pRoom->
                            exits[DIR_DOWN])
                        automap[x][y].symbol = 'B';

                    else if (!automap[x][y].pRoom->
                             exits[DIR_UP]
                             && automap[x][y].pRoom->
                             exits[DIR_DOWN])
                        automap[x][y].symbol = 'D';

                    else if (automap[x][y].pRoom->
                             exits[DIR_UP]
                             && !automap[x][y].pRoom->
                             exits[DIR_DOWN])
                        automap[x][y].symbol = 'U';

                }
                if (!smallMap)
                    sprintf(buf + strlen(buf), " %s%c~D",
                            sector_symbols[automap[x][y].
                                           pRoom->sector].
                            color, automap[x][y].symbol);

                else
                    sprintf(buf + strlen(buf), "%s%c",
                            sector_symbols[automap[x][y].
                                           pRoom->sector].
                            color,
                            automap[x][y].symbol !=
                            '.' ? automap[x][y].
                            symbol : ' ');

            }
            else
            {

                if (!smallMap)
                    strcat(buf, " ~D.");

                else
                    strcat(buf, " ");

            }

        }

        if (!smallMap)
        {

            switch (y)
            {

            case 0:

                strcat(buf, "   ~xX   You are here\n\r");

                break;

            case 2:

                strcat(buf, "   ~xo   Normal Rooms\n\r");

                break;

            case 3:

                strcat(buf, "   ~xU   Room with exit up\n\r");

                break;

            case 4:

                strcat(buf, "   ~xD   Room with exit down\n\r");

                break;

            case 5:

                strcat(buf,
                       "   ~xB   Room with exits up & down\n\r");

                break;

            case 6:

                strcat(buf, "   ~x|-  Exits\n\r");

                break;

            case 7:

                strcat(buf, "   ~xI=  Closed Doors\n\r");

                break;

            case 8:

                strcat(buf, "   ~x*   Field/Forest/Hills\n\r");

                break;

            case 9:

                strcat(buf, "   ~x@   Mountain/Cave\n\r");

                break;

            case 10:

                strcat(buf, "   ~x=   Water\n\r");

                break;

            case 11:

                strcat(buf, "   ~x%   Air\n\r");

                break;

            case 12:

                strcat(buf, "   ~x+   Desert\n\r");

                break;

            case 13:

                strcat(buf, "   ~x#   Jungle/Swamp\n\r");

                break;

            case 14:

                strcat(buf, "   ~xO   Road/Path\n\r");

                break;

            default:

                strcat(buf, "   ~x\n\r");

                break;

            }

        }
        else
        {

            sprintf(buf + strlen(buf), "~R|%s ", lcolor);

            if (!alldesc)
            {

                pos = get_line_len(p, maplen);

                if (pos > 0)
                {

                    strncat(buf, p, pos);

                    p += pos;

                }
                else
                {

                    strcat(buf, p);

                    alldesc = true;

                }

            }
            strcat(buf, "\n\r");

        }

    }

    if (!smallMap)
        writelnf(ch, "%s\n\r%s%s", fillstr(NULL, scrwidth(ch)), buf,
                 fillstr(NULL, scrwidth(ch)));

    else
    {

        sprintf(buf + strlen(buf), "~R+-----------+%s ", lcolor);

        if (!alldesc)
        {

            pos = get_line_len(p, maplen);

            if (pos > 0)
            {

                strncat(buf, p, pos);

                p += pos;

            }
            else
            {

                strcat(buf, p);

                alldesc = true;

            }

        }
        strcat(buf, "\n\r");

        if (!alldesc)
        {

            do
            {

                pos = get_line_len(p, maxlen);

                if (pos > 0)
                {

                    strncat(buf, p, pos);

                    p += pos;

                }
                else
                {

                    strcat(buf, p);

                    alldesc = true;

                }

                strcat(buf, "\n\r");

            }
            while (!alldesc);

        }
        write(ch, buf);

    }

}

void draw_map(Character *ch, const char *desc)
{

    int x, y;

    const char *buf;

    bool smallMap;

    if (nullstr(desc))
    {

        smallMap = false;

        buf = desc;

    }
    else
    {

        smallMap = true;

        buf = erase_new_lines(desc);

    }

    for (y = 0; y <= MAPY * 2; y++)
    {

        for (x = 0; x <= MAPX * 2; x++)
        {

            clear_coord(x, y);

        }

    }

    x = MAPX;

    y = MAPY;

    int depth = (smallMap) ? 2 : 0;

    map_exits(depth, ch, ch->inRoom, x, y);

    automap[x][y].symbol = 'X';

    show_map(ch, buf, smallMap);

}
