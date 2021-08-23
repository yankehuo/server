#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "epoller.h"
#include "../logbq/log.h"
#include "../timer/heaptimer.h"
#include "../threadpool/threadpool.h"
#include "../http/httpconn.h"

class WebServer {
private:
	static const int MAX_FD = 65536;

	int port_;
	// 
	bool openlinger_;
	int timeoutms_;
	bool isclose_;
	int listenfd_;
	char *srcdir_;
	// modified 1-->reactor 0-->proactor
	bool actor_;

	uint32_t listenevent_;
	uint32_t connevent_;

	std::unique_ptr<HeapTimer> timer_;
	std::unique_ptr<ThreadPool> threadpool_;
	std::unique_ptr<Epoller> epoller_;
	std::unordered_map<int, HttpConn> users_;
public:
	// 
	WebServer(int port, int trigmode, int timeoutms, bool optlinger, int threadnum, bool openlog, int loglevel, int logquesize, int actor);
	~WebServer();
	void Start();
private:
	bool InitSocket_();
	void InitEventMode_(int trigmode);
	void AddClient_(int fd, sockaddr_in addr);

	void DealListen_();
	void DealWrite_(HttpConn *client);
	void DealRead_(HttpConn *client);

	void SendError_(int fd, const char *info);
	void ExtentTime_(HttpConn *client);
	void CloseConn_(HttpConn *client);

	void OnRead_(HttpConn *client);
	void OnWrite_(HttpConn *client);
	void OnProcess_(HttpConn *client);

	static int SetFdNonblock(int fd);
};


#endif
