
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
#include <string.h>
#include <ctype.h>
#include "engine.h"
#include "str.h"
#include "character.h"
#include "area.h"
#include "player.h"
#include "log.h"
#include "flag.h"
#include "room.h"
#include "area.h"
#include "explored.h"
#include "account.h"
#include "class.h"
#include "connection.h"
#include "race.h"
#include "nonplayer.h"
#include "object.h"
#include "util.h"
#include "help.h"
#include "map.h"
#include "grid.h"
#include "buffer.h"
#include "affect.h"
#include "skill.h"
#include "lookup.h"
#include "social.h"
#include "private.h"
#include "update.h"
#include "server.h"
#include "command.h"

int compare_commands(const void *a, const void *b)
{
    Command cmd1 = *(Command *) a;
    Command cmd2 = *(Command *) b;
    return strcmp(cmd1.name, cmd2.name);
}


DOFUN(commands)
{

    if (!str_prefix(argument, "help"))
    {
        xwritelnf(ch, "%s all 		- list all commands", do_name);

        for (const Lookup *t = command_types; t->name != 0; t++)
            xwritelnf(ch, "%s %-13s	- list all %s commands",

                      do_name, t->name, t->name);
        return;
    }
    int type = index_lookup(command_types, argument);
    int count = 0;
    Command cmdlist[max_command];

    for (int i = 0; i < max_command; i++) {
        cmdlist[i] = cmd_table[i];
    }
    qsort(cmdlist, max_command, sizeof(Command), compare_commands);
    ch->titlef(ch, "%s Commands",
               type == -1 ? "All" : capitalize(command_types[type].name));
    int len = scrwidth(ch) / 5 - 1;

    for (int i = 0; i < max_command; i++)
    {

        if (cmdlist[i].level > ch->level) {
            continue;
        }

        if (type != -1
                && !(cmdlist[i].category & command_types[type].value)) {
            continue;
        }
        xwritef(ch, "%-*s ", len, cmdlist[i].name);

        if (++count % 5 == 0)
        {
            xwriteln(ch, str_empty);
        }
    }

    if (count % 5 != 0) {
        xwriteln(ch, str_empty);
    }

    if (type == -1) {
        xwritelnf(ch, "See '%s help' for more options.", do_name);
    }

}

void show_char_to_char_1(Character *victim, Character *ch)
{
    char buf[BUF_SIZ];
    Object *obj;
    int iWear;
    int percent;
    bool found;

    if (can_see(victim, ch))
    {

        if (ch == victim) {
            act(TO_ROOM, ch, 0, 0, "$n looks at $mself.");
        }

        else
        {
            act(TO_VICT, ch, 0, victim, "$n looks at you.");
            act(TO_NOTVICT, ch, 0, victim, "$n looks at $N.");
        }
    }

    if (!nullstr(victim->description))
    {
        xwriteln(ch, victim->description);
    }

    else
    {
        act(TO_CHAR, ch, 0, victim,
            "You see nothing special about $M.");
    }

    if (victim->maxHit > 0) {
        percent = (int)percent(victim->hit, victim->maxHit);
    }

    else {
        percent = -1;
    }
    strcpy(buf, chview(victim, ch));

    if (percent >= 100) {
        strcat(buf, " is in excellent condition.");
    }

    else if (percent >= 90) {
        strcat(buf, " has a few scratches.");
    }

    else if (percent >= 75) {
        strcat(buf, " has some small wounds and bruises.");
    }

    else if (percent >= 50) {
        strcat(buf, " has quite a few wounds.");
    }

    else if (percent >= 30) {
        strcat(buf, " has some big nasty wounds and scratches.");
    }

    else if (percent >= 15) {
        strcat(buf, " looks pretty hurt.");
    }

    else if (percent >= 0) {
        strcat(buf, " is in awful condition.");
    }

    else {
        strcat(buf, " is bleeding to death.");
    }
    buf[0] = UPPER(buf[0]);
    xwriteln(ch, buf);
    found = false;

    for (iWear = 0; wear_table[iWear].display != 0; iWear++)
    {

        if ((obj =
                    get_eq_char(victim,
                                wear_table[iWear].loc)) !=
                NULL && can_see_obj(ch, obj))
        {

            if (!found)
            {
                xwriteln(ch, "");
                act(TO_CHAR, ch, 0, victim, "$N is using:");
                found = true;
            }
            xwritef(ch, "~g<~w%s~g>~w ",
                    align_string(ALIGN_CENTER, 22, 0, 0,
                                 wear_table[iWear].display));
            xwriteln(ch, format_obj_to_char(obj, ch, true));
        }
    }

    if (victim != ch && is_immortal(ch))
    {
        xwriteln(ch, "");
        xwriteln(ch, "You peek at the inventory:");
        show_list_to_char(victim->carrying, ch, true, true);
    }
    return;

}


