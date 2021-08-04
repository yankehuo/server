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
	bool openLinger_;
	int timeoutMS_;
	bool isClose_;
	int listenFd_;
	char *srcDir_;
	// modified 1-->reactor 0-->proactor
	bool actor_;

	uint32_t listenEvent_;
	uint32_t connEvent_;

	std::unique_ptr<HeapTimer> timer_;
	std::unique_ptr<ThreadPool> threadpool_;
	std::unique_ptr<Epoller> epoller_;
	std::unordered_map<int, HttpConn> users_;
public:
	// 
	WebServer(int port, int trigMode, int timeoutMS, bool optLinger, /*int connPoolNum,*/ int threadNum, bool openLog, int logLevel, int logQueSize, int actor);
	~WebServer();
	void Start();
private:
	bool InitSocket_();
	void InitEventMode_(int trigMode);
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
	// proactor
	void ProRead_(HttpConn *client, int ret, int readErrno);
	void ProWrite_(HttpConn *client, int ret, int writeErrno);


	static int SetFdNonblock(int fd);
};


#endif
