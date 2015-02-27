
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

#include "channel.h"
#include "string.h"
#include "character.h"
#include "player.h"
#include "util.h"
#include "connection.h"
#include "room.h"
#include "engine.h"
#include "social.h"
#include "account.h"
#include <stdarg.h>
#include "object.h"
#include "log.h"
#include "forum.h"
#include <stdio.h>

int gcn_chat = 0;

int gcn_admin = 0;

const Lookup channel_flags[] =
{
    {"chat", CHANNEL_CHAT},
    {"admin", CHANNEL_ADMIN},
    {0, 0}
};

const Channel channel_table[] =
{
    {
        &gcn_chat, CHANNEL_CHAT, "chat",
        "A global channel for open communication.",
        "~m[~MChat~m]", "~M",
        channel_global
    },
    {
        &gcn_admin,
        CHANNEL_ADMIN,
        "admin",
        "A channel for administrators.",
        "~c[~CAdmin~c]",
        "~C",
        channel_admin
    },

    {
        0, 0,
        0
    }
};

void initialize_channels()
{

    for (int i = 0; channel_table[i].name != 0; i++)
        (*channel_table[i].pgcn) = i;

}

bool
channel_viewable(const Character *ch, const Character *victim,
                 const Channel *chan)
{

    switch (chan->type)
    {

    case channel_global:

        return true;

    case channel_admin:

        return is_immortal(victim);

    default:

        return false;

    }

}

const char *format_channel(const Channel *chan, const Character *ch)
{

    static char buf[OUT_SIZ];

    switch (chan->type)
    {

    default:

        strcpy(buf, chan->format);

        break;

    }

    strcat(buf, chan->color);

    return buf;

}

const Channel *channel_lookup(const char *arg)
{

    if (nullstr(arg))
        return 0;

    for (int i = 0; channel_table[i].name != 0; i++)
        if (!str_prefix(arg, channel_table[i].name))
            return &channel_table[i];

    return 0;

}

char is_eye(char eye)
{

    char eyes[] = { ':', ';', '=', 0 };

    for (int i = 0; eyes[i] != 0; i++)
        if (eyes[i] == eye)
            return eyes[i];

    return 0;

}

char is_nose(char nose)
{

    char noses[] = { '-', '\'', 'o', '~', '^', 0 };

    for (int i = 0; noses[i] != 0; i++)
        if (noses[i] == nose)
            return noses[i];

    return 0;

}

const char *say_verb(const char *word, Character *ch, Character *viewer,
                     int S)
{

    size_t len = strlen(word), i, j;

#define sm(a, b, c) return S == 0 ? a : S == 1 ? b : c

    if ((viewer && viewer->pc
            && is_set(viewer->flags, PLR_NOEMOTEVERBS)) || len < 3)
        sm("say", "says", "said");

    if (ch)
    {

        if (is_drunk(ch))
            sm("slur", "slurs", "slured");

        /*
         * else if (ch && IS_DEAD(ch)) sm("wail", "wails", "wailed");
         */
    }
    j = i = len - 2;

    if (is_nose(word[i]))
        i = len - 3;

    if (is_eye(word[i]))
    {

        switch (word[len - 1])
        {

        case ')':

            if (word[i] != ';')
                sm("smile", "smiles", "smiled");

            else
                sm("leer", "leers", "leered");

        case '}':

        case ']':

        case '>':

            sm("grin", "grins", "grinned");

        case '(':

        case '{':

        case '[':

        case '<':

            if (word[j] == '\'')
                sm("sob", "sobs", "sobbed");

            else
                sm("sulk", "sulks", "sulked");

        case '|':

        case '/':

        case '\\':

            sm("stare", "stares", "stared");

        case 'P':

        case 'p':

            sm("smirk", "smirks", "smirked");

        case 'o':

        case 'O':

            sm("sing", "sings", "sung");

        case '$':

            sm("blush", "blushes", "blushed");

        case 's':

        case 'S':

            sm("blab", "blabs", "blabbed");

        case 'D':

            sm("beam", "beams", "beamed");

        case '@':

            sm("shout", "shouts", "shouted");

        }

    }
    else
    {

        switch (word[len - 1])
        {

        case '!':

            if (word[len - 2] != '!')
                sm("exclaim", "exclaims", "exclaimed");

            else
                sm("scream", "screams", "screamed");

        case '?':

            if (word[len - 2] == '!')
                sm("boggle", "boggles", "boggled");

            else if (word[len - 2] != '?')
                sm("ask", "asks", "asked");

            else
                sm("demand", "demands", "demanded");

        case '.':

            if (word[len - 2] == '.' && word[len - 3] == '.')
                sm("mutter", "mutters", "muttered");

            break;

        }

    }

    sm("say", "says", "said");

}

