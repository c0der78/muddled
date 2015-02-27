
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

#ifndef CLIENT_H
#define CLIENT_H

typedef struct Client Client;

#include <libwebsockets.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include "../engine.h"
#include "../account.h"
#include <stdarg.h>
#include "olc.h"

struct Client
{

    // the order of the function pointers is so we can cast
    // to an interface in the library
    void (*handler) (Client *, const char *);

    void (*writelnf) (Client *, const char *, ...);

    void (*writeln) (Client *, const char *);

    void (*writef) (Client *, const char *, ...);

    void (*write) (Client *, const char *);

    void (*page) (Client *, const char *);

    void (*vwrite) (Client *, const char *, va_list);

    void (*titlef) (Client *, const char *, ...);

    void (*title) (Client *, const char *);

    void (*vtitle) (Client *, const char *, va_list);

    bool(*is_playing) (Client *);

    short scrHeight;

    short scrWidth;

    const char *termType;

    const char *host;

    Account *account;

    Client *next;

    int socket;

    struct sockaddr_in addr;

    struct libwebsocket *websocket;

    char lastCommand[BUF_SIZ];

    unsigned char inbuf[4 * ARG_SIZ];

    char incomm[ARG_SIZ];

    char *outbuf;

    int outsize;

    size_t outtop;

    bool fCommand;

    short repeat;

    short password_retries;

    char *showstr_head;

    const char *showstr_point;

    bool(*readln) (Client *);

    Editor *editing;

};

extern Client *first_client;

Client *new_client();

void close_client(Client *);

void destroy_client(Client *);

void initialize_client(int);

void set_playing(Client *);

bool client_is_playing(Client *);

const char *getip(const Client *);

void client_get_account_name(Client *, const char *);

void client_read_to_buffer(Client *);

bool read_line_from_client(Client *);

bool parse_input_buffer(Client *);

void client_account_menu(Client *, const char *);

void client_display_account_menu(Client *);

void client_command_parser(Client *, const char *);

void draw_line(Client *);

void client_delete_char(Connection *);

bool process_output(Client *, bool);

void bust_a_prompt(Client *);

#endif              /* // #ifndef CONNECTION_H */
