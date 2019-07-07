
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
#include "explored.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "area.h"
#include "character.h"
#include "flag.h"
#include "log.h"
#include "player.h"
#include "private.h"
#include "room.h"
#include "str.h"

int bitcount(int c) {
  int count = 0;

  for (int i = 0; i < sizeof(int) * 8; i++) {
    if (c & (1 << i)) {
      count++;
    }
  }
  return count;
}

bool is_explorable(Room *room) {
  return !is_set(room->flags, ROOM_NOEXPLORE) && room->area && !is_set(room->area->flags, AREA_NOEXPLORE);
}

int roomcount(Flag *explored) {
  int pIndex = 0, count = 0;

  for (pIndex = 0; pIndex < explored->size; pIndex++) {
    count += bitcount(explored->bits[pIndex]);
  }
  return count;
}

void update_explored(Flag *explored) {
  Room *pRoom;
  int nMatch = 0;

  for (int id = 1; nMatch < max_room; id++) {
    if ((pRoom = get_room_by_id(id)) != NULL) {
      nMatch++;

      if (!is_explorable(pRoom) && is_set(explored, id)) {
        remove_bit(explored, id);
      }
    }

    else if (is_set(explored, id)) {
      remove_bit(explored, id);
    }
  }
}

int areacount(Flag *explored, Area *area) {
  Room *pRoom;
  int count = 0;

  if (area == NULL) {
    return 0;
  }

  for (pRoom = area->rooms; pRoom != 0; pRoom = pRoom->next_in_area) {
    count += is_set(explored, pRoom->id) ? 1 : 0;
  }
  return count;
}

int arearooms(Area *area) {
  int count = 0;
  Room *pRoom;

  if (!area) {
    return 0;
  }

  for (pRoom = area->rooms; pRoom != 0; pRoom = pRoom->next_in_area) {
    if (is_explorable(pRoom)) {
      count++;
    }
  }
  return count;
}

const char *get_explored_rle(Flag *explored) {
  int bit = 0;
  int count = 1;
  int hash;
  int len = 1;
  static char out[OUT_SIZ * 2];
  strcpy(out, "0");

  for (hash = 0; hash < ID_HASH; hash++) {
    for (Room *pRoom = room_hash[hash]; pRoom != 0; pRoom = pRoom->next) {
      if ((is_set(explored, pRoom->id) ? 1 : 0) == bit) {
        count++;
      }

      else {
        len += sprintf(&out[len], ",%d", count);
        count = 1;
        bit = (is_set(explored, pRoom->id)) ? 1 : 0;
      }
    }
  }
  sprintf(&out[len], ",%d,-1", count);
  return out;
}

void convert_explored_rle(Flag *explored, const char *str) {
  int index = 0;
  int bit = 0;
  int count = 0;
  int pos = 0;
  char *pstr = strtok((char *)str, ",");

  if (pstr == 0 || !is_number(pstr)) {
    log_error("unable to read explored rle");
    return;
  }
  bit = atoi(pstr);

  for (;;) {
    pstr = strtok(NULL, ",");

    if (!is_number(pstr)) {
      log_error("error in explored rle format");
      break;
    }
    count = atoi(pstr);

    if (count < 0) {
      break;
    }

    if (count == 0) {
      continue;
    }

    do {
      if (bit == 1) {
        set_bit(explored, index);
      }
      index++;
    }

    while (index < pos + count);
    pos = index;
    bit = (bit == 1) ? 0 : 1;
  }
}