DOFUN(look)
{

    if (nullstr(argument))
    {

        if (ch->inRoom == 0)
        {
            log_warn("character with no room");
            return;
        }
        xwritelnf(ch, "~G%s~x", ch->inRoom->name);

        if (!ch->pc || !is_set(ch->pc->account->flags, PLR_BRIEF))
        {

            if (!ch->pc
                    || is_set(ch->pc->account->flags, PLR_AUTOMAP_OFF)) {
                xwritelnf(ch, "~Y%s~x", ch->inRoom->description);
            }

            else {
                draw_map(ch, ch->inRoom->description);
            }
        }
        xwrite(ch, "~g[Exits: ");
        char buf[100] = { 0 };

        for (const Lookup *t = direction_table; t->name != 0; t++)
        {

            if (ch->inRoom->exits[t->value] == 0) {
                continue;
            }
            strcat(buf, t->name);
            strcat(buf, " ");
        }

        if (buf[0] != 0) {
            buf[strlen(buf) - 1] = 0;
        }

        else {
            strcpy(buf, "None");
        }
        xwritelnf(ch, "%s]", buf);
        xwriteln(ch, "~x");

        for (Character *rch = ch->inRoom->characters; rch != 0;
                rch = rch->next_in_room)
        {

            if (rch == ch) {
                continue;
            }

            if (!rch->npc)
            {
                xwritelnf(ch, "~M%s is here.~x", rch->name);
            }

            else
            {
                xwritelnf(ch, "~M%s~x", rch->npc->longDescr);
            }
        }

        for (Object *obj = ch->inRoom->objects; obj != 0;
                obj = obj->next_content)
        {
            xwritelnf(ch, "~Y%s~x", obj->longDescr);
        }
        return;
    }
    Character *victim;

    if ((victim = get_char_room(ch, argument)) != NULL)
    {
        show_char_to_char_1(victim, ch);
        return;
    }
}


DOFUN(time)
{
    time_t t = time(0);
    struct tm *tm = localtime(&t);
    xwritelnf(ch, "Today is %s, %s day in the Month of the %s, year %d.",
              weekdays[tm->tm_wday], ordinal_string(tm->tm_wday),
              months[tm->tm_mon], tm->tm_year);
    xwritelnf(ch, "It is the season of %s.", seasons[tm->tm_mon / 3]);
    xwriteln(ch, "");

    if (last_reboot != 0)
        xwritelnf(ch, "Last reboot was on %s.",
                  str_time(last_reboot, ch->pc->account->timezone, 0));

    if (startup_time != 0)
    {
        xwritelnf(ch, "Startup time was %s.",
                  str_time(startup_time, ch->pc->account->timezone, 0));
    }
}


DOFUN(who)
{

    if (!ch->pc) {
        return;
    }
    ch->titlef(ch, "Who's Playing on %s", engine_info.name);
    Buffer *buffer = new_buf();

    for (Character *wch = first_player; wch != 0; wch = wch->next_player)
    {
        xwritelnf(buffer, "~W[~Y%02d ~R%3.3s ~B%3.3s~W] %s %s",
                  wch->level, capitalize(wch->race->name),
                  class_who(wch), wch->name, wch->pc->title);
    }
    ch->page(ch, buf_string(buffer));
    destroy_buf(buffer);

}


