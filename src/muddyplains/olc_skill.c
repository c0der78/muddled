
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

#include "olc.h"
#include "telnet.h"
#include "client.h"
#include <muddyengine/skill.h>
#include <muddyengine/string.h>
#include <muddyengine/lookup.h>
#include <muddyengine/engine.h>
#include <muddyengine/class.h>

Editor *build_skill_editor(Skill * skill)
{

    Editor *editor = new_editor();

    editor->data = skill;

    editor->edit = skill_editor;

    editor->show = skill_editor_menu;

    return editor;

}

void skill_editor_menu(Client * conn)
{

    clear_screen(conn);

    set_cursor(conn, 1, 1);

    conn->title(conn, "Skill Editor");

    Skill *skill = (Skill *) conn->editing->data;

    writelnf(conn, "   ~CId: ~W%d", skill->id);

    writelnf(conn, "~YA) ~CName: ~W%s~x", skill->name);

    writelnf(conn, "~YB) ~CDamage: ~W%s~x", skill->damage);

    writelnf(conn, "~YC) ~CMsgOff: ~W%s~x", skill->msgOff);

    writelnf(conn, "~YD) ~CMsgObj: ~W%s~x", skill->msgObj);

    writelnf(conn, "~YE) ~CWait: ~W%d~x", skill->wait);

    writelnf(conn, "~YF) ~CMana: ~W%d~x", skill->mana);

    writelnf(conn, "~YG) ~CCost: ~W%.2f~x", skill->cost);

    const char *spfun = spellfun_name(skill->spellfun);

    writelnf(conn, "~YH) ~CSpell: ~W%s~x", spfun ? spfun : "None");

    const char *gsname = gsn_name(skill->pgsn);

    writelnf(conn, "~YI) ~CGSN: ~W%s~x", gsname ? gsname : "None");

    writelnf(conn, "~YJ) ~CMinPos: ~W%s~x",
	     position_table[skill->minPos].name);

    writelnf(conn, "~YK) ~CFlags: ~W%s~x",
	     format_flags(&skill->flags, skill_flags));

    write(conn, "~YL) ~CLevels: ~W");

    for (int i = 0; i < max_class; i++) {

	writef(conn, "%s [%d] ", class_table[i].name, skill->levels[i]);

    }

    writeln(conn, "");

}

void skill_edit_list(Client * conn)
{

    int count = 0;

    for (int i = 0; i < max_skill; i++) {

	writef(conn, "%2d) %-12.12s ", skill_table[i].id,
	       skill_table[i].name);

	if (++count % 4 == 0)
	    writeln(conn, "");

    }

    if (count % 4 != 0)
	writeln(conn, "");

}

