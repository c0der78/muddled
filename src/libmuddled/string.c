
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

#include <muddled/macro.h>
#include <ctype.h>
#include <string.h>
#include <muddled/string.h>
#include <muddled/color.h>
#include <stdio.h>
#include <muddled/engine.h>
#include <muddled/log.h>
#include <stdarg.h>
#include <regex.h>

const char str_empty[1] = { 0 };

int match(const char *string, const char *pattern)
{

    int status;

    regex_t re;

    if (regcomp(&re, pattern, REG_EXTENDED | REG_NOSUB) != 0)
    {

        return 0;

    }
    status = regexec(&re, string, (size_t) 0, NULL, 0);

    regfree(&re);

    if (status != 0)
    {

        return 0;

    }
    return 1;

}

char *trim(char *b)
{
    char *e = strrchr(b, '\0');	/* Find the final null */
    while (b < e && isspace(*b))	/* Scan forward */
        ++b;
    while (e > b && isspace(*(e - 1)))	/* scan back from end */
        --e;
    *e = '\0';		/* terminate new string */
    return b;
}

/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp(const char *astr, const char *bstr)
{

    if (astr == NULL)
    {

        return true;

    }
    if (bstr == NULL)
    {

        return true;

    }
    for (; *astr || *bstr; astr++, bstr++)
    {

        if (LOWER(*astr) != LOWER(*bstr))
            return true;

    }

    return false;

}

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix(const char *astr, const char *bstr)
{

    if (astr == NULL || !*astr)
    {

        return true;

    }
    if (bstr == NULL)
    {

        return true;

    }
    for (; *astr; astr++, bstr++)
    {

        if (LOWER(*astr) != LOWER(*bstr))
            return true;

    }

    return false;

}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix(const char *astr, const char *bstr)
{

    size_t sstr1;

    size_t sstr2;

    size_t ichar;

    char c0;

    if ((c0 = LOWER(astr[0])) == '\0')
        return false;

    sstr1 = strlen(astr);

    sstr2 = strlen(bstr);

    for (ichar = 0; ichar <= sstr2 - sstr1; ichar++)
    {

        if (c0 == LOWER(bstr[ichar]) && !str_prefix(astr, bstr + ichar))
            return false;

    }

    return true;

}

/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix(const char *astr, const char *bstr)
{

    size_t sstr1, sstr2;

    sstr1 = strlen(astr);

    sstr2 = strlen(bstr);

    if (sstr1 <= sstr2 && !str_cmp(astr, bstr + sstr2 - sstr1))
        return false;

    else
        return true;

}

bool is_name(const char *str, const char *namelist)
{

    char name[BUF_SIZ], part[BUF_SIZ];

    const char *list, *string;

    /*
     * fix crash on NULL namelist
     */
    if (namelist == NULL || namelist[0] == '\0')
        return true;

    /*
     * fixed to prevent is_name on "" returning TRUE
     */
    if (str[0] == '\0')
        return false;

    string = str;

    /*
     * we need ALL parts of string to match part of namelist
     */
    for (;;)  		/* start parsing string */
    {

        str = one_argument(str, part);

        if (part[0] == '\0')
            return true;

        /*
         * check to see if this is part of namelist
         */
        list = namelist;

        for (;;)  	/* start parsing namelist */
        {

            list = one_argument(list, name);

            if (name[0] == '\0')	/* this name was not found */
                return false;

            if (!str_prefix(string, name))
                return true;	/* full pattern match */

            if (!str_prefix(part, name))
                break;

        }

    }

}

bool is_exact_name(const char *str, const char *namelist)
{

    char name[BUF_SIZ];

    if (namelist == NULL)
        return false;

    for (;;)
    {

        namelist = one_argument(namelist, name);

        if (name[0] == '\0')
            return false;

        if (!str_cmp(str, name))
            return true;

    }

}

const char *stristr(const char *String, const char *Pattern)
{

    const char *pptr, *sptr, *start;

    for (start = String; *start != 0; start++)
    {

        /*
         * find start of pattern in string
         */
        for (; ((*start != 0)
                && (toupper((int)*start) !=
                    toupper((int)*Pattern))); start++) ;

        if (0 == *start)
            return NULL;

        pptr = Pattern;

        sptr = start;

        while (toupper((int)*sptr) == toupper((int)*pptr))
        {

            sptr++;

            pptr++;

            /*
             * if end of pattern then pattern was found
             */

            if (0 == *pptr)
                return (start);

        }

    }

    return NULL;

}

