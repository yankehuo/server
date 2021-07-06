CC = g++
#CFLAGS = -Wl,--no-as-needed -g -pthread -Wall -std=c++11
CFLAGS = -g -pthread -Wall -std=c++11

TARGET = http
OBJ = httprequest.cpp httpresponse.cpp httpconn.cpp main.cpp ../log/log.cpp ../buffer/buffer.cpp
#OBJ = threadpool

all:$(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) 

#%.o:%.cpp
#$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean all
clean:
	rm -rf *.o $(TARGET)
