#ifndef HTTP_CONN_h
#define HTTP_CONN_h

#include <sys/types.h>
// readv writev
#include <sys/uio.h>
// sockaddr_in
#include <arpa/inet.h>
// atoi
#include <cstdlib>
#include <errno.h>

#include "../logbq/log.h"
#include "httprequest.h"
#include "httpresponse.h"

class HttpConn {
private:
	int fd_;
	struct sockaddr_in addr_;
	bool isclose_;
	int iovcnt_;
	struct iovec iov_[2];

	Buffer readbuff_;
	Buffer writebuff_;

	HttpRequest request_;
	HttpResponse response_;
public:
	HttpConn();
	~HttpConn();
	void Init(int sockfd, const sockaddr_in &addr);
	ssize_t Read(int *saveerrno);
	ssize_t Write(int *saveerrno);
	void Close();
	int GetFd() const;
	int GetPort() const;
	const char *GetIP() const;
	sockaddr_in GetAddr() const;
	bool Process();

	int ToWriteBytes() {
		return iov_[0].iov_len + iov_[1].iov_len;
	}
	bool IsKeepAlive() const {
		return request_.IsKeepAlive();
	}

	static bool isET;
	static const char *srcdir;
	static std::atomic<int> usercount;
};

#endif
