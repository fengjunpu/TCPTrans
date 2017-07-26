#ifndef  _SERVER_H_
#define _SERVER_H_

#include <assert.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>
#include <pthread.h>

#include "../event2/event.h"
#include "../event2/listener.h"
#include "./commoncb.h"
#include "./device.h"
#include "../hredis/hiredis.h"
#include "../hredis/async.h"
#include "../event2/event_struct.h"
#include "./commondata.h"
#include "../hredis/adapters/libevent.h"

#define LISTEN_LIST 8192
#define LISTEN_ADDR "0.0.0.0"
#define LISTEN_PORT 6615

class Peer;

class Server{
public:
	static Server *GetInstance();
	bool Start();
	bool Stop();
	struct event_base *GetEvBase();
	std::map<std::string,Peer *> peer_node_map;
	pthread_mutex_t s_lock_node_map;
	int redis_conn_flag; 	
	redisAsyncContext *redis_pconn;
	struct event redis_timer;
private:
	static Server *s_server;
	struct event_base *s_base;
	struct evconnlistener *s_lev;
	int s_fd;
	int s_flag;
	Server():s_flag(0),s_base(NULL),s_lev(NULL),redis_conn_flag(0),redis_pconn(NULL){}
};

#endif 
