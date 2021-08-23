#include "webserver.h"

WebServer::WebServer(int port, int trigmode, int timeoutms, bool openlinger, int threadnum, bool openlog, int loglevel, int logquesize, int actor) : 
	port_(port), openlinger_(openlinger), timeoutms_(timeoutms), isclose_(false), timer_(new HeapTimer()), threadpool_(new ThreadPool(threadnum)), epoller_(new Epoller()) {
		// get current working directory
		srcdir_ = getcwd(nullptr, 256);
		assert(srcdir_);
		// concatenate
		strncat(srcdir_, "/resource/", 16);
		HttpConn::usercount = 0;
		HttpConn::srcdir = srcdir_;

		// sequence
		if (!InitSocket_()) {
			isclose_ = true;
		}

		// sequence
		InitEventMode_(trigmode);
		if (openlog) {
			Log::Instance()->Init(loglevel, "./log", ".log", logquesize);
			if (isclose_) {
				LOG_ERROR("============== Server init error ================");
			}
			else {
				LOG_INFO("=============== Server init ======================");
				LOG_INFO("Port:%d, OpenLinger:%s", port_, openlinger ? "true" : "false");
				LOG_INFO("Listen Mode:%s, OpenConn Mode:%s", (listenevent_ & EPOLLET ? "ET" : "LT"), (connevent_ & EPOLLET ? "ET" : "LT"));
				LOG_INFO("LogSys level:%d", loglevel);
				LOG_INFO("srcDir:%s", HttpConn::srcdir);
				LOG_INFO("ThreadPool num:%d", threadnum);
			}
		}
		actor_ = actor;

}

WebServer::~WebServer() {
	close(listenfd_);
	isclose_ = true;
	free(srcdir_);
}

void WebServer::InitEventMode_(int trigmode) {
	listenevent_ = EPOLLRDHUP;
	connevent_ = EPOLLONESHOT | EPOLLRDHUP;
	switch (trigmode) {
		case 0:
			break;
		case 1:
			connevent_ |= EPOLLET;
			break;
		case 2:
			listenevent_ |= EPOLLET;
			break;
		case 3:
			listenevent_ |= EPOLLET;
			connevent_ |= EPOLLET;
			break;
		default:
			listenevent_ |= EPOLLET;
			connevent_ |= EPOLLET;
			break;
	}
	HttpConn::isET = (connevent_ & EPOLLET);
}

void WebServer::Start() {
	int timems = -1;
	if (!isclose_) {
		LOG_INFO("================= Server start ==============");
	}
	while (!isclose_) {
		if (timeoutms_ > 0) {
			// get the remain time if > 0
			timems = timer_->GetNextTick();
		}
		int eventcnt = epoller_->Wait(timems);
		for (int i = 0; i < eventcnt; ++i) {
			// fd events
			int fd = epoller_->GetEventFd(i);
			uint32_t events = epoller_->GetEvents(i);
			if (fd == listenfd_) {
				DealListen_();
			}
			else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
				assert(users_.count(fd) > 0);
				CloseConn_(&users_[fd]);
			}
			else if (events & EPOLLIN) {
				assert(users_.count(fd) > 0);
				DealRead_(&users_[fd]);
			}
			else if (events & EPOLLOUT) {
				assert(users_.count(fd) > 0);
				DealWrite_(&users_[fd]);
			}
			else {
				LOG_ERROR("Unexpected event.");
			}
		}
	}
}

void WebServer::SendError_(int fd, const char *info) {
	assert(fd > 0);
	int ret = send(fd, info, strlen(info), 0);
	if (ret < 0) {
		LOG_WARN("send error to client[%d] error!", fd);
	}
	close(fd);
}

void WebServer::CloseConn_(HttpConn *client) {
	assert(client);
	LOG_INFO("Client [%d] quit!", client->GetFd());
	epoller_->DelFd(client->GetFd());
	client->Close();
}
void WebServer::AddClient_(int fd, sockaddr_in addr) {
	assert(fd > 0);
	users_[fd].Init(fd, addr);
	if (timeoutms_ > 0) {
		timer_->Add(fd, timeoutms_, std::bind(&WebServer::CloseConn_, this, &users_[fd]));
	}
	epoller_->AddFd(fd, EPOLLIN | connevent_);
	SetFdNonblock(fd);
	LOG_INFO("Client[%d] in!", users_[fd].GetFd());
}

void WebServer::DealListen_() {
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	do {
		int fd = accept(listenfd_, (struct sockaddr *)&addr, &len);
		if (fd <= 0) {
			return;
		}
		else if (HttpConn::usercount >= MAX_FD) {
			SendError_(fd, "Server busy!");
			LOG_WARN("Client is full!");
			return;
		}
		AddClient_(fd, addr);
	} while (listenevent_ & EPOLLET);
}

