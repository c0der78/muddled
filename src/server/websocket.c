
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <syslog.h>
#include "client.h"
#include "websocket.h"

short websocket_port = 0;
struct lws_context *websocket_context = NULL;

extern void lws_set_log_level(int level, void (*)(int level, const char *line));
extern int log_level;

Client *first_websocket;

typedef struct {
  Client *client;
} websocket_user_info;

static int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
  return 0;
}

int websocket_server_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
  websocket_user_info *info = (websocket_user_info *)user;

  switch (reason) {
    case LWS_CALLBACK_ESTABLISHED:  // just log message that someone is connecting
    {
      info->client = new_client();
      info->client->websocket = wsi;

      LINK(first_websocket, info->client, next);

      if (greeting == 0) {
        xwritelnf(info->client, "Welcome to %s!", engine_info.name);
        xwriteln(info->client, "");
      }

      else {
        xwriteln(info->client, greeting);
      }
      xwrite(info->client, "Login: ");
      lws_callback_on_writable(wsi);
      break;
    }
    case LWS_CALLBACK_RECEIVE:  // the funny part
    {
      (*info->client->handler)(info->client, (char *)in);
      /*info->conn->buffered_writer::xwriteln();
      info->conn->process_input((char *) in);*/
      lws_callback_on_writable(wsi);
      break;
    }
    case LWS_CALLBACK_SERVER_WRITEABLE: {
      // show prompt
      // write output
      process_output(info->client, true);
      break;
    }
    default:
      break;
  }

  return 0;
}

static struct lws_protocols protocols[] = {
    /* first protocol must always be HTTP handler */
    {
        "http-only",    // name
        callback_http,  // callback
        0               // per_session_data_size
    },
    {"muddy-protocol",            // protocol name - very important!
     &websocket_server_callback,  // callback
     sizeof(websocket_user_info)},
    {
        NULL, NULL, 0 /* End of list */
    }};

bool write_to_websocket(struct lws *websocket, char *txt, size_t len) {
  if (!websocket) {
    return false;
  }

  if (!txt || !*txt || len == 0) {
    return true;
  }
  /*unsigned char *buf = (unsigned char *) malloc(LWS_SEND_BUFFER_PRE_PADDING + len +
                       LWS_SEND_BUFFER_POST_PADDING);
  int i;
  // pointer to `void *in` holds the incomming request
  // we're just going to put it in reverse order and put it in `buf` with
  // correct offset. `len` holds length of the request.

  for (i = 0; i < len; i++)
  {
      buf[LWS_SEND_BUFFER_PRE_PADDING + (len - 1) - i ] = ((char *) txt)[i];
  }
  // send response
  // just notice that we have to tell where exactly our response starts. That's
  // why there's `buf[LWS_SEND_BUFFER_PRE_PADDING]` and how long it is.
  // we know that our response has the same length as request because
  // it's the same message in reverse order.
  int status = lws_write(websocket, &buf[LWS_SEND_BUFFER_PRE_PADDING], len, LWS_WRITE_TEXT);
  // release memory back into the wild
  free(buf);*/
  int status = lws_write(websocket, (unsigned char *)txt, len, LWS_WRITE_TEXT);
  return status > 0;
}

static void lwsl_syslog(int level, const char *msg) {
  switch (level) {
    case LLL_ERR:
      syslog(LOG_ERR, "%s", msg);
      break;
    case LLL_WARN:
    case LLL_NOTICE:
      syslog(LOG_NOTICE, "%s", msg);
      break;
    case LLL_INFO:
      syslog(LOG_INFO, "%s", msg);
      break;
    default:
      syslog(LOG_DEBUG, "%s", msg);
      break;
  }
}

struct lws_context *create_websocket(int port) {
  struct lws_context_creation_info info;
  memset(&info, 0, sizeof info);
  info.port = port;
  info.iface = NULL;
  info.protocols = protocols;
  info.extensions = NULL;
  info.ssl_cert_filepath = NULL;
  info.ssl_private_key_filepath = NULL;
  info.gid = -1;
  info.uid = -1;
  info.options = 0;
  info.user = NULL;
  lws_set_log_level(LLL_NOTICE, lwsl_syslog);
  return lws_create_context(&info);
}
