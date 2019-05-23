#ifndef _DEVICE_H_
#define _DEVICE_H_
#include <map>

#include "event2/bufferevent.h"
#include "event2/bufferevent_struct.h"
#include "./commontools.h"
#include "../easylogging++.h"

class Peer{
public:
	Peer():p_bufev(NULL),rfulsh_time(-1){}
	int handle_transmsg(struct bufferevent *,Peer *,char *);
	int handle_register(struct bufferevent *,Peer *,char *);
	struct bufferevent *p_bufev;
	std::string uuid;
	int rfulsh_time;
};

typedef std::map<std::string,Peer *> Peer_Map;

#endif