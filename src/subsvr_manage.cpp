#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <string>
#include <map>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "application/subsvr_manage.h"
#include "application/commondata.h"
#include "application/commontools.h"
#include "application/redis_wrap.h"
#include "application/redis_define.h"

static uint32_t COMMON_MINUTESECONDES = 360;

static int s_redis_update_count = 0;
static int s_redis_exit = 0;
static int stps_server_port = 6604;
time_t begin_time = 0;
//通过环境变量获取配置
//和环境变量相关
static char  rediscenter_ip[48] = {0,};
static char  serverarea_name[128] = "Asia:China:Beijing";
static char  vendor_name[128] = "General";

static int  rediscenter_port = 5141;

static void update_serverinfo_to_redis(redisContext* connect)
{
	if (connect)
	{
		if (0 == redis_multi(connect))
		{
			std::string tps_server_key(
				KEY_TPS_KEY_PREFIX +std::string(TPS_SERVER_IP));

			std::string tps_server_port;
			std::string tps_server_status;
			char tps_run_time[48] = {0,};

			time_t cur_time = time(NULL);
			time_t up_time = cur_time - begin_time;
			sprintf(tps_run_time,"%d",(uint32_t)up_time);
			int peer_map_size = get_tpsserver_status();
			std::stringstream ss;
			ss << stps_server_port;
			ss >> tps_server_port;
			ss.clear();

			ss << peer_map_size;
			ss >> tps_server_status;
			ss.clear();

			int ret = redis_hset(connect,KEY_TPS_MAP,
							TPS_SERVER_IP, VALUE_TPS_ONLINE);
			ret += redis_hset(connect,tps_server_key.c_str(),
							FIELD_TPS_IP, TPS_SERVER_IP);
			ret += redis_hset(connect,tps_server_key.c_str(),
							FIELD_TPS_PORT, tps_server_port.c_str());
			ret += redis_hset(connect,tps_server_key.c_str(),
							FIELD_TPS_AREA, serverarea_name);
			ret += redis_hset(connect,tps_server_key.c_str(),
							FIELD_TPS_VENDORNAME, vendor_name);
			ret += redis_hset(connect,tps_server_key.c_str(),
							FIELD_TPS_STATUS,tps_server_status.c_str());
			ret += redis_hset(connect,tps_server_key.c_str(),
							FIELD_TPS_ACTIVEINDEX, tps_server_status.c_str());
			ret += redis_hset(connect,tps_server_key.c_str(),
							FIELD_TPS_RETOK, "0");
			ret += redis_hset(connect,tps_server_key.c_str(),
							FIELD_TPS_RETERROR,"0");
			ret += redis_hset(connect,tps_server_key.c_str(),
							FIELD_TPS_RUNSECONDS, tps_run_time);

			ret += redis_expire(connect,tps_server_key.c_str(), COMMON_MINUTESECONDES);
			if (ret < 0)
			{
				redis_discard(connect);
			}
			else
			{
				redis_exec(connect);
			}
		}
	}
}

static void* redis_update_thread(void* arg)
{
	while(!s_redis_exit)
	{
		redisContext* connect = redis_connect(rediscenter_ip, rediscenter_port);
		
		if (connect)
		{
			update_serverinfo_to_redis(connect);
			redisFree(connect);
			s_redis_update_count++;
			sleep(COMMON_MINUTESECONDES/2);
		}
		else
		{
			if(s_redis_update_count==0)
				sleep(1);
			else
				sleep(COMMON_MINUTESECONDES);
		}
	}
	return NULL;
}

int	start_subsvr_manage()
{
	begin_time = time(NULL);

	char *pEnv = NULL;
	pEnv = getenv("ServerArea");
    if(pEnv)
	{
		strcpy(serverarea_name,pEnv);
	}
	
    pEnv = getenv("VendorName");
	if(pEnv)
	{
		strcpy(vendor_name,pEnv);
	}
		
	pEnv = getenv("RedisCenter");
	if(pEnv)
	{
		strcpy(rediscenter_ip,pEnv);
	}
	else
	{
		strcpy(rediscenter_ip,REDIS_CENTER_IP);
	}
	LOG(DEBUG)<<"vendor_name = "<<vendor_name<<" rediscenter = "<<rediscenter_ip<<" serverarea_name = "<<serverarea_name;
	//创建线程，定期更新数据库中tps的状态信息
	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_create(&tid, &attr, redis_update_thread, NULL);

	int cost = 0;
	while ((s_redis_update_count <= 0)&&(cost < 20))
	{
		LOG(INFO) <<"wait for redis_update";
		sleep(1);
		cost++;
	}
	
	if(cost >= 20)
	{
		LOG(ERROR) <<"wait for redis_update timeout";
		return -1;
	}

	return 0;
}

int	stop_subsvr_manage()
{
	s_redis_exit = 1;
	sleep(1);
	return 0;
}


