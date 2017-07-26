#include "easylogging++.h"
#include "../include/application/server.h"

INITIALIZE_EASYLOGGINGPP

int main()
{
	el::Configurations conf("./logger.conf");
    el::Loggers::reconfigureAllLoggers(conf);
	Server *m_button = Server::GetInstance();
	assert(m_button);
	m_button->Start();
}