const char *get_chan_soc_string(Character *ch, Character *victim,
                                Character *vch, Object *obj, Social *soc)
{
    if (!victim && !obj)
    {
        if (ch == vch)
            return soc->charNoArg;
        else
            return soc->othersNoArg;
    }
    else if (obj && !victim)
    {
        if (ch == vch)
            return soc->charObjFound;
        else
            return soc->othersObjFound;
    }
    else
    {
        if (victim == ch)
        {
            if (vch == ch)
                return soc->charAuto;
            else
                return soc->othersAuto;
        }
        else
        {
            if (victim == vch)
                return soc->victFound;
            else if (ch == vch)
                return soc->charFound;
            else
                return soc->othersFound;
        }
    }
}

void channel_social(Character *ch, Character *victim, Object *obj,
                    Social *soc, const Channel *chan)
{
    Character *pch;
    const char *type;

    type = format_channel(chan, ch);

    for (pch = first_player; pch; pch = pch->next_player)
    {
        if (pch == ch || pch == victim
                || channel_viewable(ch, pch, chan))
        {
            const char *string = get_chan_soc_string(ch,
                                 victim, pch, obj, soc);

            char msg[BUFSIZ + 1] = {0};

            snprintf(msg, BUFSIZ, "%s %s", type, string);

            act(TO_VICT | TO_WORLD, ch, obj, victim, msg);
            act(TO_NOTVICT | TO_WORLD, ch, obj, pch, msg);
            act(TO_CHAR | TO_WORLD, ch, obj, victim, msg);

        }
    }
}

