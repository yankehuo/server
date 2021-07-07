CC = g++
#CFLAGS = -Wl,--no-as-needed -g -pthread -Wall -std=c++11
CFLAGS = -g -pthread -Wall -std=c++14

TARGET = server 
OBJ = threadpool/threadpool.h \
	  server/webserver.cpp server/epoller.cpp \
	  timer/heaptimer.cpp \
	  http/httpconn.cpp http/httprequest.cpp http/httpresponse.cpp \
	  buffer/buffer.cpp \
	  log/log.cpp config/config.cpp main.cpp

all:$(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o build/$(TARGET) 

#%.o:%.cpp
#$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean all
clean:
	rm -rf *.o $(TARGET)
	rm -rf build/*