void skill_editor(Client * conn, const char *argument)
{

    char arg[100];

    argument = one_argument(argument, arg);

    if (!str_cmp(arg, "Q")) {

	finish_editing(conn);

	return;

    }
    if (!str_prefix(arg, "show")) {

	conn->editing->show(conn);

	return;

    }
    Skill *skill = (Skill *) conn->editing->data;

    if (!str_cmp(arg, "save")) {

	save_skill(skill);

	writeln(conn, "~CSkill saved.~x");

	return;

    }
    if (!str_cmp(arg, "list")) {

	skill_edit_list(conn);

	return;

    }
    if (!str_cmp(arg, "A") || !str_cmp(arg, "name")) {

	if (nullstr(argument)) {

	    writeln(conn, "~CChange skill name to what?~x");

	    return;

	}
	free_str(skill->name);

	skill->name = str_dup(argument);

	conn->editing->show(conn);

	return;

    }
    if (!str_cmp(arg, "B") || !str_cmp(arg, "damage")) {

	if (nullstr(argument)) {

	    free_str(skill->damage);

	    skill->damage = str_empty;

	} else {

	    free_str(skill->name);

	    skill->damage = str_dup(argument);

	}

	conn->editing->show(conn);

	return;

    }
    if (!str_cmp(arg, "C") || !str_cmp(arg, "msgOff")) {

	if (nullstr(argument)) {

	    free_str(skill->msgOff);

	    skill->msgOff = str_empty;

	} else {

	    free_str(skill->msgOff);

	    skill->msgOff = str_dup(argument);

	}

	conn->editing->show(conn);

	return;

    }
    if (!str_cmp(arg, "D") || !str_cmp(arg, "msgObj")) {

	if (nullstr(argument)) {

	    free_str(skill->msgObj);

	    skill->msgObj = str_empty;

	} else {

	    free_str(skill->msgObj);

	    skill->msgObj = str_dup(argument);

	}

	conn->editing->show(conn);

	return;

    }
    if (!str_cmp(arg, "F") || !str_cmp(arg, "mana")) {

	if (!is_number(argument)) {

	    writeln(conn, "~CThat is not a number.~x");

	    return;

	}
	skill->mana = atoi(argument);

	conn->editing->show(conn);

    }
    if (!str_cmp(arg, "E") || !str_cmp(arg, "wait")) {

	if (!is_number(argument)) {

	    writeln(conn, "~CThat is not a number.~x");

	    return;

	}
	skill->mana = atoi(argument);

	conn->editing->show(conn);

	return;

    }
    if (!str_cmp(arg, "G") || !str_cmp(arg, "cost")) {

	if (!is_number(argument)) {

	    writeln(conn, "~CThat is not a number.~x");

	    return;

	}
	skill->mana = (float) atof(argument);

	conn->editing->show(conn);

	return;

    }
    if (!str_cmp(arg, "H") || !str_cmp(arg, "spell")) {

	if (nullstr(argument)) {

	    skill->spellfun = 0;

	} else {

	    skill->spellfun = spellfun_lookup(argument);

	    if (skill->spellfun == 0) {

		writeln(conn, "~CValid spells are:~x");

		int i;

		for (i = 0; spellfun_table[i].name != 0; i++) {

		    writef(conn, "%-10s ", spellfun_table[i].name);

		    if (i % 4 == 0)
			writeln(conn, "");

		}

		if (i % 4 != 0)
		    writeln(conn, "");

		return;

	    }
	}

	conn->editing->show(conn);

	return;

    }
    if (!str_cmp(arg, "I") || !str_cmp(arg, "gsn")) {

	if (nullstr(argument)) {

	    skill->pgsn = 0;

	    conn->editing->show(conn);

	    return;

	}
	skill->pgsn = gsn_lookup(argument);

	if (skill->pgsn == 0) {

	    writeln(conn, "~CValid gsns are:~x");

	    int i;

	    for (i = 0; gsn_table[i].name != 0; i++) {

		writef(conn, "%-10s ", gsn_table[i].name);

		if (i % 4 == 0)
		    writeln(conn, "");

	    }

	    if (i % 4 != 0)
		writeln(conn, "");

	    return;

	}
	conn->editing->show(conn);

	return;

    }
    if (!str_cmp(arg, "J") || !str_cmp(arg, "minpos")) {

	long pos = value_lookup(position_table, argument);

	if (pos == -1) {

	    writelnf(conn, "~CValid positions are: ~W%s~x",
		     lookup_names(position_table));

	    return;

	}
	skill->minPos = (position_t) pos;

	conn->editing->show(conn);

	return;

    }
    if (!str_cmp(arg, "K") || !str_cmp(arg, "flags")) {

	if (edit_flag("flags", conn, &skill->flags, argument, skill_flags))
	    conn->editing->show(conn);

	return;

    }
    if (!str_cmp(arg, "L") || !str_cmp(arg, "levels")) {

	char name[BUF_SIZ];

	argument = one_argument(argument, name);

	int c = class_lookup(name);

	if (c == -1) {

	    writeln(conn, "~CValid classes are:~W");

	    for (int i = 0; i < max_class; i++) {

		writef(conn, "%s ", class_table[i].name);

	    }
	    writeln(conn, "~x");

	    return;

	}
	if (!is_number(argument)) {

	    writeln(conn, "~CThat is not a valid level.~x");

	    return;

	}
	skill->levels[c] = atoi(argument);

	conn->editing->show(conn);

	return;

    }
}
