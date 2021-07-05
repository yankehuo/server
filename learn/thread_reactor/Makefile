CC = g++
#CFLAGS = -Wl,--no-as-needed -g -pthread -Wall -std=c++11
CFLAGS = -g -pthread -Wall -std=c++11

TARGET = threadpool
OBJ = threadpool.cpp
#OBJ = threadpool

all:$(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) 

#%.o:%.cpp
#$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean all
clean:
	rm -rf *.o $(TARGET)