const char *str_replace(const char *orig, const char *old, const char *pnew)
{

    char *buf;

    const char *ptr;

    size_t len, a, b;

    if ((ptr = strstr(orig, old)) == NULL
            || (a = ptr - orig) + (len = strlen(pnew)) >= (OUT_SIZ - 4))
        return orig;

    static char outbuf[7][OUT_SIZ];

    static int i = 0;

    ++i, i %= 7;

    buf = outbuf[i];

    b = OUT_SIZ - 4 - a - len;

    strcpy(buf, orig);

    strncpy(buf + a, pnew, len);

    strncpy(buf + a + len, ptr + strlen(old), b);

    buf[OUT_SIZ - 4] = 0;

    return (buf);

}

const char *str_replace_all(const char *orig, const char *old,
                            const char *newstr)
{

    const char *ptr;

    size_t len;

    if (!strstr(orig, old) || !str_cmp(old, newstr))
    {

        return orig;

    }
    len = strlen(newstr);

    while ((ptr = strstr(orig, old)) && (ptr - orig) + len < (OUT_SIZ - 4))
    {

        orig = str_replace(orig, old, newstr);

    }

    return orig;

}

const char *str_ireplace(const char *orig, const char *old, const char *pnew)
{

    char *buf;

    const char *ptr;

    size_t len, a, b;

    if ((ptr = stristr(orig, old)) == NULL
            || (a = ptr - orig) + (len = strlen(pnew)) >= (OUT_SIZ - 4))
        return orig;

    static char outbuf[7][OUT_SIZ];

    static int i = 0;

    ++i, i %= 7;

    buf = outbuf[i];

    b = OUT_SIZ - 4 - a - len;

    strcpy(buf, orig);

    strncpy(buf + a, pnew, len);

    strncpy(buf + a + len, ptr + strlen(old), b);

    buf[OUT_SIZ - 4] = 0;

    return (buf);

}

const char *str_ireplace_all(const char *orig, const char *old,
                             const char *newstr)
{

    const char *ptr;

    size_t len;

    if (!stristr(orig, old) || !str_cmp(old, newstr))
    {

        return orig;

    }
    len = strlen(newstr);

    while ((ptr = stristr(orig, old))
            && (ptr - orig) + len < (OUT_SIZ - 4))
    {

        orig = str_ireplace(orig, old, newstr);

    }

    return orig;

}

/*
 * Returns an initial-capped string.
 */
const char *capitalize(const char *str)
{

    static char strcap[5][500];

    static int o;

    int i;

    if (!str)
        return str_empty;

    ++o, o %= 5;

    for (i = 0; str[i] != '\0'; i++)
        strcap[o][i] = LOWER(str[i]);

    strcap[o][i] = '\0';

    strcap[o][0] = UPPER(strcap[o][0]);

    return strcap[o];

}

const char *str_dup(const char *str)
{

    if (str == 0 || *str == 0)
        return str_empty;

    char *p = alloc_mem(1, strlen(str) + 1);

    if (p)
    {
        strcpy(p, str);
    }
    return p;

}

void free_str(const char *pstr)
{

    if (pstr == 0 || !*pstr)
        return;

    free_mem((char *)pstr);

    pstr = 0;

}

void free_str_dup(const char **pstr, const char *nstr)
{

    free_str(*pstr);

    *pstr = str_dup(nstr);

}

/*
 * Credit: http://www.secureprogramming.com/?action=view&feature=recipes&recipeid=3
 */