DOFUN(score)
{
    Grid *grid = new_grid(scrwidth(ch), 3);
    Border left;
    Border right;
    grid_init_border(&left);
    strncpy(left.color, "~!B", 3);
    left.flags = BORDER_LEFT;
    grid_init_border(&right);
    strncpy(right.color, "~!B", 3);
    right.flags = BORDER_RIGHT;
    grid_addf(grid, ALIGN_CENTER, 3, 0, "~!B", 0, 0, "%s %s", NAME(ch),
              ch->pc ? ch->pc->title : ch->npc ? "the npc" : "");
    grid_addf_border(grid, &left, "~CLevel: ~W%d", ch->level);
    grid_addf_default(grid, "~CSex: ~W%s",
                      capitalize(sex_table[ch->sex].name));
    grid_addf_border(grid, &right, "~CPosition: ~W%s",
                     capitalize(position_table[ch->position].name));
    grid_addf_border(grid, &left, "~CFlags: ~W%s",

                     format_flags(ch->flags,
                                  ch->pc ? plr_flags : npc_flags));
    grid_addf_default(grid, "~CRace: ~W%s", capitalize(ch->race->name));
    grid_addf_border(grid, &right, "~CClass: ~W%s", class_short(ch));
    grid_add(grid, ALIGN_CENTER, 3, 0, "~!B", 0, 0, "Vitals");
    grid_addf_border(grid, &left, "~CHit: ~R%ld~W/~R%ld", ch->hit,
                     ch->maxHit);
    grid_addf_default(grid, "~CMana: ~M%ld~W/~M%ld", ch->mana, ch->maxMana);
    grid_addf_border(grid, &right, "~CMove: ~B%ld~W/~B%ld", ch->move,
                     ch->maxMove);
    // grid_set_next_row(grid, 6);
    int maxTrain = getMaxTrain(ch);
    grid_addf_border(grid, &left, "~CStr: ~W%d/%d",
                     getCurrStat(ch, STAT_STR), maxTrain);
    grid_addf_default(grid, "~CInt : ~W%d/%d", getCurrStat(ch, STAT_INT),
                      maxTrain);
    grid_addf_border(grid, &right, "~CWis : ~W%d/%d",
                     getCurrStat(ch, STAT_WIS), maxTrain);
    grid_addf_border(grid, &left, "~CDex: ~W%d/%d",
                     getCurrStat(ch, STAT_DEX), maxTrain);
    grid_addf_default(grid, "~CCon : ~W%d/%d", getCurrStat(ch, STAT_CON),
                      maxTrain);
    grid_addf_border(grid, &right, "~CLuck: ~W%d/%d",
                     getCurrStat(ch, STAT_LUCK), maxTrain);
    grid_add(grid, ALIGN_LEFT, 3, 0, "~!B", 0, 0, 0);
    grid_addf_border(grid, &left, "~CGold: ~Y%d", gold(ch->gold));
    grid_addf_default(grid, "~CSilver: ~W%d~x", silver(ch->gold));

    if (ch->pc)
    {
        grid_addf_border(grid, &right, "~CTerminal: ~W%s",
                         nullstr(ch->pc->conn->
                                 termType) ? "Unknown" : ch->pc->conn->
                         termType);
        grid_addf_border(grid, &left, "~CScreen Width: ~W%d",
                         ch->pc->conn->scrWidth);
        grid_addf_default(grid, "~CScreen Height: ~W%d",
                          ch->pc->conn->scrHeight);
    }
    // grid_set(grid, grid->x, grid->y, ALIGN_LEFT, 1, &right, 0, 0, 0);
    grid_addf_border(grid, &right, "~CAlignment: ~W%d", ch->alignment);

    if (!is_immortal(ch))
    {
        grid_addf_border(grid, &left, "~CExp: ~W%ld",
                         ch->pc->experience);
        grid_addf_default(grid, "~CTo Level: ~W%ld", exp_to_level(ch));
        grid_add(grid, ALIGN_LEFT, 1, &right, 0, 0, 0, 0);
    }
    grid_add(grid, ALIGN_LEFT, 3, 0, "~!B", 0, 0, 0);
    grid_print(grid, ch);
    destroy_grid(grid);

}


DOFUN(map)
{

    if (!ch->inRoom) {
        return;
    }
    draw_map(ch, NULL);
    return;

}

struct area_index
{
    Area *area;

    double percent;
    long stat;

};

int compare_area_explored(const void *v1, const void *v2)
{
    struct area_index area1 = *(struct area_index *)v1;
    struct area_index area2 = *(struct area_index *)v2;
    return (int)(area2.percent - area1.percent);

}


