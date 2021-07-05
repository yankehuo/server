CC = g++
CFLAGS = -Wall -g -std=c++11

OBJ = locker
all:$(OBJ)
#$(OBJ): $(OBJ).o
#$(OBJ): *.o
#	$(CC) $(CFLAGS) $^ -o $@
%.o : %.cpp
	$(CC) $(CFLAGS) -c $<  -o $@

.PHONY:clean all
clean:
	rm -rf *.o $(OBJ)
