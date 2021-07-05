CC = g++
CFLAGS = -Wall -g -std=c++11

TARGET = server
OBJ = processpool_cgi
all:$(OBJ)
	$(CC) $(CFLAGS) *.o  -o $(TARGET)
#$(OBJ): $(OBJ).o
#$(OBJ): *.o
#	$(CC) $(CFLAGS) $^ -o $@
%.o : %.cpp
	$(CC) $(CFLAGS) -c $<  -o $@

.PHONY:clean all
clean:
	rm -rf *.o $(OBJ)