DOFUN(explored)
{
    int i = 0, c = 0, rcnt;

    double rooms, percent;
    Area *pArea;
    struct area_index *list;

    if (!ch || !ch->pc) {
        return;
    }

    if (nullstr(argument))
    {
        rcnt = roomcount(ch->pc->explored);
        rooms = (double)max_explorable_room;
        percent = UMIN(rcnt / (rooms / 100.0), 100.0);
        xwritelnf(ch, "%s has ~G%d~x explorable rooms.",
                  engine_info.name, max_explorable_room);
        xwritelnf(ch,
                  "You have explored ~G%d~x rooms (%.2f%%)~x of the world.",
                  rcnt, percent);
        rcnt = areacount(ch->pc->explored, ch->inRoom->area);
        rooms = (double)(arearooms(ch->inRoom->area));
        percent = UMIN(rcnt / (rooms / 100.0), 100.0);
        xwritelnf(ch, "\n\r%s has ~G%.0f~x explorable rooms.",
                  ch->inRoom->area->name, rooms);
        xwritelnf(ch,
                  "You have explored ~G%d (%.2f%%)~x rooms in this area.",
                  rcnt, percent);
        xwritelnf(ch,
                  "\n\rSee '%s list' to view all area percentages.~x",

                  do_name);
    }

    else if (is_exact_name(argument, "reset"))
    {
        destroy_flags(ch->pc->explored);
        ch->pc->explored = new_flag();
        xwriteln(ch, "Your explored rooms were set to 0.");
    }

    else if (!str_prefix(argument, "list"))
    {
        Buffer *output = new_buf();
        list =
            (struct area_index *)alloc_mem(max_area,
                                           sizeof(struct area_index));

        for (pArea = first_area; pArea != 0; pArea = pArea->next)
        {
            /*
             * if (!check_area(ch, pArea, false, false))
             * continue;
             */

            if (!is_set(pArea->flags, AREA_NOEXPLORE)) {
                continue;
            }
            rcnt = areacount(ch->pc->explored, pArea);
            rooms = (double)(arearooms(pArea));
            percent = UMIN(rcnt / (rooms / 100.0), 100.0);
            list[i].area = pArea;
            list[i].percent = percent;
            i++;
        }
        qsort(list, i, sizeof(struct area_index),
              compare_area_explored);

        for (c = 0; c < i; c++)
        {
            pArea = list[c].area;
            percent = list[c].percent;
            xwritelnf(output, "~D[~Y%3.0f~y%%~D]~x %s", percent,
                      pArea->name);
        }
        ch->page(ch, buf_string(output));
        free_mem(list);
        destroy_buf(output);
    }

    else
    {
        cmd_syntax(ch, do_name, "       - show current area and world.",
                   "list   - list percentages for all areas.",
                   "reset  - reset explored rooms.", 0);
    }

}


DOFUN(inventory)
{
    xwriteln(ch, "You are carrying:");
    show_list_to_char(ch->carrying, ch, true, true);

}


DOFUN(equipment)
{

    for (int i = 1; wear_table[i].display != 0; i++)
    {

        if (wear_table[i].canUse && !(*wear_table[i].canUse) (ch))
        {
            continue;
        }
        Object *obj = get_eq_char(ch, wear_table[i].loc);
        xwritef(ch, "~g<~w%s~g>~w ",
                align_string(ALIGN_CENTER, 22, 0, 0,
                             wear_table[i].display));

        if (obj)
        {

            format_obj_to_char(obj, ch, true);
        }

        else
        {
            xwriteln(ch, "Nothing");
        }
    }

}


