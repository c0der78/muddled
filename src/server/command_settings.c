
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "character.h"
#include "player.h"
#include "room.h"
#include "engine.h"
#include "log.h"
#include "account.h"
#include "connection.h"
#include "util.h"
#include "str.h"
#include "buffer.h"
#include "private.h"
#include "client.h"
#include "command.h"

DOFUN(color)
{

    if (ch->pc == 0)
        return;

    if (!is_set(ch->pc->account->flags, ACC_COLOR_OFF))
    {

        set_bit(ch->pc->account->flags, ACC_COLOR_OFF);

        xwriteln(ch, "Color off. *sigh*");

    }
    else
    {

        remove_bit(ch->pc->account->flags, ACC_COLOR_OFF);

        xwriteln(ch, "~?C~?o~?l~?o~?r ~?O~?n~?!~x");

    }

}

DOFUN(save)
{

    if (ch->pc == 0)
        return;

    if (!save_player(ch))
    {

        xwriteln(ch, "There was a problem saving this character.");

    }
    else
    {

        xwriteln(ch, "Character saved.");

    }

}

DOFUN(title)
{

    if (!ch->pc)
        return;

    free_str_dup(&ch->pc->title, argument);

    if (!argument || !*argument)
    {

        xwriteln(ch, "Title cleared.");

    }
    else
    {

        xwriteln(ch, "Title set.");

    }

}

DOFUN(autotick)
{

    if (!ch->pc)
        return;

    if (is_set(ch->pc->account->flags, PLR_TICKS_OFF))
    {

        remove_bit(ch->pc->account->flags, PLR_TICKS_OFF);

        xwriteln(ch, "You will now be notified of game ticks.");

    }
    else
    {

        set_bit(ch->pc->account->flags, PLR_TICKS_OFF);

        xwriteln(ch, "You will no longer be notified of game ticks.");

    }

}

DOFUN(brief)
{

    if (!ch->pc)
        return;

    if (is_set(ch->pc->account->flags, PLR_BRIEF))
    {

        remove_bit(ch->pc->account->flags, PLR_BRIEF);

        xwriteln(ch, "You now see room descriptions.");

    }
    else
    {

        set_bit(ch->pc->account->flags, PLR_BRIEF);

        xwriteln(ch, "You no longer see room descriptions.");

    }

}

DOFUN(timezone)
{

    if (!ch->pc)
        return;

    if (!*argument)
    {

        Buffer *buf = new_buf();

        const char *line = fillstr("~w-~W-", scrwidth(ch));

        xwritelnf(buf, "%-6s %-30s (%s)", "Name", "City/Zone Crosses",
                  "Time");

        time_t current_time = time(0);

        xwritelnf(buf, "%s~x", line);

        for (int i = 0; timezones[i].name != 0; i++)
        {

            xwritelnf(buf, "%-6s %-30s (%s)", timezones[i].name,
                      timezones[i].zone, str_time(current_time, i,
                                                  NULL));

        }
        xwritelnf(buf, "%s~x", line);

        xwritelnf(buf, "Use '%s <name>' to set your timezone.", do_name);

        ch->page(ch, buf_string(buf));

        destroy_buf(buf);

        return;

    }
    int i = timezone_lookup(argument);

    if (i == -1)
    {

        xwritelnf(ch,
                  "That time zone does not exists.  See '%s' for a list.",
                  do_name);

        return;

    }
    ch->pc->account->timezone = i;

    xwritelnf(ch, "Your time zone is now %s %s (%s)", timezones[i].name,
              timezones[i].zone, str_time(time(0), i, NULL));

}

DOFUN(hints)
{

    if (!ch->pc)
        return;

    if (is_set(ch->pc->account->flags, PLR_HINTS))
    {

        remove_bit(ch->pc->account->flags, PLR_HINTS);

        xwriteln(ch, "You no longer see hints.");

    }
    else
    {

        set_bit(ch->pc->account->flags, PLR_HINTS);

        xwriteln(ch, "You now see hints.");

    }

}

DOFUN(delete)
{

    if (!ch->pc)
        return;

    char_from_room(ch);

    UNLINK(first_character, Character, ch, next);

    UNLINK(first_player, Character, ch, next_player);

    client_delete_char(ch->pc->conn);

}

DOFUN(automap)
{

    if (!ch->pc)
        return;

    if (is_set(ch->pc->account->flags, PLR_AUTOMAP_OFF))
    {

        remove_bit(ch->pc->account->flags, PLR_AUTOMAP_OFF);

        xwriteln(ch, "You now see a map in room descriptions.");

    }
    else
    {

        set_bit(ch->pc->account->flags, PLR_AUTOMAP_OFF);

        xwriteln(ch, "You no longer see maps in room descriptions.");

    }

}

DOFUN(prompt)
{

    if (!ch->pc)
        return;

    free_str_dup(&ch->pc->prompt, argument);

    xwriteln(ch, "Prompt set.");

}
