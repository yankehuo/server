CC = g++
#CFLAGS = -Wl,--no-as-needed -g -pthread -Wall -std=c++11
CFLAGS = -g -pthread -Wall -std=c++11

mkdir -p build/bin
TARGET = server 
OBJ = threadpool/*.cpp server/*.cpp timer/*.cpp http/*.cpp buffer/*.cpp log/*.cpp main.cpp

all:$(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o build/bin/$(TARGET) 

#%.o:%.cpp
#$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean all
clean:
	rm -rf *.o $(TARGET)
	rm -rf build
