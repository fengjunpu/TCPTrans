LIBS += ../lib/libevent.a
LIBS += ../lib/json_lib/libjson.a
LIBS += ../lib/redis_lib/libhiredis.a

tps_server:main.o server.o commoncb.o device.o commontools.o commondata.o subsvr_manage.o redis_wrap.o
	g++ -std=c++11 -o tps_server main.o server.o commoncb.o device.o commondata.o commontools.o subsvr_manage.o redis_wrap.o $(LIBS) -lrt -lpthread 
main.o:main.cpp
	g++ -std=c++11 -c main.cpp
server.o:server.cpp
	g++ -std=c++11 -c server.cpp
commoncb.o:commoncb.cpp
	g++ -std=c++11 -c commoncb.cpp
device.o:device.cpp
	g++ -std=c++11 -c device.cpp
commontools.o:commontools.cpp
	g++ -std=c++11 -c commontools.cpp
commondata.o:commondata.cpp
	g++ -std=c++11 -c commondata.cpp
subsvr_manage.o:subsvr_manage.cpp
	g++ -std=c++11 -c subsvr_manage.cpp
redis_wrap.o:redis_wrap.cpp
	g++ -std=c++11 -c redis_wrap.cpp
clean:
	rm -rf *.o tps_server