void WebServer::DealRead_(HttpConn *client) {
	assert(client);
	ExtentTime_(client);
	// actor_mode
	// reactor
	if (actor_ == 1) {
		threadpool_->AddTask(std::bind(&WebServer::OnRead_, this, client));
	}
	//proactor
	else {
		assert(client);
		int ret = -1;
		int readerrno = 0;
		ret = client->Read(&readerrno);
		if (ret <= 0 && readerrno != EAGAIN) {
			CloseConn_(client);
			return;
		}	
		threadpool_->AddTask(std::bind(&WebServer::OnProcess_, this, client));
	}
}
void WebServer::DealWrite_(HttpConn *client) {
	assert(client);
	ExtentTime_(client);

	// reactor
	if (actor_ == 1) {
		threadpool_->AddTask(std::bind(&WebServer::OnWrite_, this, client));
	}
	//proactor
	else {
		assert(client);
		int ret = -1;
		int writeerrno = 0;
		ret = client->Write(&writeerrno);
		if (client->ToWriteBytes() == 0) {
			if (client->IsKeepAlive()) {
				threadpool_->AddTask(std::bind(&WebServer::OnProcess_, this, client));
				return;
			}
		}
		else if (ret < 0) {
			if (writeerrno == EAGAIN) {
				// continue trans
				epoller_->ModFd(client->GetFd(), connevent_ | EPOLLOUT);
				return;
			}
		}
		CloseConn_(client);
	}
}

void WebServer::ExtentTime_(HttpConn *client) {
	assert(client);
	if (timeoutms_ > 0) {
		timer_->Adjust(client->GetFd(), timeoutms_);
	}
}

void WebServer::OnRead_(HttpConn *client) {
	assert(client);
	int ret = -1;
	int readerrno = 0;
	ret = client->Read(&readerrno);
	if (ret <= 0 && readerrno != EAGAIN) {
		CloseConn_(client);
		return;
	}
	OnProcess_(client);
}

void WebServer::OnWrite_(HttpConn *client) {
	assert(client);
	int ret = -1;
	int writeerrno = 0;
	ret = client->Write(&writeerrno);
	if (client->ToWriteBytes() == 0) {
		if (client->IsKeepAlive()) {
			OnProcess_(client);
			return;
		}
	}
	else if (ret < 0) {
		if (writeerrno == EAGAIN) {
			// continue trans
			epoller_->ModFd(client->GetFd(), connevent_ | EPOLLOUT);
			return;
		}
	}
	CloseConn_(client);
}

void WebServer::OnProcess_(HttpConn *client) {
	if (client->Process()) {
		epoller_->ModFd(client->GetFd(), connevent_ | EPOLLOUT);
	}
	else {
		epoller_->ModFd(client->GetFd(), connevent_ | EPOLLIN);
	}
}


bool WebServer::InitSocket_() {
	int ret;
	struct sockaddr_in addr;
	if (port_ > 65535 || port_ < 1024) {
		LOG_ERROR("Port:%d error!", port_);
		return false;
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port_);
	// close:--> elegant
	struct linger optlinger = {0};
	if (openlinger_) {
		// close until data finished or timeouted
		optlinger.l_onoff = 1;
		optlinger.l_linger = 1;
	}
	listenfd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd_ < 0) {
		LOG_ERROR("Create socket error!", port_);
		return false;
	}

	ret = setsockopt(listenfd_, SOL_SOCKET, SO_LINGER, &optlinger, sizeof(optlinger));
	if (ret < 0) {
		close(listenfd_);
		LOG_ERROR("Init linger error!", port_);
		return false;
	}

	int optval = 1;
	// reuse the port
	ret = setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
	if (ret == -1) {
		LOG_ERROR("set socket error!");
		close(listenfd_);
		return false;
	}

	ret = bind(listenfd_, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		LOG_ERROR("Bind port:%d error!", port_);
		close(listenfd_);
		return false;
	}

	ret = listen(listenfd_, 8);
	if (ret < 0) {
		LOG_ERROR("Listen port:%d error!", port_);
		close(listenfd_);
		return false;
	}

	ret = epoller_->AddFd(listenfd_, listenevent_ | EPOLLIN);
	if (ret == false) {
		LOG_ERROR("Add listen error!");
		close(listenfd_);
		return false;
	}
	SetFdNonblock(listenfd_);
	LOG_INFO("Server port:%d", port_);
	return true;
}

int WebServer::SetFdNonblock(int fd) {
	assert(fd > 0);
	return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}
