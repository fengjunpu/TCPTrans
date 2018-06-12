#include "../include/application/commontools.h"

int get_recv_buflen(int fd)
{
	int rcvbuf_len;
	int len = sizeof(rcvbuf_len);
	getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (void *)&rcvbuf_len, (socklen_t *)&len);
	return rcvbuf_len;
}

static std::string getString(const Json::Value & table)
{
        std::string temp;
        Json::StyledWriter writer(temp);
        writer.write(table);
        return temp;
}

int error_rps_data(struct bufferevent *bufev,int code)
{
	if(NULL == bufev)
		return -1;
	char rps[128] = {0};
	int ret = snprintf(rps,128,"HTTP/1.1 %d %s\r\n",code,status_code_to_str(code));
	sprintf(rps+ret,"%s","Content-Type: text/plain\r\n\r\n");
	int len = strlen(rps) + 1;
	bufferevent_write(bufev,rps,len);
	return 0;
}

char *parse_regist_request(char *msg)
{
	if(NULL == msg)
		return NULL;
	char *pBody = strstr(msg,"\r\n\r\n");
	if(pBody != NULL)
		pBody += 4;
	return pBody;
}

int parse_trans_request(const char *msg,std::string &srcid,std::string &destid)
{
	if(NULL == msg)
		return -1;
	std::string dest_flag = "DestUuid:";
	std::string src_flag = "SrcUuid:";
	
	const char *DestUuid = strstr(msg,dest_flag.c_str());
	if(NULL == DestUuid) {
		return -1;
	}
	
	DestUuid += dest_flag.length();
	while(DestUuid && *DestUuid == ' ') {
		DestUuid += 1;
	}
	const char *DestUuidEnd = strstr(DestUuid,"\r\n");
	if(NULL == DestUuidEnd) {
		return -1;
	}
	int dest_len = DestUuidEnd - DestUuid;
	std::string t_destid(DestUuid,dest_len);
	destid = t_destid;
	
	const char *SrcUuid = strstr(msg,src_flag.c_str());
	if(NULL == SrcUuid) {
		return -1;
	}
	
	SrcUuid += src_flag.length();
	while(SrcUuid && *SrcUuid == ' ') {
		SrcUuid += 1;
	}
	
	const char *SrcUuidEnd = strstr(SrcUuid,"\r\n");
	if(NULL == SrcUuidEnd) {
		return -1;
	}
	int src_len = SrcUuidEnd - SrcUuid;
	std::string t_srcid(SrcUuid,src_len);
	srcid = t_srcid;
	LOG(DEBUG)<<"srcid = "<<srcid<<"  destid = "<<destid;
	return 0;
}

int make_regist_response(std::string &rsp)
{
	rsp.clear();
	char sectim[32] = {0,};
	time_t now;
	time(&now);
	sprintf(sectim,"%ld",now);
	
	char keepalive_time[32] = {0,};
	sprintf(keepalive_time,"%d",HEATER_BEAT_INTERNAL);
	
	Json::Value responseValue = Json::Value::null;
	responseValue["TransProxy"]["Body"]["KeepAliveIntervel"] = keepalive_time;
	responseValue["TransProxy"]["Body"]["UtcTime"] = sectim;
	responseValue["TransProxy"]["Header"]["ErrorNum"] = "200";
	responseValue["TransProxy"]["Header"]["ErrorString"] = "Success OK";
	responseValue["TransProxy"]["Header"]["MessageType"] = "MSG_TRANSPROXY_REGISTER_RSP";
	responseValue["TransProxy"]["Header"]["Version"] = "1.0";
	std::string strbody = getString(responseValue);
	int bodylength = strbody.length();
	std::stringstream ss;
	std::string strlen;
	ss << bodylength;
	ss >> strlen;
	ss.clear();
	
	rsp += "HTTP/1.1 200 OK\r\n";
	rsp += "Content-Encoding: UTF-8\r\n";
	rsp += "Content-Length: "+strlen+"\r\n";
	rsp += "Content-Type: text/plain\r\n\r\n";
	rsp += strbody;
	
	int msg_len = rsp.length();
	return msg_len;
}

int insert_one_peer(std::string &uuid,Peer *pNode)
{
	int length = uuid.length();
	if(0 == length || NULL == pNode)
		return -1;
	
	Server *pServer = Server::GetInstance();
	assert(pServer);
	pthread_mutex_lock(&pServer->s_lock_node_map);

	Peer_Map::iterator dest_it = pServer->peer_node_map.find(uuid);
	if(dest_it != pServer->peer_node_map.end())
	{
		Peer *m_node = dest_it->second;
		if(m_node != NULL)
		{
			pServer->peer_node_map.erase(dest_it);
			if(m_node->p_bufev != NULL)
				bufferevent_free(m_node->p_bufev);
			delete m_node;
		}
	}
	
	pServer->peer_node_map.insert(Peer_Map::value_type(uuid,pNode));
	pthread_mutex_unlock(&pServer->s_lock_node_map);
	return 0;
}

Peer *get_one_peer(std::string &uuid)
{
	Peer *pNode = NULL;
	int length = uuid.length();
	if(length > 0)
	{
		Server *pServer = Server::GetInstance();
		assert(pServer);
		pthread_mutex_lock(&pServer->s_lock_node_map);
		Peer_Map::iterator dest_it = pServer->peer_node_map.find(uuid);
		if(dest_it != pServer->peer_node_map.end())
			pNode = dest_it->second;
		pthread_mutex_unlock(&pServer->s_lock_node_map);
	}
	return pNode;
}

bool erase_one_peer(std::string &uuid)
{
	int length = uuid.length();
	if(length > 0)
	{
		Server *pServer = Server::GetInstance();
		assert(pServer);
		pthread_mutex_lock(&pServer->s_lock_node_map);
		pServer->peer_node_map.erase(uuid);
		pthread_mutex_unlock(&pServer->s_lock_node_map);
	}
	return true;
}

bool update_timer_event(struct event *ev,int time)
{
	if(NULL == ev)
		return false;
	struct timeval tv;	
	evutil_timerclear(&tv);	
	tv.tv_sec = time;
	tv.tv_usec = 0;
	event_add(ev,&tv);
	return true;
}

bool update_buffer_timerout(struct bufferevent *bev,int tm)
{
	struct timeval tv;
	tv.tv_sec = tm;
	tv.tv_usec = 0;
	bufferevent_set_timeouts(bev,&tv,NULL);	
	return true;
}

int get_tpsserver_status()
{
	Server *pServer = Server::GetInstance();
	int size = 0;
	pthread_mutex_lock(&pServer->s_lock_node_map);
	size = pServer->peer_node_map.size();
	pthread_mutex_unlock(&pServer->s_lock_node_map);
	return size;
}
