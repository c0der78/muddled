
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

#include <ctype.h>
#include <stdarg.h>
#include "client.h"
#include "../engine.h"
#include "../exit.h"
#include "../character.h"
#include "../room.h"
#include "../connection.h"
#include "../account.h"
#include "../player.h"
#include "../string.h"
#include "../gsn.h"
#include "../fight.h"
#include "../channel.h"
#include "../log.h"
#include "../skill.h"
#include "../util.h"
#include "../object.h"
#include "../string.h"
#include "../social.h"
#include "command.h"

const Command cmd_table[] =
{
    {"north", 0, do_north, COMMAND_MANIP, POS_STANDING},
    {"east", 0, do_east, COMMAND_MANIP, POS_STANDING},
    {"south", 0, do_south, COMMAND_MANIP, POS_STANDING},
    {"west", 0, do_west, COMMAND_MANIP, POS_STANDING},
    {"up", 0, do_up, COMMAND_MANIP, POS_STANDING},
    {"down", 0, do_down, COMMAND_MANIP, POS_STANDING},
    {"look", 0, do_look, COMMAND_INFO, POS_RESTING},
    {"quit", 0, do_quit, COMMAND_CONN, POS_DEAD},
    {"logout", 0, do_logout, COMMAND_CONN, POS_DEAD},
    {"commands", 0, do_commands, COMMAND_INFO, POS_DEAD},
    {"shutdown", LEVEL_IMMORTAL, do_shutdown, COMMAND_ADMIN, POS_DEAD},
    {"reboot", LEVEL_IMMORTAL, do_reboot, COMMAND_ADMIN, POS_DEAD},
    {"save", 0, do_save, COMMAND_SETTINGS, POS_DEAD},
    {"edit", LEVEL_IMMORTAL, do_edit, COMMAND_ADMIN, POS_DEAD},
    {"color", 0, do_color, COMMAND_SETTINGS, POS_DEAD},
    {"time", 0, do_time, COMMAND_INFO, POS_DEAD},
    {"title", 0, do_title, COMMAND_SETTINGS, POS_DEAD},
    {"who", 0, do_who, COMMAND_INFO, POS_DEAD},
    {"score", 0, do_score, COMMAND_INFO, POS_DEAD},
    {"autotick", 0, do_autotick, COMMAND_SETTINGS, POS_DEAD},
    {"brief", 0, do_brief, COMMAND_SETTINGS, POS_DEAD},
    {"say", 0, do_say, COMMAND_COMM, POS_RESTING},
    {"timezone", 0, do_timezone, COMMAND_SETTINGS, POS_DEAD},
    {"hints", 0, do_hints, COMMAND_SETTINGS, POS_DEAD},
    {"logoff", 0, do_logout, COMMAND_CONN, POS_DEAD},
    {"delete", 0, do_delete, COMMAND_SETTINGS, POS_DEAD},
    {"kill", 0, do_kill, COMMAND_COMBAT, POS_STANDING},
    {"map", 0, do_map, COMMAND_INFO, POS_RESTING},
    {"explored", 0, do_explored, COMMAND_INFO, POS_DEAD},
    {"kick", 0, do_kick, COMMAND_COMBAT, POS_STANDING},
    {"cast", 0, do_cast, COMMAND_COMBAT, POS_RESTING},
    {"automap", 0, do_automap, COMMAND_SETTINGS, POS_DEAD},
    {"prompt", 0, do_prompt, COMMAND_SETTINGS, POS_DEAD},
    {"chat", 0, do_chat, COMMAND_COMM, POS_SLEEPING},
    {"admin", LEVEL_IMMORTAL, do_admin, COMMAND_ADMIN | COMMAND_COMM, POS_DEAD},
    {"autologin", 0, do_autologin, COMMAND_CONN, POS_DEAD},
    {"get", 0, do_get, COMMAND_MANIP, POS_RESTING},
    {"inventory", 0, do_inventory, COMMAND_INFO, POS_DEAD},
    {"equipment", 0, do_equipment, COMMAND_INFO, POS_DEAD},
    {"drop", 0, do_drop, COMMAND_MANIP, POS_RESTING},
    {"wear", 0, do_wear, COMMAND_MANIP, POS_RESTING},
    {"forum", 0, do_forum, COMMAND_COMM, POS_DEAD},
    {"note", 0, do_note, COMMAND_COMM, POS_DEAD},
    {"subscribe", 0, do_subscribe, COMMAND_SETTINGS, POS_DEAD},
    {"think", 0, do_think, COMMAND_COMM, POS_RESTING},
    {"whisper", 0, do_whisper, COMMAND_COMM, POS_RESTING},
    {"shout", 0, do_shout, COMMAND_COMM, POS_RESTING},
    {"affects", 0, do_affects, COMMAND_INFO, POS_DEAD},
    {"force", LEVEL_IMMORTAL, do_force, COMMAND_ADMIN, POS_DEAD},
    {"resists", 0, do_resists, COMMAND_INFO, POS_DEAD},
    {"sockets", LEVEL_IMMORTAL, do_sockets, COMMAND_ADMIN, POS_DEAD},
    {"goto", LEVEL_IMMORTAL, do_goto, COMMAND_ADMIN, POS_DEAD},
    {"socials", 0, do_socials, COMMAND_INFO, POS_DEAD},
    {"db", LEVEL_IMMORTAL, do_db, COMMAND_ADMIN, POS_DEAD},
    {"import", LEVEL_IMMORTAL, do_import, COMMAND_ADMIN, POS_DEAD},
    {"help", 0, do_help, COMMAND_INFO, POS_DEAD},
    {"areas", 0, do_areas, COMMAND_INFO, POS_DEAD},
    {0, 0, 0, 0}
};

