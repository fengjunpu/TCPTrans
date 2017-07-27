#ifndef _COMMONTOOLS_H_
#define _COMMONTOOLS_H_

#include <cstring>
#include <string>
#include <sstream>
#include "../json/json.h"
#include "./device.h"
#include "./server.h"

class Peer;

char *parse_regist_request(char *msg);
int make_regist_response(std::string &rsp);

int parse_trans_request(const char *msg,std::string &srcid,std::string &destid);

int insert_one_peer(std::string &,Peer *);
Peer *get_one_peer(std::string &);
bool erase_one_peer(std::string &);
int get_recv_buflen(int fd);
bool update_timer_event(struct event *ev,int time);
bool update_buffer_timerout(struct bufferevent *bev,int tm);
int error_rps_data(struct bufferevent *bufev,int code);
int get_tpsserver_status();

#endif
