#ifndef _DEVICE_H_
#define _DEVICE_H_
#include "./commontools.h"
#include "../easylogging++.h"
#include <map>

class Peer{
public:
	Peer():p_bufev(NULL),rfulsh_time(-1){}
	void handle_register(struct bufferevent *bufev,Peer *pNode,char *msg);
	int handle_transmsg(struct bufferevent *,char *,int);
	struct bufferevent *p_bufev;
	std::string uuid;
	int rfulsh_time;
};

typedef std::map<std::string,Peer *> Peer_Map;

#endif