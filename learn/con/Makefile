CC = g++
CFLAGS = -Wl,--no-as-needed -pthread -Wall -g -std=c++11

TARGET = hello1 
OBJ = hello1.cpp
all:$(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)
#all:$(OBJ)
#$(CC) $(CFLAGS) *.o  -o $(TARGET)
#$(OBJ): $(OBJ).o
#$(OBJ): *.o
#	$(CC) $(CFLAGS) $^ -o $@
#%.o:%.cpp
#	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean all
clean:
	rm -rf *.o $(TARGET)
