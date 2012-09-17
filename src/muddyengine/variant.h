
/******************************************************************************
 *         __  __           _     _         ____  _       _                   *
 *        |  \/  |_   _  __| | __| |_   _  |  _ \| | __ _(_)_ __  ___         *
 *        | |\/| | | | |/ _` |/ _` | | | | | |_) | |/ _` | | '_ \/ __|        *
 *        | |  | | |_| | (_| | (_| | |_| | |  __/| | (_| | | | | \__ \        *
 *        |_|  |_|\__,_|\__,_|\__,_|\__, | |_|   |_|\__,_|_|_| |_|___/        *
 *                                  |___/                                     *
 *                                                                            *
 *    (C) 2010 by Ryan Jennings <c0der78@gmail.com> www.ryan-jennings.net     *
 *             Many thanks to creators of muds before me.                 *
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
 * @header Structure, methods and constants related to variants @ignore
 * __VARIANT_H_
 */
#ifndef __VARIANT_H_
#define __VARIANT_H_

/*!
 * @enum Variable Types @abstract definitions for variable types @constant
 * VAR_CHAR a character variable @constant VAR_SHORT a short int variable
 * @constant VAR_INT a int variable @constant VAR_LONG a long variable
 * @constant VAR_FLOAT a float variable @constant VAR_DOUBLE a double
 * variable @constant VAR_PTR a generic ptr varaible @discussion
 */
enum {
    VAR_CHAR,
    VAR_SHORT,
    VAR_INT,
    VAR_LONG,
    VAR_FLOAT,
    VAR_DOUBLE,
    VAR_PTR
};

/*!
 * @typedef variant_t @abstract Represents a variable that can have
 * multiple types @field type the type of variable @field char_value the
 * character value @field short_value the short integer value @field
 * int_value the integer value @field long_value the long integer value
 * @field float_value the float_value @field double_value the double value
 * @field ptr_value the generic pointer value
 */
typedef struct variant_t {
    int type;
    union {
        char char_value;
        short short_value;
        int int_value;
        long long_value;
        float float_value;
        double double_value;
        const void *ptr_value;
    };
} variant_t;

/*!
 * sets the variant char value @param v the variant to set @param value the
 * char value
 */
void vset_char(variant_t *, char);
/*!
 * sets the variant short value @param v the variant to set @param value
 * the short value to set
 */
void vset_short(variant_t *, short);
/*!
 * sets the variant int value @param v the variant to set @param value the
 * int value to set
 */
void vset_int(variant_t *, int);
/*!
 * sets the variant long value @param v the variant to set @param value the
 * long value to set
 */
void vset_long(variant_t *, long);
/*!
 * sets the variant float value @param v the variant to set @param value
 * the float value to set
 */
void vset_float(variant_t *, float);
/*!
 * sets the variant double value @param v the variant to set @param value
 * the double value to set
 */
void vset_double(variant_t *, double);
/*!
 * sets the variant pointer value @param v the variant to set @param value
 * the pointer value to set
 */
void vset_ptr(variant_t *, const void *);

/*!
 * gets the set variant type @param v the variant to get the type from
 */
int vget_type(variant_t * t);
/*!
 * gets the variant char value @param v the variant to get the value from
 */
char vget_char(variant_t *);
/*!
 * gets the variant short value @param v the variant to get the value from
 */
short vget_short(variant_t *);
/*!
 * gets the variant integer value @param v the variant to get the value
 * from
 */
int vget_int(variant_t *);
/*!
 * gets the variant long integer value @param v the variant to get the
 * value from
 */
long vget_long(variant_t *);
/*!
 * gets the variant float value @param v the variant to get the value from
 */
float vget_float(variant_t *);
/*!
 * gets the variant double value @param v the variant to get the value from
 */
double vget_double(variant_t *);
/*!
 * gets the variant pointer value @param v the variant to get the value
 * from
 */
const void *vget_ptr(variant_t *);

#endif
