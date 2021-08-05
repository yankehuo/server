#include "httpconn.h"

const char *HttpConn::srcdir;
std::atomic<int> HttpConn::usercount;
bool HttpConn::isET;

HttpConn::HttpConn() {
	fd_ = -1;
	addr_ = {0};
	isclose_ = true;
}
HttpConn::~HttpConn() {
	Close();
}

void HttpConn::Init(int fd, const sockaddr_in &addr) {
	assert(fd > 0);
	++usercount;
	addr_ = addr;
	fd_ = fd;
	writebuff_.RetrieveAll();
	readbuff_.RetrieveAll();
	isclose_ = false;
	LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, GetIP(), GetPort(), (int)usercount);
}

void HttpConn::Close() {
	response_.UnmapFile();
	if (isclose_ == false) {
		isclose_ = true;
		--usercount;
		close(fd_);
		LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, GetIP(), GetPort(), (int)usercount);
	}
}

int HttpConn::GetFd() const {
	return fd_;
}
struct sockaddr_in HttpConn::GetAddr() const {
	return addr_;
}
const char *HttpConn::GetIP() const {
	return inet_ntoa(addr_.sin_addr);
}
int HttpConn::GetPort() const {
	return addr_.sin_port;
}

ssize_t HttpConn::Read(int *saveerrno) {
	ssize_t len = -1;
	do {
		len = readbuff_.ReadFd(fd_, saveerrno);
		if (len <= 0) {
			break;
		}
	} while (isET);
	return len;
}

ssize_t HttpConn::Write(int *saveerrno) {
	ssize_t len = -1;
	do {
		len = writev(fd_, iov_, iovcnt_);
		if (len <= 0) {
			*saveerrno = errno;
			break;
		}
		if (iov_[0].iov_len + iov_[1].iov_len == 0) {
			break;
		}
		else if (static_cast<size_t>(len) > iov_[0].iov_len) {
			iov_[1].iov_base = (uint8_t *)iov_[1].iov_base + (len - iov_[0].iov_len);
			iov_[1].iov_len -= (len - iov_[0].iov_len);
			if (iov_[0].iov_len) {
				writebuff_.RetrieveAll();
				iov_[0].iov_len = 0;
			}
		}
		else {
			iov_[0].iov_base = (uint8_t *)iov_[0].iov_base + len;
			iov_[0].iov_len -= len;
			writebuff_.Retrieve(len);
		}

	} while (isET || ToWriteBytes() > 10240);
	return len;
}

bool HttpConn::Process() {
	request_.Init();
	if (readbuff_.ReadableBytes() <= 0) {
		return false;
	}
	else if (request_.Parse(readbuff_)) {
		LOG_DEBUG("%s", request_.Path().c_str());
		response_.Init(srcdir, request_.Path(), request_.IsKeepAlive(), 200);
	}
	else {
		response_.Init(srcdir, request_.Path(), false, 400);
	}

	response_.MakeResponse(writebuff_);
	// response header
	iov_[0].iov_base = const_cast<char *>(writebuff_.Peek());
	iov_[0].iov_len = writebuff_.ReadableBytes();
	iovcnt_ = 1;

	// response file
	if (response_.FileLen() > 0 && response_.File()) {
		iov_[1].iov_base = response_.File();
		iov_[1].iov_len = response_.FileLen();
		iovcnt_ = 2;
	}
	LOG_DEBUG("filesize:%d, %d to %d", response_.FileLen(), iovcnt_, ToWriteBytes());
	return true;
}