bool is_valid_email(const char *address)
{

    int count = 0;

    const char *c, *domain;

    static char *rfc822_specials = "()<>@,;:\\\"[]";

    /*
     * first we validate the name portion (name@domain)
     */
    for (c = address; *c; c++)
    {

        if (*c == '\"'
                && (c == address || *(c - 1) == '.' || *(c - 1) == '\"'))
        {

            while (*++c)
            {

                if (*c == '\"')
                    break;

                if (*c == '\\' && (*++c == ' '))
                    continue;

                if (*c < ' ' || *c >= 127)
                    return 0;

            }

            if (!*c++)
                return 0;

            if (*c == '@')
                break;

            if (*c != '.')
                return 0;

            continue;

        }
        if (*c == '@')
            break;

        if (*c <= ' ' || *c >= 127)
            return 0;

        if (strchr(rfc822_specials, *c))
            return 0;

    }

    if (c == address || *(c - 1) == '.')
        return 0;

    /*
     * next we validate the domain portion (name@domain)
     */
    if (!*(domain = ++c))
        return 0;

    do
    {

        if (*c == '.')
        {

            if (c == domain || *(c - 1) == '.')
                return 0;

            count++;

        }
        if (*c <= ' ' || *c >= 127)
            return 0;

        if (strchr(rfc822_specials, *c))
            return 0;

    }
    while (*++c);

    return (count >= 1);

}

/*
 * Return true if an argument is completely numeric.
 */
int is_number(const char *arg)
{

    bool precision = false;

    if (!arg || !*arg)
        return 0;

    if (*arg == '+' || *arg == '-')
        arg++;

    for (; *arg != 0; arg++)
    {

        if (*arg == '.')
        {

            precision = true;

            continue;

        }
        if (!isdigit((int)*arg))
            return 0;

    }

    return precision ? 2 : 1;

}

const char *get_line(const char *str, char *buf)
{

    while (str && *str != 0)
    {

        if (*str == '\n')
            break;

        *buf++ = *str++;

    }

    if (str && *str != 0)
    {

        if (*(++str) == '\r')
            str++;

    }
    *buf = 0;

    return str;

}

int count_lines(const char *pstr)
{

    int count = 0;

    while (pstr && *pstr)
    {

        if (*pstr == '\n')
            count++;

        if (*(++pstr) == '\r')
            pstr++;

    }

    return count;

}

const char *one_argument(const char *argument, char *arg)
{

    return first_arg(argument, arg, false);

}

const char *first_arg(const char *argument, char *arg_first, bool fCase)
{

    char cEnd;

    while (*argument == ' ')
        argument++;

    cEnd = ' ';

    if (*argument == '\'' || *argument == '"'
            || *argument == '%' || *argument == '(')
    {

        if (*argument == '(')
        {

            cEnd = ')';

            argument++;

        }
        else
            cEnd = *argument++;

    }
    while (*argument != '\0')
    {

        if (*argument == cEnd)
        {

            argument++;

            break;

        }
        if (fCase)
            *arg_first = LOWER(*argument);

        else
            *arg_first = *argument;

        arg_first++;

        argument++;

    }

    *arg_first = '\0';

    while (*argument == ' ')
        argument++;

    return argument;

}

const char *strip_color(const char *str)
{

    static char out[5][OUT_SIZ];

    static int i;

    ++i, i %= 5;

    char *res = out[i];

    for (const char *pstr = str; *pstr != 0; pstr++)
    {

        if (*pstr != COLOR_CODE)
        {

            *res++ = *pstr;

            continue;

        }
        if (*(++pstr) == 0)
            break;

        if (*pstr == '!')
            pstr++;

        else
            continue;

        if (*pstr == 0)
            break;

    }

    return out[i];

}

int strlen_color(const char *str)
{

    int count = 0;

    for (const char *pstr = str; *pstr != 0; pstr++)
    {

        if (*pstr != COLOR_CODE)
        {

            count++;

            continue;

        }
        pstr++;

        if (*pstr == 0)
            break;

        if (*pstr == '!')
            pstr++;

        else
            continue;

        if (*pstr == 0)
            break;

    }

    return count;

}

static const char *fill_str_len(const char *src, size_t len)
{

    int cz, c, count;
    size_t sz, pos, mod;

    char *result;

    if (!src || !*src || len <= 0)
        return &str_empty[0];

    static char outbuf[7][OUT_SIZ * 2];

    static int i = 0;

    ++i, i %= 7;

    result = outbuf[i];

    result[0] = 0;

    cz = strlen_color(src);

    mod = len % cz;

    len /= cz;

    sz = strlen(src);

    pos = 0;

    count = 0;

    for (c = 0; c < len; c++)
    {

        strcat(result, src);

        pos += sz;

    }

    for (c = 0; c < sz && count < mod; c++, pos++)
    {

        if (src[c] == COLOR_CODE)
        {

            result[pos++] = src[c++];

            if (src[c] == '!')
                result[pos++] = src[c++];

            result[pos] = src[c];

        }
        else
        {

            result[pos] = src[c];

            count++;

        }

    }

    result[pos] = 0;

    return result;

}

