#ifndef THREADPOOL
#define THREADPOOL

//
#include <sys/types.h>
//
#include <sys/socket.h>
//
#include <netinet/in.h>
//
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <signal.h>
//
#include <sys/wait.h>
//
#include <sys/stat.h>
#include <pthread.h>
//
#include <set>

class thread {
public:
	pthread_t m_tid;
	int pipefd[2];
};

template <typename T>
class threadpool {
private:
	static const int MAX_THREAD_NUMBER = 16;
};

#endif
