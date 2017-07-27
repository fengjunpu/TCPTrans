#include "../include/application/server.h"

Server *Server::s_server = NULL;

Server *Server::GetInstance()
{
	if(NULL == s_server)
	{
		s_server = new Server;
		assert(s_server);
	}
	return s_server;
}

bool Server::Start()
{
	if(1 == s_flag)
		return false;
	
	s_flag = 1; 
	peer_node_map.clear();
	pthread_mutex_init(&s_lock_node_map,NULL);
	
	s_base = event_base_new();
	assert(s_base);

	s_fd = socket(AF_INET,SOCK_STREAM,0);
	assert(s_fd>0);
	
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(struct sockaddr_in));
	int socklen = sizeof(struct sockaddr_in);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(TPS_SERVER_PORT);
	addr.sin_addr.s_addr = inet_addr(LISTEN_ADDR);
	
	s_lev = evconnlistener_new_bind(s_base,listen_accept_cb,s_base,LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE,LISTEN_LIST,\
									(const struct sockaddr *)&addr,socklen);
	assert(s_lev);
	evconnlistener_set_error_cb(s_lev,listen_error_cb);
	
	redis_pconn = redisAsyncConnect(REDIS_CENTER_IP,REDIS_STATUS_PORT);
	redisLibeventAttach(redis_pconn,s_base);
	redisAsyncSetConnectCallback(redis_pconn,redis_conn_cb);
	redisAsyncSetDisconnectCallback(redis_pconn,redis_disconn_cb);

	event_base_dispatch(s_base);
}

struct event_base *Server::GetEvBase()
{
	return s_base;	
}

bool Server::Stop()
{
	
}
