#ifndef EPOLLER_H
#define EPOLLER_H

// epoll_ctl
#include <sys/epoll.h>
// fcntl
#include <fcntl.h>
// close
#include <unistd.h>
#include <cassert>
#include <vector>
#include <errno.h>

class Epoller {
private:
	int epollfd_;
	// store ready 
	std::vector<struct epoll_event> events_;
public:
	explicit Epoller(int maxevent = 1024);
	~Epoller();
	bool AddFd(int fd, uint32_t events);
	bool ModFd(int fd, uint32_t events);
	bool DelFd(int fd);

	int Wait(int timeoutms = -1);
	int GetEventFd(size_t i) const;
	uint32_t GetEvents(size_t i) const;
};



#endif