int max_command = (sizeof(cmd_table) / sizeof(cmd_table[0]) - 1);

const Lookup command_types[] =
{
    {"info", COMMAND_INFO},
    {"settings", COMMAND_SETTINGS},
    {"admin", COMMAND_ADMIN},
    {"manipulation", COMMAND_MANIP},
    {"connection", COMMAND_CONN},
    {"communication", COMMAND_COMM},
    {"combat", COMMAND_COMBAT},
    {0, 0}
};

void command_interpret(Character *ch, const char *argument)
{
    char arg[500];
    bool found;
    const Command *cmd;

    /*
     * Strip leading spaces.
     */
    while (isspace((int)*argument))
        argument++;
    if (argument[0] == '\0')
        return;

    argument = one_argument(argument, arg);

    /*
     * Look for command in command table.
     */
    found = false;
    for (cmd = cmd_table; cmd->name != 0; cmd++)
    {
        if (UPPER(arg[0]) == UPPER(cmd->name[0])
                && !str_prefix(arg, cmd->name) && cmd->level <= ch->level)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {

        if (interpret_social(ch, arg, argument))
            return;

        const char *const message[] =
        {
            "Huh?",
            "Pardon?",
            "Excuse me?",
            "What?",
            "Say again.",
            "Eh?",
            "Can you repeat that?",
            "You want what?",
            "Ummm...",
            "Syntax Error",
            "Come again?",
            "Bad command or file name.",
            "I do not understand..."
        };

        writeln(ch,
                message[number_range
                        (0,
                         (sizeof(message) / sizeof(message[0])) - 1)]);
        return;
    }
    if (cmd->position < ch->position)
    {
        writelnf(ch, "You can't do that while you are %s.",
                 position_table[ch->position].name);
        return;
    }
    (*cmd->dofun) (cmd->name, ch, argument);
}

void cmd_syntax(Character *ch, const char *n_fun, ...)
{
    va_list args;
    char *str;
    size_t i;
    char *title;

    if (!n_fun || !*n_fun)
        return;

    va_start(args, n_fun);

    str = va_arg(args, char *);

    if (str == NULL)
        return;

    switch (number_range(1, 2))
    {
    default:
    case 1:
        title = "Syntax";
        break;
    case 2:
        title = "Usage";
        break;
    }

    i = strlen(title) + 1;

    writelnf(ch, "~W%s: ~w%s %s~x", title, n_fun, str);

    while ((str = va_arg(args, char *)) != NULL)

        writelnf(ch, "~W%*c ~w%s %s~x", i, ' ', n_fun, str);

    va_end(args);
}

DOFUN(quit)
{
    if (ch->pc == 0)
        return;

    const char *const chmessage[] =
    {
        "Alas, all good things must come to an end.",
        "This concludes our broadcast of the emergency mudding system.",
        "We await your return friend.",
        "What???? You're not addicted yet?????",
        "~RYou have been KILLED!!!~x",
        "Today, Tomorrow, Yesterday...on the mud, they are all the same.",
        formatf("Thank you for playing %s.", engine_info.name),
        "It's not a bug, it's a feature!!!",
        "%s boots you in the head on your way out.  OuCH!!!"
    };

    const char *const wmessage[] =
    {
        "$n has left the realms.",
        "$n retires from the realm.",
        "$n has returned to real life!",
    };

    long number =
        number_range(1, sizeof(chmessage) / sizeof(chmessage[0])) - 1;
    writeln(ch, chmessage[number]);

    number = number_range(1, sizeof(wmessage) / sizeof(wmessage[0])) - 1;

    announce(ch, INFO_LEAVE, "%s", wmessage[number]);
    // extract_char(ch, true);

    ch->pc->conn->handler = 0;
}

DOFUN(logout)
{
    if (ch->pc == 0)
        return;

    char_from_room(ch);

    UNLINK(first_character, Character, ch, next);
    UNLINK(first_player, Character, ch, next_player);

    client_display_account_menu((Client *) ch->pc->conn);

    ch->pc->conn->handler =
        (void ( *)(Connection *, const char *))client_account_menu;
}

DOFUN(autologin)
{
    if (ch->pc == 0)
        return;

    AccountPlayer *p;
    int count = 0;

    if (nullstr(argument))
    {
        for (p = ch->pc->account->players; p; p = p->next)
        {
            writelnf(ch, "%s~Y%2d)~C %s",
                     p->charId ==
                     ch->pc->account->autologinId ? "~R*" : " ",
                     ++count, p->name);
        }
        writelnf(ch, "~WSyntax: %s <# or name>~x", do_name);
        return;
    }
    for (p = ch->pc->account->players; p; p = p->next)
    {
        if (atoi(argument) == ++count || !str_prefix(argument, p->name))
        {
            break;
        }
    }

    if (p == 0)
    {
        do_autologin(do_name, ch, str_empty);
        return;
    }
    ch->pc->account->autologinId = p->charId;

    writelnf(ch, "Autologin set to %s.", p->name);
}

void move_char(Character *ch, direction_t dir)
{
    if (ch->inRoom == 0)
    {
        log_error("character with no room");
        return;
    }
    Exit *ex = ch->inRoom->exits[dir];

    if (ex == 0 || ex->to.room == 0)
    {
        writeln(ch, "You can't move in that direction.");
        return;
    }
    char_from_room(ch);

    char_to_room(ch, ex->to.room);

    do_look(str_empty, ch, str_empty);
}

DOFUN(north)
{
    move_char(ch, DIR_NORTH);
}

DOFUN(east)
{
    move_char(ch, DIR_EAST);
}

DOFUN(south)
{
    move_char(ch, DIR_SOUTH);
}

DOFUN(west)
{
    move_char(ch, DIR_WEST);
}

DOFUN(up)
{
    move_char(ch, DIR_UP);
}

DOFUN(down)
{
    move_char(ch, DIR_DOWN);
}

DOFUN(kill)
{
    Character *victim = get_char_room(ch, argument);

    if (victim == 0)
    {
        writeln(ch, "They are not here.");
        return;
    }
    victim->fighting = ch;
    ch->fighting = victim;

    multi_hit(ch, victim, GSN_UNDEFINED, DAM_UNDEFINED);
}

DOFUN(kick)
{
    if (ch->fighting == 0)
    {
        writeln(ch, "Your not fighting anyone!");
        return;
    }
    multi_hit(ch, ch->fighting, gsn_kick, DAM_BASH);
}

DOFUN(cast)
{
    int sn;

    for (sn = 0; sn < max_skill; sn++)
    {
        if (skill_table[sn].spellfun == 0)
            continue;

        if (!str_prefix(argument, skill_table[sn].name))
        {
            break;
        }
    }

    if (sn >= max_skill)
    {
        writeln(ch, "No such spell!");
        return;
    }
    (*skill_table[sn].spellfun) (sn, ch);
}

DOFUN(get)
{
    if (nullstr(argument))
    {
        writeln(ch, "Get what?");
        return;
    }
    Object *obj = get_obj_list(ch, argument, ch->inRoom->objects);

    if (obj == 0)
    {
        writeln(ch, "That is not here.");
        return;
    }
    obj_from_room(obj);

    obj_to_char(obj, ch);

    act(TO_ROOM, ch, obj, 0, "$n gets $p.");
    act(TO_CHAR, ch, obj, 0, "You get $p.");
}

DOFUN(drop)
{
    if (nullstr(argument))
    {
        writeln(ch, "Drop what?");
        return;
    }
    Object *obj = get_obj_list(ch, argument, ch->carrying);

    if (obj == 0)
    {
        writeln(ch, "You are not carrying that.");
        return;
    }
    obj_from_char(obj);

    obj_to_room(obj, ch->inRoom);

    act(TO_CHAR, ch, obj, 0, "You drop $p.");
    act(TO_ROOM, ch, obj, 0, "$n drops $p.");
}

bool remove_obj(Character *ch, int iWear, bool fReplace)
{
    Object *obj;

    if ((obj = get_eq_char(ch, iWear)) == NULL)
        return true;

    if (!fReplace)
        return false;

    unequip_char(ch, obj);
    act(TO_ROOM, ch, obj, 0, "$n stops using $p.");
    act(TO_CHAR, ch, obj, 0, "You stop using $p.");
    return true;
}

void wear_obj(Character *ch, Object *obj, bool fReplace)
{
    if (ch->level < obj->level)
    {
        writelnf(ch, "You must be level %d to use this object.",
                 obj->level);

        act(TO_ROOM, ch, obj, 0,
            "$n tries to use $p, but is too inexperienced.");
        return;
    }
    const wear_table_type *t = wear_table;

    while (t->display != 0)
    {
        if (t->flags == obj->wearFlags)
            break;
        t++;
    }

    if (t != 0 && t->canUse != 0 && !(*t->canUse) (ch))
    {
        writeln(ch, "You do not know how to wear that object.");
        act(TO_ROOM, ch, obj, 0,
            "$n tries to use $p, but doesn't know how.");
        return;
    }
    switch (obj->wearFlags)
    {
    case WEAR_HEAD:
        if (!remove_obj(ch, WEAR_HEAD, fReplace))
            return;
        act(TO_ROOM, ch, obj, 0, "$n wears $p on $s head.");
        act(TO_CHAR, ch, obj, 0, "You wear $p on your head.");
        equip_char(ch, obj, WEAR_HEAD);
        return;

    case WEAR_NECK:
        if (!remove_obj(ch, WEAR_NECK, fReplace))
            return;
        act(TO_ROOM, ch, obj, 0, "$n wears $p around $s neck.");
        act(TO_CHAR, ch, obj, 0, "You wear $p around your neck.");
        equip_char(ch, obj, WEAR_NECK);
        return;

    case WEAR_EYES:
        if (!remove_obj(ch, WEAR_EYES, fReplace))
            return;
        act(TO_ROOM, ch, obj, 0, "$n wears $p over $s eyes.");
        act(TO_CHAR, ch, obj, 0, "You wear $p over your eyes.");
        equip_char(ch, obj, WEAR_EYES);
        return;

    case WEAR_SHOULDERS:
        if (!remove_obj(ch, WEAR_SHOULDERS, fReplace))
            return;
        act(TO_ROOM, ch, obj, 0, "$n wears $p on $s shoulders.");
        act(TO_CHAR, ch, obj, 0, "You wear $p on your shoulders.");
        equip_char(ch, obj, WEAR_SHOULDERS);
        return;
    case WEAR_BACK:
        if (!remove_obj(ch, WEAR_BACK, fReplace))
            return;
        act(TO_ROOM, ch, obj, 0, "$n wears $p on $s back.");
        act(TO_CHAR, ch, obj, 0, "You wear $p on your back.");
        equip_char(ch, obj, WEAR_BACK);
        return;
    case WEAR_ARMS:
        if (!remove_obj(ch, WEAR_ARMS, fReplace))
            return;
        act(TO_ROOM, ch, obj, 0, "$n wears $p on $s arms.");
        act(TO_CHAR, ch, obj, 0, "You wear $p on your arms.");
        equip_char(ch, obj, WEAR_ARMS);
        return;
    case WEAR_HANDS:
        if (!remove_obj(ch, WEAR_HANDS, fReplace))
            return;
        act(TO_ROOM, ch, obj, 0, "$n wears $p on $s hands.");
        act(TO_CHAR, ch, obj, 0, "You wear $p on your hands.");
        equip_char(ch, obj, WEAR_HANDS);
        return;
    case WEAR_WRIST:
        if (get_eq_char(ch, WEAR_WRIST) != 0 &&
                get_eq_char(ch, WEAR_WRIST_2) != 0 &&
                !remove_obj(ch, WEAR_WRIST, fReplace) &&
                !remove_obj(ch, WEAR_WRIST_2, fReplace))
            return;
        if (get_eq_char(ch, WEAR_WRIST) == 0)
        {
            act(TO_ROOM, ch, obj, 0,
                "$n wears $p on $s left wrist.");
            act(TO_CHAR, ch, obj, 0,
                "You wear $p on your left wrist.");
            equip_char(ch, obj, WEAR_WRIST);
            return;
        }
        if (get_eq_char(ch, WEAR_WRIST_2) == 0)
        {
            act(TO_ROOM, ch, obj, 0,
                "$n wears $p on $s right wrist.");
            act(TO_CHAR, ch, obj, 0,
                "You wear $p on your right wrist.");
            equip_char(ch, obj, WEAR_WRIST_2);
            return;
        }
        writeln(ch,
                "You are already wearing something on your wrists.");
        return;
    case WEAR_FINGER:
        if (get_eq_char(ch, WEAR_FINGER) != 0 &&
                get_eq_char(ch, WEAR_FINGER_2) != 0 &&
                !remove_obj(ch, WEAR_FINGER, fReplace) &&
                !remove_obj(ch, WEAR_FINGER_2, fReplace))
            return;
        if (get_eq_char(ch, WEAR_FINGER) == 0)
        {
            act(TO_ROOM, ch, obj, 0,
                "$n wears $p on $s left finger.");
            act(TO_CHAR, ch, obj, 0,
                "You wear $p on your left finger.");
            equip_char(ch, obj, WEAR_FINGER);
            return;
        }
        if (get_eq_char(ch, WEAR_FINGER_2) == 0)
        {
            act(TO_ROOM, ch, obj, 0,
                "$n wears $p on $s right finger.");
            act(TO_CHAR, ch, obj, 0,
                "You wear $p on your right finger.");
            equip_char(ch, obj, WEAR_FINGER_2);
            return;
        }
        writeln(ch, "You are already wearing two rings.");
        return;
    case WEAR_TORSO:
        if (!remove_obj(ch, WEAR_TORSO, fReplace))
            return;
        act(TO_ROOM, ch, obj, 0, "$n wears $p on $s torso.");
        act(TO_CHAR, ch, obj, 0, "You wear $p on your torso.");
        equip_char(ch, obj, WEAR_TORSO);
        return;
    case WEAR_WAIST:
        if (!remove_obj(ch, WEAR_WAIST, fReplace))
            return;
        act(TO_ROOM, ch, obj, 0, "$n wears $p around $s waist.");
        act(TO_CHAR, ch, obj, 0, "You wear $p around your waist.");
        equip_char(ch, obj, WEAR_WAIST);
        return;
    case WEAR_LEGS:
        if (!remove_obj(ch, WEAR_LEGS, fReplace))
            return;
        act(TO_ROOM, ch, obj, 0, "$n wears $p on $s legs.");
        act(TO_CHAR, ch, obj, 0, "You wear $p on your legs.");
        equip_char(ch, obj, WEAR_LEGS);
        return;
    case WEAR_FEET:
        if (!remove_obj(ch, WEAR_FEET, fReplace))
            return;
        act(TO_ROOM, ch, obj, 0, "$n wears $p on $s feet.");
        act(TO_CHAR, ch, obj, 0, "You wear $p on your feet.");
        equip_char(ch, obj, WEAR_FEET);
        return;
    case WEAR_ABOUT:
        if (!remove_obj(ch, WEAR_ABOUT, fReplace))
            return;
        act(TO_ROOM, ch, obj, 0, "$n wears $p about $s body.");
        act(TO_CHAR, ch, obj, 0, "You wear $p about your body.");
        equip_char(ch, obj, WEAR_ABOUT);
        return;
    case WEAR_WIELD:
        if (!remove_obj(ch, WEAR_WIELD, fReplace))
            return;
        act(TO_ROOM, ch, obj, 0, "$n wields $p.");
        act(TO_CHAR, ch, obj, 0, "You wield $p.");
        equip_char(ch, obj, WEAR_WIELD);
        return;
    case WEAR_DUAL:
        if (!remove_obj(ch, WEAR_DUAL, fReplace))
            return;
        act(TO_ROOM, ch, obj, 0, "$n wields $p in $s off hand.");
        act(TO_CHAR, ch, obj, 0, "You wield $p in your off hand.");
        equip_char(ch, obj, WEAR_DUAL);
        return;
    case WEAR_SHIELD:
        if (!remove_obj(ch, WEAR_SHIELD, fReplace))
            return;
        act(TO_ROOM, ch, obj, 0, "$n usees $p as $s shield.");
        act(TO_CHAR, ch, obj, 0, "You use $p as your shield.");
        equip_char(ch, obj, WEAR_SHIELD);
        return;
    case WEAR_LIGHT:
        if (!remove_obj(ch, WEAR_LIGHT, fReplace))
            return;
        act(TO_ROOM, ch, obj, 0, "$n lights $p and holds it.");
        act(TO_CHAR, ch, obj, 0, "You light $p and hold it.");
        equip_char(ch, obj, WEAR_LIGHT);
        return;
    case WEAR_FLOAT:
        if (!remove_obj(ch, WEAR_FLOAT, fReplace))
            return;

        act(TO_ROOM, ch, obj, 0, "$n releases $p to float near $m.");
        act(TO_CHAR, ch, obj, 0, "You release $p to float nearby.");
        equip_char(ch, obj, WEAR_FLOAT);

        return;
    default:
        if (fReplace)
            writeln(ch, "You can't wear, wield, or hold that.");
        return;
    }
}

DOFUN(wear)
{
    char arg[BUF_SIZ];
    Object *obj;

    one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        writeln(ch, "Wear, wield, or hold what?");
        return;
    }
    if (!str_cmp(arg, "all"))
    {
        Object *obj_next;

        for (obj = ch->carrying; obj != NULL; obj = obj_next)
        {
            obj_next = obj->next_content;
            if (obj->wearLoc == WEAR_NONE && can_see_obj(ch, obj))
                wear_obj(ch, obj, false);
        }
        return;
    }
    else
    {
        if ((obj = get_obj_carry(ch, arg, ch)) == NULL)
        {
            writeln(ch, "You do not have that item.");
            return;
        }
        wear_obj(ch, obj, true);
    }
}
