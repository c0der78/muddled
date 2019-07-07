
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
#include "nonplayer.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "area.h"
#include "character.h"
#include "db.h"
#include "engine.h"
#include "log.h"
#include "lookup.h"
#include "macro.h"
#include "private.h"
#include "race.h"
#include "str.h"

const Lookup npc_flags[] = {{"sentinel", NPC_SENTINEL}, {"scavenger", NPC_SCAVENGER}, {"aggressive", NPC_AGGRESSIVE},
                            {"wimpy", NPC_WIMPY},       {"stay_area", NPC_STAY_AREA}, {0, 0}};

NPC *new_npc() {
  NPC *npc = (NPC *)alloc_mem(1, sizeof(NPC));
  npc->shortDescr = str_empty;
  npc->longDescr = str_empty;
  npc->startPosition = POS_STANDING;
  return npc;
}

void destroy_npc(NPC *npc) {
  free_str(npc->shortDescr);
  free_str(npc->longDescr);
  free_mem(npc);
}

void load_npc_columns(Character *ch, sql_stmt *stmt) {
  int count = sql_column_count(stmt);

  for (int i = 0; i < count; i++) {
    const char *colname = sql_column_name(stmt, i);

    if (load_char_column(ch, stmt, colname, i)) {
    }

    else if (!str_cmp(colname, "shortDescr")) {
      ch->npc->shortDescr = str_dup(sql_column_str(stmt, i));
    }

    else if (!str_cmp(colname, "longDescr")) {
      ch->npc->longDescr = str_dup(sql_column_str(stmt, i));
    }

    else if (!str_cmp(colname, "startPosition")) {
      ch->npc->startPosition = sql_column_int(stmt, i);
    }

    else if (!str_cmp(colname, "flags")) {
      parse_flags(ch->flags, sql_column_str(stmt, i), npc_flags);
    }

    else if (!str_cmp(colname, "areaId")) {
      ch->npc->area = get_area_by_id(sql_column_int(stmt, i));
    }

    else {
      log_warn("unknown nonplayer column '%s'", colname);
    }
  }
}

Character *load_npc(identifier_t id) {
  char buf[400];
  sql_stmt *stmt;
  Character *ch = 0;
  int len = sprintf(buf, "select * from character join nonplayer on nonplayerId=characterId where charId=%" PRId64, id);

  if (sql_query(buf, len, &stmt) != SQL_OK) {
    log_data("could not prepare statement");
    return 0;
  }

  if (sql_step(stmt) != SQL_DONE) {
    ch = new_char();
    ch->npc = new_npc();
    load_npc_columns(ch, stmt);
    LINK(ch->npc->area->npcs, ch, next_in_area);
    LINK(first_character, ch, next);
  }

  if (sql_finalize(stmt) != SQL_OK) {
    log_data("could not finalize statement");
  }
  return ch;
}

int load_npcs(Area *area) {
  char buf[400];
  sql_stmt *stmt;
  int total = 0;
  int len = sprintf(buf, "select * from character natural join nonplayer where areaId=%" PRId64, area->id);

  if (sql_query(buf, len, &stmt) != SQL_OK) {
    log_data("could not prepare statement");
    return 0;
  }

  while (sql_step(stmt) != SQL_DONE) {
    Character *ch = new_char();
    ch->npc = new_npc();
    ch->npc->area = area;
    load_npc_columns(ch, stmt);
    LINK(area->npcs, ch, next_in_area);
    LINK(first_character, ch, next);
    total++;
  }

  if (sql_finalize(stmt) != SQL_OK) {
    log_data("could not finalize statement");
  }
  return total;
}

int save_npc(Character *ch) {
  int res = save_character(ch, npc_flags);
  field_map npc_values[] = {{"nonplayerId", &ch->id, SQL_INT},
                            {"shortDescr", &ch->npc->shortDescr, SQL_TEXT},
                            {"longDescr", &ch->npc->longDescr, SQL_TEXT},
                            {"startPosition", &ch->npc->startPosition, SQL_INT},
                            {"areaId", &ch->npc->area->id, SQL_INT},
                            {0, 0, 0}};

  if (res == 1) {
    if (sql_insert_query(npc_values, "nonplayer") != SQL_OK) {
      log_data("could not insert player");
      return 0;
    }
  }

  else if (res == 2) {
    if (sql_update_query(npc_values, "nonplayer", ch->id) != SQL_OK) {
      log_data("could not update character");
      return 0;
    }
  }
  return UMIN(res, 1);
}

int delete_npc(Character *ch) {
  char buf[BUF_SIZ];

  if (!delete_character(ch)) {
    return 0;
  }
  sprintf(buf, "delete from nonplayer where charId=%" PRId64, ch->id);

  if (sql_exec(buf) != SQL_OK) {
    log_data("could not delete player");
    return 0;
  }
  return 1;
}

Character *npc_lookup(const char *arg) {
  if (is_number(arg)) {
    return get_npc_by_id(atoi(arg));
  } else {
    for (Character *nch = first_character; nch != 0; nch = nch->next) {
      if (nch->npc == 0) {
        continue;
      }

      if (is_name(arg, nch->name)) {
        return nch;
      }
    }
  }
  return 0;
}

Character *get_npc_by_id(identifier_t id) {
  for (Character *nch = first_character; nch != 0; nch = nch->next) {
    if (nch->npc == 0) {
      continue;
    }

    if (nch->id == id) {
      return nch;
    }
  }
  return 0;
}