const char *align_string(align_t align, int width, const char *color,
                         const char *fill, const char *str)
{

    size_t nCount, cnt, sz;

    static char outbuf[7][OUT_SIZ * 2];

    static int i = 0;

    // rotate buffers
    ++i, i %= 7;

    char *result = outbuf[i];

    if (!fill || !*fill)
        fill = " ";

    sz = strlen_color(str);

    nCount = UMIN(sz, width);

    strcpy(result, color ? color : &str_empty[0]);

    switch (align)
    {

    case ALIGN_RIGHT:

        cnt = (width - ++nCount);

        strcat(result, fill_str_len(fill, width - nCount));

        break;

    case ALIGN_CENTER:

        nCount = (width - nCount) / 2;

        cnt = nCount;

        strcat(result, fill_str_len(fill, nCount));

        break;

    case ALIGN_INDENT:

        nCount = 8;

        cnt = nCount;

        strcat(result, fill_str_len(fill, nCount));

        break;

    default:

        cnt = 0;

        break;

    }

    int pos = strpos(str, width);

    strncat(result, str, pos);

    cnt = UMIN(cnt + sz, width);

    strcat(result, fill_str_len(fill, width - cnt));

    if (color)
        strcat(result, "~x");

    return result;

}

const char *valign_string(align_t align, int width, const char *color,
                          const char *fill, const char *fmt, va_list args)
{

    char buf[OUT_SIZ];

    vsnprintf(buf, sizeof(buf), fmt, args);

    return align_string(align, width, color, fill, buf);

}

int strpos(const char *str, int width)
{

    int i = 0, count = 0;

    for (i = 0; str[i] != 0; i++)
    {

        if (str[i] != COLOR_CODE)
        {

            if (++count >= width)
            {

                return i + 1;

            }
            continue;

        }
        if (str[++i] == '!')
            i++;

    }

    return i;

}

const char *ordinal_string(int n)
{

    static char buffer[3][20];

    static int i;

    ++i, i %= 3;

    char *buf = buffer[i];

    if (n == 1 || n == 0)
        return "first";

    else if (n == 2)
        return "second";

    else if (n == 3)
        return "third";

    else if (n % 10 == 1)
        sprintf(buf, "%dst", n);

    else if (n % 10 == 2)
        sprintf(buf, "%dnd", n);

    else if (n % 10 == 3)
        sprintf(buf, "%drd", n);

    else
        sprintf(buf, "%dth", n);

    return buffer[i];

}

const char *strupper(const char *str)
{

    if (!str)
        return &str_empty[0];

    static char buf[5][OUT_SIZ];

    static int i;

    ++i, i %= 5;

    char *pstr = buf[i];

    while (*str)
    {

        *pstr++ = toupper((int)(*str++));

    }

    return buf[i];

}

const char *fillstr(const char *fill, size_t len)
{

    if (!fill || !*fill)
        fill = "-";

    return fill_str_len(fill, len);

}

bool nullstr(const char *str)
{

    return (str == 0 || *str == 0);

}

/*
 * searches a string for a count identifier
 * and returns the count
 */
long x_argument(char c, const char *argument, char *arg)
{

    const char *p;

    char *q;

    long number;

    p = strchr(argument, c);

    if (p == NULL)
    {

        strcpy(arg, argument);

        return 1;

    }
    number = strtoul(argument, &q, 0);

    if (q != p)
        number = 0;

    strcpy(arg, p + 1);

    return number;

}

/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
long number_argument(const char *argument, char *arg)
{

    return x_argument('.', argument, arg);

}

long multi_argument(const char *argument, char *arg)
{

    return x_argument('*', argument, arg);

}

const char *formatf(const char *fmt, ...)
{

    static char out[5][OUT_SIZ];

    static int i;

    ++i, i %= 5;

    char *buf = out[i];

    va_list args;

    va_start(args, fmt);

    vsnprintf(buf, OUT_SIZ, fmt, args);

    va_end(args);

    return buf;

}
