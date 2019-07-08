
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
#include "client.h"
#include "lookup.h"
#include "macro.h"
#include "olc.h"
#include "social.h"
#include "str.h"
#include "telnet.h"

Editor *build_social_editor(Social *social) {
  Editor *editor = new_editor();
  editor->data = social;
  editor->edit = social_editor;
  editor->show = social_editor_menu;
  return editor;
}

void social_editor_menu(Client *conn) {
  clear_screen(conn);
  set_cursor(conn, 1, 1);
  conn->title(conn, "Social Editor");
  Social *social = (Social *)conn->editing->data;
  xwritelnf(conn, "   ~CId: ~W%d", social->id);
  xwritelnf(conn, "~YA) ~CName: ~W%s~x", social->name);
  xwritelnf(conn, "~YB) ~CCharNoArg: ~W%s~x", social->charNoArg);
  xwritelnf(conn, "~YC) ~COthersNoArg: ~W%s~x", social->othersNoArg);
  xwritelnf(conn, "~YD) ~CCharFound: ~W%s~x", social->charFound);
  xwritelnf(conn, "~YE) ~COthersFound: ~W%s~x", social->othersFound);
  xwritelnf(conn, "~YF) ~CVictFound: ~W%s~x", social->victFound);
  xwritelnf(conn, "~YG) ~CCharNotFound: ~W%s~x", social->charNotFound);
  xwritelnf(conn, "~YH) ~CCharAuto: ~W%s~x", social->charAuto);
  xwritelnf(conn, "~YI) ~COthersAuto: ~W%s~x", social->othersAuto);
  xwritelnf(conn, "~YJ) ~CCharObjFound: ~W%s~x", social->charObjFound);
  xwritelnf(conn, "~YK) ~COthersObjFound: ~W%s~x", social->othersObjFound);
  xwritelnf(conn, "~YL) ~CMin Position: ~W%s~x", position_table[social->minPosition].name);
}

void social_edit_list(Client *conn) {
  int count = 0;

  for (Social *social = first_social; social != 0; social = social->next) {
    xwritef(conn, "%2d) %-12.12s ", social->id, social->name);

    if (++count % 4 == 0) {
      xwriteln(conn, "");
    }
  }

  if (count % 4 != 0) {
    xwriteln(conn, "");
  }
}

void social_editor(Client *conn, const char *argument) {
  char arg[100];
  argument = one_argument(argument, arg);

  if (nullstr(arg) || arg[0] == '?') {
    olc_syntax(conn, 0);
    return;
  }

  if (!str_cmp(arg, "Q") || !str_cmp(arg, "quit")) {
    finish_editing(conn);
    return;
  }

  if (!str_prefix(arg, "show")) {
    conn->editing->show(conn);
    return;
  }
  Social *social = (Social *)conn->editing->data;

  if (!str_cmp(arg, "save")) {
    save_social(social);
    xwriteln(conn, "~CSocial saved.~x");
    return;
  }

  if (!str_cmp(arg, "list")) {
    social_edit_list(conn);
    return;
  }

  if (!str_cmp(arg, "A") || !str_cmp(arg, "name")) {
    free_str(social->name);
    social->name = str_dup(argument);
    conn->editing->show(conn);
    return;
  }

  if (!str_cmp(arg, "B") || !str_cmp(arg, "charnoarg")) {
    free_str(social->charNoArg);
    social->charNoArg = str_dup(argument);
    conn->editing->show(conn);
    return;
  }

  if (!str_cmp(arg, "C") || !str_cmp(arg, "othersnoarg")) {
    free_str(social->othersNoArg);
    social->othersNoArg = str_dup(argument);
    conn->editing->show(conn);
    return;
  }

  if (!str_cmp(arg, "D") || !str_cmp(arg, "charfound")) {
    free_str(social->charFound);
    social->charFound = str_dup(argument);
    conn->editing->show(conn);
    return;
  }

  if (!str_cmp(arg, "E") || !str_cmp(arg, "othersfound")) {
    free_str(social->othersFound);
    social->othersFound = str_dup(argument);
    conn->editing->show(conn);
    return;
  }

  if (!str_cmp(arg, "F") || !str_cmp(arg, "victfound")) {
    free_str(social->name);
    social->name = str_dup(argument);
    conn->editing->show(conn);
    return;
  }

  if (!str_cmp(arg, "G") || !str_cmp(arg, "charnotfound")) {
    free_str(social->charNotFound);
    social->charNotFound = str_dup(argument);
    conn->editing->show(conn);
    return;
  }

  if (!str_cmp(arg, "H") || !str_cmp(arg, "charauto")) {
    free_str(social->charAuto);
    social->charAuto = str_dup(argument);
    conn->editing->show(conn);
    return;
  }

  if (!str_cmp(arg, "I") || !str_cmp(arg, "othersAuto")) {
    free_str(social->othersAuto);
    social->othersAuto = str_dup(argument);
    conn->editing->show(conn);
    return;
  }

  if (!str_cmp(arg, "J") || !str_cmp(arg, "charobjfound")) {
    free_str(social->charObjFound);
    social->charObjFound = str_dup(argument);
    conn->editing->show(conn);
    return;
  }

  if (!str_cmp(arg, "K") || !str_cmp(arg, "othersobjfound")) {
    free_str(social->othersObjFound);
    social->othersObjFound = str_dup(argument);
    conn->editing->show(conn);
    return;
  }

  if (!str_cmp(arg, "L") || !str_cmp(arg, "minposition")) {
    long val = value_lookup(position_table, argument);

    if (val == -1) {
      xwritelnf(conn, "~CValid positions are: ~W%s~x", lookup_names(position_table));
      return;
    }
    social->minPosition = (position_t)val;
    conn->editing->show(conn);
    return;
  }
}
