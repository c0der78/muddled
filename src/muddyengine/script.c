
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

#include <stdlib.h>
#include <muddyengine/script.h>
#include <muddyengine/log.h>

#ifdef HAVE_LUA

lua_State *lua_instance = 0;

void init_lua()
{

    lua_instance = lua_open();

    luaopen_base(lua_instance);

    luaopen_string(lua_instance);

}

void close_lua()
{

    if (lua_instance != 0)
    {

        log_info("closing lua");

        lua_close(lua_instance);

        lua_instance = 0;

    }
}
#else

void init_lua()
{
    log_info("No lua scripting support");
}

void close_lua()
{

}

#endif