int interpret_channel(Character *ch, int gcn, const char *argument)
{

    const Channel *chan = &channel_table[gcn];
    const char *arg_left;
    char command[ARG_SIZ];
    channel_arg_t arg_type = CHANNEL_NORMAL;

    if (chan == 0)
        return 0;

    if (nullstr(argument))
    {

        if (!ch->pc)
            return 1;

        if (is_set(ch->pc->channels, chan->bit))
        {

            remove_bit(ch->pc->channels, chan->bit);

            writelnf(ch, "%s channel off.", capitalize(chan->name));

        }
        else
        {

            set_bit(ch->pc->channels, chan->bit);

            writelnf(ch, "%s channel on.", capitalize(chan->name));

        }

        return 1;

    }
    arg_left = one_argument(argument, command);

    const char *format = format_channel(chan, ch);

    if (command[0] == '+')
    {
        Character *victim;
        Object *obj;
        Social *soc;
        char argx[ARG_SIZ];

        arg_left = one_argument(arg_left, command);

        if (nullstr(command))
        {
            writelnf(ch,
                     "{W%s + <social> is used for channel based socials.{x",
                     chan->name);
            return 1;
        }
        soc = social_lookup(command);

        if (!soc)
        {
            writeln(ch, "{WWhat kind of social is that?!?!{x");
            return 1;
        }
        one_argument(arg_left, argx);
        victim = 0;
        if (nullstr(argx))
        {
            channel_social(ch, 0, 0, soc, chan);
        }
        else if ((victim = get_char_world(ch, argx)) == 0)
        {
            if (!nullstr(soc->othersObjFound)
                    && ch->inRoom
                    &&
                    ((obj = get_obj_list(ch, argx, ch->inRoom->objects))
                     || (obj = get_obj_carry(ch, argx, ch))))
            {
                channel_social(ch, 0, obj, soc, chan);
            }
            else
            {
                writeln(ch, "They aren't here.");
            }
        }
        else if (victim)
        {
            if (victim != ch && !channel_viewable(ch, victim, chan))
            {
                writeln(ch, "They can't use that channel.");

            }
            else
                channel_social(ch, victim, NULL, soc, chan);
        }
        return 1;
    }
    else if (command[0] == '!')
    {
        if (nullstr(argument))
        {
            writelnf(ch, "Syntax: %s ! <argument>", chan->name);
            return 1;
        }
        arg_type = CHANNEL_EMOTE;
        writelnf(ch, "%s %s %s{x", format, ch->name, argument);
    }
    else if (command[0] == '@')
    {
        if (nullstr(argument))
        {
            writelnf(ch, "Syntax: %s @ <argument>", chan->name);
            return 1;
        }
        arg_type = CHANNEL_THINK;
        writelnf(ch, "%s %s . o O ( %s ){x", format, ch->name,
                 argument);
    }
    else if (is_exact_name(command, "-who -w -wholist")
             && nullstr(argument))
    {
        arg_type = CHANNEL_WHO;
        writelnf(ch, "{WPlayers on %s{x", format);
        writeln(ch, "{C-------------------{x");
    }
    else if (is_exact_name(command, "-h -help") && nullstr(argument))
    {
        writelnf(ch, "Syntax: %s <message>          - send a message",
                 chan->name);
        if (chan->bit)
            writelnf(ch,
                     "      : %s                    - toggle channel on/off",
                     chan->name);
        writelnf(ch,
                 "      : %s -hist              - display channel history",
                 chan->name);
        writelnf(ch,
                 "      : %s -who               - display who is on channel",
                 chan->name);
        writelnf(ch,
                 "      : %s ! <emote>          - send an emote over channel",
                 chan->name);
        writelnf(ch,
                 "      : %s + <social> [args]  - do a social over channel",
                 chan->name);
        writelnf(ch,
                 "      : %s @ <message>        - enclose a message in 'thought bubbles'",
                 chan->name);
        writelnf(ch, "      : %s -help              - this message",
                 chan->name);
        return 1;
    }
    else
    {

        writelnf(ch, "%s You %s '%s'~x", format,
                 say_verb(argument, ch, 0, 0), argument);

        for (Character *pch = first_player; pch != 0;
                pch = pch->next_player)
        {

            if (ch == pch || !channel_viewable(ch, pch, chan))
                continue;

            switch (arg_type)
            {
            case CHANNEL_NORMAL:
                writelnf(pch, "%s %s %s '%s'~x", format,
                         chview(ch, pch), say_verb(argument, ch,
                                                   pch, 1),
                         argument);
                break;
            default:
                break;
            }

        }

    }
    return 1;

}

void announce(Character *ch, info_t type, const char *message, ...)
{

    char buf[BUF_SIZ], buf2[BUF_SIZ];

    const char *iType;

    bool Private = false;

    va_list args;

    if (nullstr(message))
        return;

    va_start(args, message);

    vsnprintf(buf2, sizeof(buf2), message, args);

    va_end(args);

    if (ch && (type & INFO_PRIVATE))
    {

        Private = true;

        type &= ~INFO_PRIVATE;

        if (ch == 0)
        {

            log_bug("null ch in private announce");

            return;

        }
    }
    switch (type)
    {

    default:

        iType = "~RINFO";
        break;

    }

    sprintf(buf, "%s~W:~x %s~x", iType, buf2);

    for (Character *p = first_player; p; p = p->next_player)
    {

        if (!p->pc->conn->is_playing(ch->pc->conn))
            continue;

        if (Private && ch != p)
            continue;

        switch (type)
        {

        case INFO_NOTE:

            if (ch && !is_note_to(p, ch->pc->account->inProgress))
                continue;

            break;

        default:

            break;

        }

        if (ch == NULL)
            writeln(p, buf);

        else
            act_pos(TO_VICT, POS_DEAD, ch, 0, p, buf);

    }

}
