#include <unistd.h>
#include "../include/application/easylogging++.h"
#include "../include/application/server.h"
#include "../include/application/subsvr_manage.h"

INITIALIZE_EASYLOGGINGPP

static const char * optstr = "hi:s:a:p:f:c:d:v:t:e:";
static const char * help   =
	"Options: \n"
	"  -h         : This help text\n"
	"  -i 	<str> : Local Http Server IP \n"
	"  -s 	<int> : Local Http Server Port \n"
	"  -a 	<str> : Status Redis IP\n"
	"  -p 	<int> : Status Redis Port\n"
	"  -c 	<str> : Authcode Redis IP\n"
	"  -d 	<int> : Authcode Redis Port\n"
	"  -v 	<int> : Peer Keepalive Interval\n"
	"  -t 	<int> : Peer Keepalive Timeout\n"
	"  -e	<int> : Redis ExpireTime\n";

static int parse_args(int argc, char ** argv) 
{
	extern char * optarg;
	int           c;
	while ((c = getopt(argc, argv, optstr)) != -1) {
	    switch (c) {
	        case 'h':
	            printf("Usage: %s [opts]\n%s", argv[0], help);
	            return -1;
	        case 'i':
				memcpy(TPS_SERVER_IP,strdup(optarg),sizeof(TPS_SERVER_IP));
	            break;
	        case 's':
	            TPS_SERVER_PORT = atoi(optarg);
	            break;
			case 'a':
				memcpy(REDIS_CENTER_IP,strdup(optarg),sizeof(REDIS_CENTER_IP));
	            break;
			case 'p':
				REDIS_STATUS_PORT = atoi(optarg);
	            break;
			case 'f':
				memcpy(REDIS_CENTER_IP,strdup(optarg),sizeof(REDIS_CENTER_IP));
	            break;
			case 'c':
				memcpy(REDIS_CENTER_IP,strdup(optarg),sizeof(REDIS_CENTER_IP));
	            break;
			case 'd':
				REDIS_AUTH_PORT = atoi(optarg);
	            break;
	        case 'v':
	            HEATER_BEAT_INTERNAL = atoi(optarg);
	            break;
	        case 't':
	            HEART_BEAT_TIMEOUT = atoi(optarg);
	            break;
	        case 'e':
	            REDIS_EXPIRE_TIME = atoi(optarg);
	            break;
	        default:
	            return -1;
	    }
	}
	return 0;
} 

int main(int argc,char ** argv)
{
	el::Configurations conf("./logger.conf");
    el::Loggers::reconfigureAllLoggers(conf);
	parse_args(argc,argv);
#if 0
	int Ret = start_subsvr_manage();
	assert(Ret == 0);
#endif
	Server *m_button = Server::GetInstance();
	assert(m_button);
	m_button->Start();
}
