#include "../include/application/device.h"

int Peer::handle_register(struct bufferevent *bufev,Peer *pNode,char *msg)
{
	if(NULL == pNode || NULL == msg)
		return HTTP_RES_BADREQ;
	
	char *body = parse_regist_request(msg);
	if(NULL == body)
		return HTTP_RES_BADREQ;
	
	Json::Reader	reader;
	Json::Value 	requestValue;
	if(reader.parse(body, requestValue) == false)
	{
			LOG(ERROR) <<"invalid register request";
			return HTTP_RES_BADREQ;
	}

	if((requestValue.isObject())&&(requestValue.isMember("TransProxy")) \
		&&(requestValue["TransProxy"].isMember("Header")) \
		&&(requestValue["TransProxy"].isMember("Body")) \
		&&(requestValue["TransProxy"]["Header"].isMember("MessageType")) \
        &&(requestValue["TransProxy"]["Body"].isMember("SerialNumber")) \
        &&(requestValue["TransProxy"]["Header"].isMember("TerminalType")))
    {
		std::string uuid = requestValue["TransProxy"]["Body"]["SerialNumber"].asString();
		std::string msgtype = requestValue["TransProxy"]["Header"]["MessageType"].asString();
		
		if(msgtype != "MSG_TRANSPROXY_REGISTER_REQ")
		{
			LOG(ERROR)<<"correct: MSG_TRANSPROXY_REGISTER_REQ"<<"  error:"<<msgtype;
			return HTTP_RES_BADREQ;
		}
		Peer *pPeer = get_one_peer(uuid);
		if(NULL == pPeer && (pNode->uuid.length() != 0))
		{ 
			LOG(ERROR)<<"uuid :"<<uuid<<" Unknown ERROR";
			if(pNode->p_bufev != NULL)
				bufferevent_free(pNode->p_bufev);
			delete pNode;
			return HTTP_RES_500;
		}
		else if(NULL == pPeer || pPeer != pNode)
		{
			pNode->uuid = uuid;
			pNode->rfulsh_time = 0;
			pPeer = pNode;
			insert_one_peer(uuid,pNode);
		}
		std::string terminalType = requestValue["TransProxy"]["Header"]["TerminalType"].asString();
		/*更新数据库*/
		struct timeval nowtv;
		evutil_gettimeofday(&nowtv, NULL);
		Server *conn_redis = Server::GetInstance();
		if(conn_redis->redis_conn_flag == 1 &&(nowtv.tv_sec - pPeer->rfulsh_time > 3*HEATER_BEAT_INTERNAL - 5))
		{
			redisAsyncCommand(conn_redis->redis_pconn,redis_op_status,NULL, "HMSET %s TerminalType %s ServerIP %s", \
	                                        uuid.c_str(),terminalType.c_str(),TPS_SERVER_IP);
	        redisAsyncCommand(conn_redis->redis_pconn,redis_op_status,NULL,"EXPIRE %s %d",uuid.c_str(),REDIS_EXPIRE_TIME);
			pPeer->rfulsh_time = nowtv.tv_sec;
		}
		std::string rps;
		int rps_len = make_regist_response(rps);
		bufferevent_write(bufev,rps.c_str(),rps_len);
		return HTTP_RES_200;
	}
	return HTTP_RES_BADREQ;
}

int Peer::handle_transmsg(struct bufferevent *bufev,Peer *pNode,char *msg)
{
	if(NULL == bufev || NULL == msg)
		return HTTP_RES_BADREQ;
	std::string source_uuid;
	std::string dest_uuid;
	int ret = parse_trans_request(msg,source_uuid,dest_uuid);
	if(ret != 0)
		return HTTP_RES_BADREQ;
	
	Peer *des_pNode = get_one_peer(dest_uuid);
	if(NULL == des_pNode || NULL == des_pNode->p_bufev)
		return HTTP_RES_NOTFOUND;

	Peer *src_pNode = get_one_peer(source_uuid);
	if((src_pNode == NULL) || (src_pNode != NULL && src_pNode->p_bufev != bufev))
	{
		pNode->uuid = source_uuid;
		pNode->rfulsh_time = -1;
		insert_one_peer(source_uuid,pNode);
	}
	else if(des_pNode->p_bufev == src_pNode->p_bufev)
	{
		return HTTP_RES_BADREQ;
	}
	
	error_rps_data(bufev,HTTP_RES_200);
	
	bufferevent_write_buffer(des_pNode->p_bufev,bufev->input);
	
	return HTTP_RES_200;
}
