#include "../include/application/device.h"

void Peer::handle_register(struct bufferevent *bufev,Peer *pNode,char *msg)
{
	if(NULL == pNode || NULL == msg)
		return ;
	
	char *body = parse_regist_request(msg);
	if(NULL == body)
		return ;

	LOG(ERROR)<<"body = "<<body;
	Json::Reader	reader;
	Json::Value 	requestValue;
	if(reader.parse(body, requestValue) == false)
	{
			LOG(ERROR) <<"invalid register request";
			error_rps_data(bufev,HTTP_RES_BADREQ);
			return ;
	}

	if((requestValue.isObject())&&(requestValue.isMember("TransProxy")) \
		&&(requestValue["TransProxy"].isMember("Header")) \
		&&(requestValue["TransProxy"].isMember("Body")) \
		&&(requestValue["TransProxy"]["Header"].isMember("MessageType")) \
        &&(requestValue["TransProxy"]["Body"].isMember("SerialNumber")) \
        &&(requestValue["TransProxy"]["Body"].isMember("TerminalType")))
    {
		std::string uuid = requestValue["TransProxy"]["Body"]["SerialNumber"].asString();
		std::string msgtype = requestValue["TransProxy"]["Header"]["MessageType"].asString();
		
		if(msgtype != "MSG_TRANSPROXY_REGISTER_REQ")
		{
			LOG(ERROR)<<"correct: MSG_TRANSPROXY_REGISTER_REQ"<<"  error:"<<msgtype;
			return;
		}
		
		Peer *pPeer = get_one_peer(uuid);
		if(NULL == pPeer && (pNode->uuid.length() != 0))
		{
			LOG(ERROR)<<"uuid :"<<uuid<<" Unknown ERROR";
			if(pNode->p_bufev != NULL)
				bufferevent_free(pNode->p_bufev);
			delete pNode;
			return;
		}
		else if(NULL == pPeer || pPeer != pNode)
		{
			pNode->uuid = uuid;
			pNode->rfulsh_time = 0;
			insert_one_peer(uuid,pNode);
		}
		std::string terminalType = requestValue["TransProxy"]["Body"]["TerminalType"].asString();

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
		LOG(DEBUG)<<"rps len = "<<rps_len<<" response body = \n"<<rps;
		bufferevent_write(bufev,rps.c_str(),rps_len);
		return;
	}
	error_rps_data(bufev,HTTP_RES_BADREQ);
	return;
}

int Peer::handle_transmsg(struct bufferevent *bufev,char *msg,int len)
{
	if(NULL == bufev || NULL == msg)
		return HTTP_RES_BADREQ;
	std::string source_uuid;
	std::string dest_uuid;
	int ret = parse_trans_request(msg,source_uuid,dest_uuid);
	if(ret != 0)
		return HTTP_RES_BADREQ;

	Peer *pNode = get_one_peer(dest_uuid);
	if(NULL == pNode || NULL == pNode->p_bufev)
		return HTTP_RES_NOTFOUND;
	
	struct evbuffer *pOutPut = bufferevent_get_output(pNode->p_bufev);
	bufferevent_write_buffer(bufev,pOutPut);
	return HTTP_RES_200;
}
