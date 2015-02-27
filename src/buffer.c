
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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "log.h"
#include "engine.h"
#include "buffer.h"

#define MAX_BUF     16384
#define MAX_BUF_LIST    10
#define BASE_BUF    1024

/*
 * valid states
 */
enum buf_t
{
    BUFFER_SAFE,
    BUFFER_OVERFLOW
};

/*
 * buffer sizes
 */
const int buf_size[MAX_BUF_LIST] =
{
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384
};

/*
 * local procedure for finding the next acceptable size
 */

/*
 * -1 indicates out-of-boundary error
 */
static int get_size(int val)
{

    int i;

    for (i = 0; i < MAX_BUF_LIST; i++)
        if (buf_size[i] >= val)
        {

            return buf_size[i];

        }
    return -1;

}


char *get_temp_buf()
{
    static char temp_buf[MAX_TEMP_BUF][TEMP_BUF_SIZ];

    static size_t temp_buf_index = 0;

    ++temp_buf_index;

    temp_buf_index %= MAX_TEMP_BUF;

    return temp_buf[temp_buf_index];
}

Buffer *new_buf()
{

    Buffer *buffer = (Buffer *) alloc_mem(1, sizeof(Buffer));

    buffer->state = BUFFER_SAFE;

    buffer->size = get_size(BASE_BUF);

    buffer->string = (char *)alloc_mem(buffer->size, sizeof(char));

    buffer->string[0] = '\0';

    buffer->write = buf_add;

    buffer->writeln = buf_addln;

    buffer->writef = buf_addf;

    buffer->writelnf = buf_addlnf;

    return buffer;

}

Buffer *new_buf_size(int size)
{

    Buffer *buffer = (Buffer *) alloc_mem(1, sizeof(Buffer));

    buffer->state = BUFFER_SAFE;

    buffer->size = get_size(size);

    if (buffer->size == -1)
    {

        log_error("new_buf: buffer size %d too large.", size);

        abort();
    }
    buffer->string = (char *)alloc_mem(buffer->size, sizeof(char));

    buffer->string[0] = '\0';

    return buffer;

}

void destroy_buf(Buffer *buffer)
{

    free_mem(buffer->string);

    free_mem(buffer);

}

bool buf_addf(Buffer *buffer, const char *fmt, ...)
{

    char buf[OUT_SIZ];

    va_list args;

    va_start(args, fmt);

    vsnprintf(buf, sizeof(buf), fmt, args);

    va_end(args);

    return buf_add(buffer, buf);

}

bool buf_addlnf(Buffer *buffer, const char *fmt, ...)
{

    char buf[OUT_SIZ];

    va_list args;

    va_start(args, fmt);

    vsnprintf(buf, sizeof(buf) - 3, fmt, args);

    va_end(args);

    strcat(buf, "\n\r");

    return buf_add(buffer, buf);

}

bool buf_addln(Buffer *buffer, const char *string)
{

    return buf_add(buffer, string) && buf_add(buffer, "\n\r");

}

bool buf_add_len(Buffer *buffer, const char *string, size_t str_len)
{

    size_t len;

    char *oldstr;

    int oldsize;

    oldstr = buffer->string;

    oldsize = buffer->size;

    if (buffer->state == BUFFER_OVERFLOW)   /* don't waste time on bad
                         * * strings! */
        return false;

    len = strlen(buffer->string) + str_len + 1;

    while (len >= buffer->size)     /* increase the buffer size */
    {

        buffer->size = get_size(buffer->size + 1);

        if (buffer->size == -1)     /* overflow */
        {

            buffer->size = oldsize;

            buffer->state = BUFFER_OVERFLOW;

            log_warn("buffer overflow past size %d", buffer->size);

            return false;

        }
    }

    if (buffer->size != oldsize)
    {

        buffer->string = (char *)alloc_mem(buffer->size, sizeof(char));

        strcpy(buffer->string, oldstr);

        free_mem(oldstr);

    }
    strncat(buffer->string, string, str_len);

    return true;

}

bool buf_add(Buffer *buffer, const char *string)
{

    return buf_add_len(buffer, string, strlen(string));

}

void clear_buf(Buffer *buffer)
{

    buffer->string[0] = '\0';

    buffer->state = BUFFER_SAFE;

}

char *buf_string(Buffer *buffer)
{

    return buffer->string;

}
