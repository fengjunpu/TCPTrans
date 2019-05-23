#ifndef _FUNC_CB_
#define _FUNC_CB_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include "event2/bufferevent.h"

#include "event2/listener.h"
#include "easylogging++.h"
#include "event2/buffer.h"
#include "device.h"


void listen_accept_cb(struct evconnlistener *lev, evutil_socket_t fd, struct sockaddr *addr, int socklen, void *arg);
void listen_error_cb(struct evconnlistener *lev, void *arg);

void bufev_read_cb(struct bufferevent *bev, void *ctx);
void bufev_error_cb(struct bufferevent *bev, short what, void *ctx);

void redis_conn_cb(const struct redisAsyncContext* c, int status);
void redis_disconn_cb(const struct redisAsyncContext* c, int status);
void redis_reconn_cb(evutil_socket_t fd, short event, void *ctx);
void redis_check_health_cb(evutil_socket_t fd, short event, void *ctx);
void redis_op_status(redisAsyncContext *c, void * redis_reply, void * arg);

#endif