DOFUN(affects)
{
    Affect *paf, *paf_last;
    // const char *buf4;
    // char buf3[BUF_SIZ];
    // char buf2[BUF_SIZ];
    bool found = false;
    // Flag * filter;
    // Flag * printme;
    Buffer *buffer = new_buf();
    // Object * obj;
    // int iWear;

    if (ch->affects)
    {
        xwriteln(buffer, "You are affected by the following spells:");
        paf_last = NULL;

        for (paf = ch->affects; paf; paf = paf->next)
        {

            if (paf_last != NULL && paf->from == paf_last->from)
            {

                if (ch->level >= 20)
                    xwrite(buffer,
                           "                          ");

                else {
                    continue;
                }
            }

            else
            {
                xwritef(buffer, "Spell: ~c%-19s~x",
                        valid_skill(paf->from)
                        ? skill_table[paf->from].
                        name : "unknown");
            }

            if (ch->level >= 20)
            {
                xwritef(buffer, ": modifies %s by %d ",
                        affect_name(paf), paf->modifier);

                if (paf->duration == -1) {
                    xwrite(buffer, "permanently~x");
                }

                else
                    xwritef(buffer, "for %d minutes~x",
                            (paf->duration *
                             (PULSE_TICK /
                              PULSE_PER_SECOND)) / 60);
            }
            xwriteln(buffer, "");
            paf_last = paf;
        }
        found = true;
        xwriteln(buffer, "");
    }
    /*
     * if (!COMM_SET(ch, COMM_SHORTAFF)) { if (ch->race->aff != 0 &&
     * IS_AFFECTED(ch, ch->race->aff)) { buffer.println("You are affected
     * by the following racial abilities:{x");
     *
     * strcpy(buf3, flag_string(affect_flags, ch->race->aff)); buf4 =
     * buf3; buf4 = one_argument(buf4, buf2); while (buf2[0]) {
     * buffer.printlnf("{xSpell: {c%-19s{x", buf2); buf4 =
     * one_argument(buf4, buf2); } found = true; buffer.println(NULL); }
     * if (ch->affected_by != 0 && (ch->affected_by != ch->race->aff)) {
     * bool print = false;
     *
     * for (iWear = 0; iWear < MAX_WEAR; iWear++) { if ((obj =
     * get_eq_char(ch, (wear_t) iWear)) != NULL) { for
     * (iter.Start(obj->affects); (paf = iter.Next());) { if
     * (!IS_SET(ch->affected_by, paf->bitvector)) continue;
     *
     * if (paf->where != TO_AFFECTS) continue;
     *
     * filter = paf->bitvector; filter &= ch->affected_by; printme =
     * filter; if (!print) { buffer.println("You are affected by the
     * following equipment spells:{x");
     *
     * print = true; }
     *
     * strcpy(buf3, flag_string(affect_flags, printme)); buf4 = buf3; buf4
     * = one_argument(buf4, buf2); while (buf2[0]) {
     * buffer.printlnf("{xSpell: {c%-19s:{x %s", buf2,
     * obj->getShortDesc()); buf4 = one_argument(buf4, buf2); } } if
     * (!obj->enchanted) { for (iter.Start(obj->pIndexData->affects); (paf
     * = iter.Next());) { if (!IS_SET(ch->affected_by, paf->bitvector))
     * continue; if (paf->where != TO_AFFECTS) continue; filter =
     * paf->bitvector; filter &= ch->affected_by; printme = filter; if
     * (!print) { buffer.println("You are affected by the following
     * equipment spells:{x"); print = true; }
     *
     * strcpy(buf3, flag_string(affect_flags, printme));
     *
     * buf4 = buf3; buf4 = one_argument(buf4, buf2); while (buf2[0]) {
     * buffer.printlnf("{xSpell: {c%-19s:{x %s", buf2,
     * obj->getShortDesc()); buf4 = one_argument(buf4, buf2); } } } } }
     * found = true; if (print) buffer.println(NULL); } }
     */

    if (!found)
    {
        xwriteln(buffer, "You are not affected by any spells.");
    }
    ch->page(ch, buf_string(buffer));
    destroy_buf(buffer);

}


DOFUN(resists)
{
    Buffer *buf = new_buf();
    xwritelnf(buf, "~c%-12s %s", "Type", "Value");
    xwritelnf(buf, "~c%s~x", fillstr("-", 19));

    for (int i = 0; i < MAX_DAM; i++)
    {
        xwritelnf(buf, "~C%-12s  ~W%3d~x", capitalize(dam_types[i].name),
                  ch->resists[dam_types[i].value]);
    }
    ch->page(ch, buf_string(buf));
    destroy_buf(buf);

}


DOFUN(socials)
{

    if (first_social == 0)
    {
        xwriteln(ch, "No socials found.");
        return;
    }
    Grid *grid = new_grid(scrwidth(ch), 5);

    for (Social *s = first_social; s; s = s->next)
    {
        grid_add_default(grid, s->name);
    }
    grid_print(grid, ch);
    destroy_grid(grid);

}


DOFUN(areas)
{
    Grid *grid = new_grid(scrwidth(ch), 2);

    for (Area *a = first_area; a; a = a->next)
    {
        grid_add_default(grid, a->name);
    }
    grid_print(grid, ch);
    destroy_grid(grid);
}


DOFUN(help)
{

    if (first_help == 0)
    {
        xwriteln(ch, "No help found.");
        return;
    }
    Help *help = help_match(argument);

    if (help == 0)
    {
        xwritelnf(ch, "No help found for '%s'.", argument);
        return;
    }

    if (!nullstr(help->syntax)) {
        xwriteln(ch, help->syntax);
    }
    xwriteln(ch, help->text);

    if (help->related) {
        xwritelnf(ch, "\n\rRelated: %s", help_related_string(help));
    }

}
