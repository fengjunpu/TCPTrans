#include "../include/application/commoncb.h"

void listen_accept_cb(struct evconnlistener *lev, evutil_socket_t fd, struct sockaddr *addr, int socklen, void *arg)
{
	if(fd <= 0 || NULL == arg)
		return;

//	LOG(INFO)<<"accept new connect addr:"<<inet_ntoa(((struct sockaddr_in *)addr)->sin_addr);
	struct event_base *base = (struct event_base *)arg;
	Peer *p_node = new Peer;
	assert(p_node);
	struct bufferevent *bufev = bufferevent_socket_new(base,fd,BEV_OPT_CLOSE_ON_FREE);
	if(NULL == bufev)
		return;
	bufferevent_setcb(bufev,bufev_read_cb,NULL,bufev_error_cb,p_node);
	
	update_buffer_timerout(bufev,HEART_BEAT_TIMEOUT);
	
	p_node->p_bufev = bufev;
	int ret = bufferevent_enable(bufev,EV_READ | EV_WRITE);
	if(-1 == ret)
	{
		delete p_node;
		return bufferevent_free(bufev);
	}
}

void listen_error_cb(struct evconnlistener *lev, void *arg)
{
	LOG(ERROR)<<"Listen error dispatch loop will exit";
	struct event_base *base = evconnlistener_get_base(lev);
	event_base_loopexit(base,NULL);
	return;
}

void bufev_read_cb(struct bufferevent *bev, void *ctx)
{
	if(NULL == bev || NULL == ctx)
		return ;
	Peer *pNode = (Peer *)ctx;
	struct evbuffer *buf_in = bufferevent_get_input(bev);
	assert(buf_in);
	size_t buf_len = evbuffer_get_length(buf_in);
	if(0 == buf_len)
		return;
	char *read_msg = (char *)evbuffer_pullup(buf_in,-1);
	if(NULL == read_msg)
			return;
	read_msg[buf_len] = 0;
	
	int Ret_Code = HTTP_RES_BADREQ;
	if(strstr(read_msg,"POST /TransProxy"))
	{
		Ret_Code = pNode->handle_register(bev,pNode,read_msg);
		if(Ret_Code == HTTP_RES_500)
			return;
		if(Ret_Code == HTTP_RES_200)
			evbuffer_drain(buf_in,buf_len);
	}
	else if(strstr(read_msg,"POST /PrivateData"))
	{
		Ret_Code = pNode->handle_transmsg(bev,pNode,read_msg);
	}
	else
	{
		update_buffer_timerout(bev,5);
	}
	
	if(Ret_Code != HTTP_RES_200)
	{
		evbuffer_drain(buf_in,buf_len);
		error_rps_data(bev,Ret_Code);
	}
}

void bufev_error_cb(struct bufferevent *bev, short what, void *ctx)
{
	if(what & BEV_EVENT_EOF)
	{
		LOG(ERROR)<<"connection closed";
	}
	else if(what & BEV_EVENT_ERROR)
	{
		LOG(ERROR)<<"some other error";
	}
	else if(what&BEV_EVENT_TIMEOUT)
	{
		LOG(ERROR)<<"connection timeout";
	}
	else
	{
		LOG(ERROR)<<"unkonwn error:"<<what;
	}

	if(ctx != NULL)
	{
		Peer *pNode = (Peer *)ctx;
		erase_one_peer(pNode->uuid);
		Server *redis_ev = Server::GetInstance();
		if((redis_ev->redis_conn_flag == 1) && (pNode->rfulsh_time != -1))
		{
			redisAsyncCommand(redis_ev->redis_pconn,redis_op_status,NULL,"DEL %s",pNode->uuid.c_str());	
		}
		LOG(ERROR)<<"connect error uuid: "<<pNode->uuid;
		delete(pNode);
	}
	
	if(NULL != bev)
		bufferevent_free(bev);
	return ;
}

/*redis 相关*/
void redis_conn_cb(const struct redisAsyncContext* c, int status)
{
	if(NULL == c)
		return;
	Server *redis_ev = Server::GetInstance();
	struct event_base *s_base = redis_ev->GetEvBase();
	if(status != REDIS_OK)
	{
		redis_ev->redis_conn_flag = 0;
		if(event_initialized(&redis_ev->redis_timer))
			event_del(&redis_ev->redis_timer);
		event_assign(&redis_ev->redis_timer,s_base,-1,0,redis_reconn_cb,NULL);
		update_timer_event(&redis_ev->redis_timer,REDIS_RECONN_INTERNAL);
	}
	else
	{
		redis_ev->redis_conn_flag = 1;
		if(event_initialized(&redis_ev->redis_timer))
			event_del(&redis_ev->redis_timer);
		event_assign(&redis_ev->redis_timer,s_base,-1,0,redis_check_health_cb,NULL);
		update_timer_event(&redis_ev->redis_timer,REDIS_CHECKHEALTH_INTERNAL);
	}
}

void redis_disconn_cb(const struct redisAsyncContext* c, int status)
{
	LOG(ERROR)<<"redis disconn need to reconnect";
	Server *redis_ev = Server::GetInstance();
	struct event_base *s_base = redis_ev->GetEvBase();
	redis_ev->redis_conn_flag = 0;
	if(event_initialized(&redis_ev->redis_timer))
		event_del(&redis_ev->redis_timer);
	event_assign(&redis_ev->redis_timer,s_base,-1,0,redis_reconn_cb,NULL);
	update_timer_event(&redis_ev->redis_timer,REDIS_RECONN_INTERNAL);
}

/*重连redis*/
void redis_reconn_cb(evutil_socket_t fd, short event, void *ctx)
{
	Server *redis_ev = Server::GetInstance();
	struct event_base *s_base = redis_ev->GetEvBase();
	redis_ev->redis_pconn  = redisAsyncConnect(REDIS_CENTER_IP,REDIS_STATUS_PORT);
	redisLibeventAttach(redis_ev->redis_pconn,s_base);
	redisAsyncSetConnectCallback(redis_ev->redis_pconn,redis_conn_cb);
	redisAsyncSetDisconnectCallback(redis_ev->redis_pconn,redis_disconn_cb);
	return;
}

/*redis健康检查*/
void redis_check_health_cb(evutil_socket_t fd, short event, void *ctx)
{
	Server *redis_ev = Server::GetInstance();
	if(redis_ev->redis_conn_flag == 1)
	{
		redisAsyncCommand(redis_ev->redis_pconn,redis_op_status,NULL, "SET Test Test");
	}
	return ;
}

void redis_op_status(redisAsyncContext *c, void * redis_reply, void * arg)
{
	Server *redis_ev = Server::GetInstance();
	redisReply * reply   = (redisReply *)redis_reply;
    if(NULL == reply)
	{
		LOG(ERROR)<<"redis health check connection disconected";
		redisAsyncDisconnect(c);
		return;
	}
	update_timer_event(&redis_ev->redis_timer,REDIS_CHECKHEALTH_INTERNAL);
}

