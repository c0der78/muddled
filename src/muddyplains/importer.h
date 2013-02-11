
/******************************************************************************
 *         __  __           _     _         ____  _       _                   *
 *        |  \/  |_   _  __| | __| |_   _  |  _ \| | __ _(_)_ __  ___         *
 *        | |\/| | | | |/ _` |/ _` | | | | | |_) | |/ _` | | '_ \/ __|        *
 *        | |  | | |_| | (_| | (_| | |_| | |  __/| | (_| | | | | \__ \        *
 *        |_|  |_|\__,_|\__,_|\__,_|\__, | |_|   |_|\__,_|_|_| |_|___/        *
 *                                  |___/                                     *
 *                                                                            *
 *         (C) 2010 by Ryan Jennings <c0der78@gmail.com> www.arg3.com         *
 *	               Many thanks to creators of muds before me.                 *
 *                                                                            *
 *        In order to use any part of this Mud, you must comply with the      *
 *     license in 'license.txt'.  In particular, you may not remove either    *
 *                        of these copyright notices.                         *
 *                                                                            *
 *       Much time and thought has gone into this software and you are        *
 *     benefitting.  I hope that you share your changes too.  What goes       *
 *                            around, comes around.                           *
 ******************************************************************************/
#ifndef __IMPORTER_H_
#define __IMPORTER_H_

#include <stdbool.h>
#include <stdio.h>

const char *fread_word(FILE *);
const char *fread_string(FILE *);
const char *fread_string_eol(FILE *);
char fread_letter(FILE *);
long fread_number(FILE *);
long fread_flag(FILE *);
void fread_to_eol(FILE *);
bool import_rom_area(FILE *);
bool import_rom_olc_area(FILE *);
bool import_rom_helps(FILE *);
bool import_rom_objects(FILE *);
bool import_rom_mobiles(FILE *);
bool import_rom_rooms(FILE *);
bool import_rom_specials(FILE *);
bool import_rom_shops(FILE *);
bool import_rom_socials(FILE *);
bool import_rom_resets(FILE *);
bool import_rom_area_list(const char *, FILE *);
bool import_rom_file(FILE *);

void import_commit(bool);

#endif
