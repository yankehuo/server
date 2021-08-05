CC = g++
CFLAGS = -g -O2 -pthread -Wall -std=c++14

TARGET = tinyserver 
OBJ = threadpool/threadpool.h \
	  server/webserver.cpp server/epoller.cpp \
	  timer/heaptimer.cpp \
	  http/httpconn.cpp http/httprequest.cpp http/httpresponse.cpp \
	  buffer/buffer.cpp \
	  logbq/log.cpp config/config.cpp main.cpp

all:$(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) 

#%.o:%.cpp
#$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean all
clean:
	rm -rf *.o $(TARGET)
	rm -rf log
