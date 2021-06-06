#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
// why
#include <stdarg.h>
#include <errno.h>
#include "locker.h"

class http_conn {
public:
	static const int FILENAME_LEN = 200;
	static const int READ_BUFFER_SIZE = 2048;
	static const int WRITE_BUFFER_SIZE = 1024;
	// 请求方法
	enum METHOD {
		GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT, PATCH
	};
	// 状态
	enum CHECK_STATE {
		CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT
	};

	// 处理HTTP
	enum HTTP_CODE {
		NO_REQUEST, GET_REQUEST, BAD_REQUES, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION
	};

	// 行的读取状态
	enum LINE_STATUS {
		LINE_OK = 0, LINE_BAD, LINE_OPEN
	};

public:
	http_conn() {}
	~http_conn() {}
public:
	void init(int sockfd, const sockaddr_in &addr);
	void close_conn(bool real_close = true);
	void porcess();
	// 非阻塞操作
	bool read();
	bool write();

private:
	void init();
	// 解析请求
	HTTP_CODE process_read();
	// 填充应答
	bool process_write(HTTP_CODE ret);

	HTTP_CODE parse_request_line(char *text);
	HTTP_CODE parse_headers(char *text);
	HTTP_CODE parse_content(char *text);
	HTTP_CODE do_request();
	char *get_line() {
		return m_read_buf + m_start_line;
	}
	LINE_STATUS parse_line();

	void unmap();
	// why?
	bool add_response(const char *format, ...);
	bool add_content(const char *content);
	bool add_headers(int status, const char *title);
	bool add_content_length(int content_length);
	bool add_linger();
	bool add_blank_line();

public:
	// 所有都注册到同一个
	static int m_epollfd;
	static int m_user_count;

private:
	int m_sockfd;
	sockaddr_in m_address;

	char m_read_buf[READ_BUFFER_SIZE];
	int m_read_idx;
	int m_checked_idx;
	int m_start_line;
	char m_write_buf[WRITE_BUFFER_SIZE];
	int m_write_idx;

	CHECK_STATE m_check_state;
	METHOD m_method;

	//	客户请求的目标文件完整路径
	char m_real_file[FILENAME_LEN];
	char *m_url;
	char *m_version;
	char *m_host;
	char *m_content_length;
	// 是否保持连接
	bool m_linger;

	// 目标文件mmap到内存的位置
	char *m_file_address;
	struct stat m_file_stat;
	// writev执行写操作
	struct iovec m_iv[2];
	int m_iv_count;

};

#endif
