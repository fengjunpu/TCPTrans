#ifndef _DEVICE_H_
#define _DEVICE_H_
#include "./commontools.h"
#include "../easylogging++.h"
#include <map>

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