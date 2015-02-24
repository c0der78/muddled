
/******************************************************************************
 *         __  __           _     _         ____  _       _                   *
 *        |  \/  |_   _  __| | __| |_   _  |  _ \| | __ _(_)_ __  ___         *
 *        | |\/| | | | |/ _` |/ _` | | | | | |_) | |/ _` | | '_ \/ __|        *
 *        | |  | | |_| | (_| | (_| | |_| | |  __/| | (_| | | | | \__ \        *
 *        |_|  |_|\__,_|\__,_|\__,_|\__, | |_|   |_|\__,_|_|_| |_|___/        *
 *                                  |___/                                     *
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

#include "telnet.h"
#include "../muddled/connection.h"
#include "../muddled/string.h"
#include <stdbool.h>
#include "../muddled/log.h"
#include "../muddled/macro.h"

ssize_t send_telopt(const Client *conn, unsigned char cmd, unsigned char opt)
{
    if (conn->websocket) return 0;

    unsigned char telopt[] = { IAC, cmd, opt };

    return send(conn->socket, telopt, sizeof(telopt) / sizeof(telopt[0]),
                0);

}

ssize_t test_telopts(const Client *conn)
{

    return send_telopt(conn, DO, TELOPT_TSPEED);

}

ssize_t send_telopt_sb(const Client *conn, unsigned char opt)
{
    if (conn->websocket) return 0;

    unsigned char telopt[] = { IAC, SB, opt, SEND, IAC, SE };

    return send(conn->socket, telopt, sizeof(telopt) / sizeof(telopt[0]),
                0);

}

void advertise_telopts(const Client *conn)
{

    send_telopt(conn, DO, TELOPT_NAWS);

    send_telopt(conn, DO, TELOPT_TTYPE);

}

void set_cursor(Client *conn, int r, int c)
{

    if (!conn->websocket)
        writef(conn, "\0337\033[%d;%dH", r, c);

}

void restore_cursor(Client *conn)
{
    if (!conn->websocket)
        write(conn, "\0338");

}

void clear_screen(Client *conn)
{
    if (!conn->websocket)
        write(conn, "\033[2J");

}

void process_telnet(Client *conn, unsigned char *buf, size_t index)
{

    size_t i = index;

    bool finished = false;

    size_t iac_sb_index;

    while (!finished && buf[i] == IAC)
    {

        switch (buf[i + 1])
        {

        case IAC:

            finished = true;

            break;

        case DO:

            switch (buf[i + 2])
            {

            case '\0':

                finished = true;

                i -= 3;

                break;

            case TELOPT_TSPEED:

                advertise_telopts(conn);

                break;

            }

            i += 3;

            break;

        case DONT:

            switch (buf[i + 2])
            {

            case '\0':

                finished = true;

                i -= 3;

                break;

            case TELOPT_TSPEED:

                advertise_telopts(conn);

                break;

            }

            i += 3;

            break;

        case WILL:

            switch (buf[i + 2])
            {

            case '\0':

                finished = true;

                break;

            case TELOPT_TSPEED:

                advertise_telopts(conn);

                break;

            case TELOPT_NAWS:

                send_telopt(conn, DO, TELOPT_NAWS);

                break;

            case TELOPT_TTYPE:

                send_telopt_sb(conn, TELOPT_TTYPE);

                break;

            }

            i += 3;

            break;

        case WONT:

            switch (buf[i + 2])
            {

            case '\0':

                finished = true;

                break;

            case TELOPT_TSPEED:

                advertise_telopts(conn);

                break;

            }

            i += 3;

            break;

        case SB:

            iac_sb_index = i;

            i += 2;

            // the only supported telnet option which uses SB is in the
            // format:
            // IAC SB TELOPT_TTYPE IS...IAC SE
            if (buf[i] == TELOPT_TTYPE && buf[i + 1] == IS
                    && buf[i + 2] != '\0')
            {

                // we have the starting of the terminal name, see RFC 930
                i += 2;

                size_t term_type_starts = i;
                // record the start of the terminal name

                // scan till we find the IAC SE terminating the terminal
                // name
                while (!(buf[i] == '\0' && buf[i + 1] == '\0')
                        && buf[i] != SE)
                {

                    if (buf[i] < 0x1F)
                    {

                        buf[i] = '?';
                        // replace any control characters
                    }
                    i++;
                    // skip all the characters up till the SE
                }

                // at this point we have either a double NUL or an SE
                if (buf[i] == '\0' && buf[i + 1] == '\0')
                {

                    // if we find a double NUL we have reached the end of
                    // the input
                    // stream before finding the expected SE
                    i = iac_sb_index;   // jump i back to the code
                    // starting IAC SB
                    finished = true;

                }
                else
                {

                    /*
                     * we know we have an SE within
                     * buf[i] ,due to the code above
                     * check for an IAC directly before
                     * it...e.g.format: IAC SB
                     * TELOPT_TTYPE IS...IAC SE
                     */

                    if (buf[i - 1] != IAC)
                    {

                        /*
                         * there is no IAC directly
                         * before the SE we have
                         * encounted, //it is not
                         * valid to have the SE value
                         * in a terminal name //
                         * based on the rules for
                         * terminal names in RFC
                         * 1060. // Quoting RFC 1060:
                         * //"A terminal names may be
                         * up to 40 characters taken
                         * from the set of upper- //
                         * case letters, digits, and
                         * the two punctuation
                         * characters hyphen and //
                         * slash.It must start with a
                         * letter, and end with a
                         * letter or digit." //
                         * therefore we will just
                         * gobble and ignore the
                         * sequence.
                         */
                    }
                    else
                    {

                        /*
                         * we have IAC SB
                         * TELOPT_TTYPE IS...IAC SE
                         * // term_type_start points
                         * at the first character
                         * after the IS. // i points
                         * at the SE.
                         */
                        buf[i - 1] = '\0';

                        for (size_t j =
                                    term_type_starts; buf[j];
                                j++)
                        {

                            if (buf[j] > 0x7f)
                            {

                                buf[j] = '?';

                            }
                        }

                        /*
                         * copy the terminal type
                         * text from the input into
                         * the buffer
                         */
                        free_str_dup(&conn->termType,
                                     (const char *)
                                     &buf
                                     [term_type_starts]);

                        log_trace("TTYPE %s",
                                  conn->termType);

                    }
                    i++;
                    /*
                     * skip over the SE
                     */
                }
            }
            else if (buf[i] == TELOPT_NAWS)
            {

                i += 1;

                size_t naws_starts = i;
                /*
                 * record the start of the terminal name
                 */

                /*
                 * scan till we find the IAC SE terminating
                 * the terminal name
                 */
                while (!(buf[i] == '\0' && buf[i + 1] == '\0')
                        && buf[i] != SE)
                {

                    i++;
                    /*
                     * skip all the characters up till
                     * the SE
                     */
                }

                /*
                 * at this point we have either a double NUL
                 * or an SE
                 */
                if (buf[i] == '\0' && buf[i + 1] == '\0')
                {

                    /*
                     * if we find a double NUL we have
                     * reached the end of the input //
                     * stream before finding the expected
                     * SE
                     */
                    i = iac_sb_index;   /* jump i back to the code
                                 * starting IAC SB */
                    finished = true;

                }
                else
                {

                    /*
                     * we know we have an SE within
                     * buf[i] ,due to the code above //
                     * check for an IAC directly before
                     * it...e.g.format: //IAC SB
                     * TELOPT_TTYPE IS...IAC SE
                     */

                    if (buf[i - 1] != IAC)
                    {

                        /*
                         * there is no IAC directly
                         * before the SE we have
                         * encounted, //it is not
                         * valid to have the SE value
                         * in a terminal name //
                         * based on the rules for
                         * terminal names in RFC
                         * 1060. // Quoting RFC 1060:
                         * //"A terminal names may be
                         * up to 40 characters taken
                         * from the set of upper- //
                         * case letters, digits, and
                         * the two punctuation
                         * characters hyphen and //
                         * slash.It must start with a
                         * letter, and end with a
                         * letter or digit." //
                         * therefore we will just
                         * gobble and ignore the
                         * sequence.
                         */
                    }
                    else
                    {

                        /*
                         * we have IAC SB
                         * TELOPT_NAWS...IAC SE //
                         * term_type_start points at
                         * the first character after
                         * the IS. // i points at the
                         * SE.
                         */
                        buf[i - 1] = '\0';

                        int x = 0, y = 0, t1, t2;

                        size_t j = naws_starts;

                        t1 = buf[j + 1];

                        t2 = buf[j + 2];

                        x = t1 + (t2 * 16);

                        t1 = buf[j + 3];

                        t2 = buf[j + 4];

                        y = t1 + (t2 * 16);

                        conn->scrWidth =
                            URANGE(10, x, 250);

                        conn->scrHeight =
                            URANGE(10, y, 250);

                        log_trace("NAWS %dx%d",
                                  conn->scrWidth,
                                  conn->scrHeight);

                    }
                    i++;

                }
            }
            else
            {

                /*
                 * non supported IAC SB option, or we dont
                 * have enough in the buffer to // know that
                 * we support it - scan till we find an SE or
                 * the end of buffer
                 */
                while (!(buf[i] == '\0' && buf[i + 1] == '\0')
                        && buf[i] != SE)
                {

                    i++;
                    /*
                     * skip all the characters up till
                     * the SE
                     */
                }

                if (buf[i] == SE)
                {

                    i++;

                    /*
                     * suboptions_chk(this,
                     * iac_sb_index);
                     */
                }
                else
                {

                    /*
                     * if we find a double NUL we have
                     * reached the end of the input //
                     * stream before finding the expected
                     * SE
                     */
                    i = iac_sb_index;
                    /*
                     * jump i back to the code starting
                     * IAC SB
                     */
                    finished = true;

                }

            }

            break;

        default:
            /*
             * we dont know how to handle it, assume it is IAC
             * something something // so skip 3 characters
             */
            if (buf[i + 2] == '\0')
            {

                finished = true;

            }
            else
            {

                log_warn("ignoring IAC %d %d", buf[i + 1],
                         buf[i + 2]);

                i += 3;

            }

        }

    }

    memmove(&buf[index], &buf[i], strlen((const char *)&buf[i]) + 1);

}
