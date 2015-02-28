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

/*!
 * @header Structure, methods and constants related to areas @ignore
 * __AREA_H_
 */
#ifndef MUDDLED_AREA_H_
#define MUDDLED_AREA_H_

#include "typedef.h"

enum
{
    AREA_NOEXPLORE,
    AREA_CHANGED
};

/*!
 * @group Memory Management
 */

BEGIN_DECL

/*!
 * creates a new area @return the newly allocate area
 */
Area *new_area();

/*!
 * destroys an area @param the allocated area to destroy
 */
void destroy_area(Area *);

/*!
 * @group Memory Management
 */

/*!
 * @group Data Management
 */

/*!
 * loads all areas @return the number of area loaded
 */
int load_areas();

/*!
 * loads an area @param id the id of the area to load @return the area
 * loaded
 */
Area *load_area(identifier_t);

/*!
 * saves an area, including the structure an area contains @param area the
 * area to save @return 1 if successfull or zero
 */
int save_area(Area *);

/*!
 * saves an area only @param area the area to save @return 1 if successful
 * or zero
 */
int save_area_only(Area *);

/*!
 * @group Data Management
 */

/*!
 * get a loaded area by id @param id the area id @return the area or 0
 */
Area *get_area_by_id(identifier_t);

/*!
 * get a loaded area by name @param name the area name @return the area or
 * 0
 */
Area *area_lookup(const char *);

/*!
 * gets the default area.  Will create one if non-existant @return the
 * default area
 */
Area *get_default_area();

END_DECL

/*!
 * @constant first_area the linked-list of loaded areas
 */
extern Area *first_area;

/*!
 * @constants max_area the number of loaded areas
 */
extern int max_area;

/*!
 * @constants area_flags the table to convert area flags with strings
 */
extern const Lookup area_flags[];

#endif              /* // #ifndef AREA_H */
