#ifndef THREADPOOL_H
#define THREADPOOL_H

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

// m_tid--target TID, m_pipefd--parent and child pipe
// 工作线程类 working thread
class thread {
public:
	pthread_t m_tid;
	int m_pipefd[2];
};

// threadpool 
template <typename T>
class threadpool {
private:
	// max child thread-->working threading
	static const int MAX_THREAD_NUMBER = 16;
	static const int USER_PER_THREAD = 1000;
	// event numbers in epoll kernel list
	static const int MAX_EVENT_NUMBER = 1000;
	// total thread number
	int m_thread_number;
	// point to listening socket, main and work share the same
	int m_listenfd;
	thread *m_work_thread;
	static threadpool *m_instance;
private:
	threadpool(int listenfd, int thread_number = 8);
	// work thread run
	static void *worker(void *arg);
public:
	// problem ? T ?
	static threadpool<T> *create(int listenfd, int thread_number = 8) {
		if (!m_instance) {
			m_instance = new threadpool<T>(listenfd, thread_number);
		}
		return m_instance;
	}
	// added
	static threadpool *get_instance() {
		return m_instance;
	}

	~processpool() {
		// 
		delete [] m_work_thread;
	}

	// main thread
	void run();
	void setup_sig_pipe();
};
template <typename T>
threadpool<T> *threadpool<T>::m_instance = NULL;

// pipe for signal
int sig_pipefd[2];

int setnonblocking(int fd) {
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

// used_fd store fd
void addfd(int epollfd, std::set<int> &used_fd) {
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	used_fd.insert(fd);
	setnonblocking(fd);
}

void removefd(int epollfd, std::set<int> &used_fd) {
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
	used_fd.erase(fd);
}

void close_fd(int epollfd, std::set<int> &used_fd) {
	for (const auto &fd : used_fd) {
		epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
	}
	used_fd.clear();
	close(epollfd);
}

void sig_handler(int sig) {
	int save_errno = errno;
	int msg = sig;
	// signal fdpipe
	send(sig_pipefd[1], (char*)&msg, 1, 0);
	errno = save_errno;
}

void addsig(int sig, void(*handler)(int), bool restart = true) {
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = handler;
	if(restart) {
		sa.sa_flags |= SA_RESTART;
	}
	sigfillset(&sa.sa_mask);
	assert(sigaction(sig, &sa, NULL) != -1);
}

// unified event source: I/O signal time?
template <typename T>
void processpool<T>::setup_sig_pipe() {
	m_epollfd = epoll_create(5);
	assert(m_epollfd != -1);

	int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, sig_pipefd);
	assert(ret != -1);

	setnonblocking(sig_pipefd[1]);
	addfd(m_epollfd, sig_pipefd[0]);

	// addsig(SIGCHLD, sig_handler);
	addsig(SIGTERM, sig_handler);
	addsig(SIGINT, sig_handler);
	addsig(SIGPIPE, sig_handler);
}
// constructor, listenfd must be created before pool
template <typename T>
processpool<T>::processpool(int listenfd, int thread_number) 
	: m_listenfd(listenfd), m_thread_number(thread_number), m_idx(-1)
{
	assert((thread_nubmer > 0) && (thread_number <= MAX_THREAD_NUMBER));

	m_work_thread = new thread[m_thread_number];
	assert(m_work_thread);

	// make pipes for work and main
	for(int i = 0; i < m_thread_number; ++i) {
		// two pipe
		int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_work_thread[i].m_pipefd);
		assert(ret == 0);

		ret = pthread_create(&m_work_thread[i].m_tid, NULL, worker, m_work_thread + i);
		assert(ret == 0);

		ret = pthread_detach(m_work_thread[i].m_tid);
		assert(ret == 0);
	}
}

// work thread --> worker
void *threadpool<T>::worker(void *arg) {
	m_thread *cur_thread = (m_thread*)arg;
	std::set<int> used_fd;
	int epollfd = epoll_create(5);
	assert(epollfd != -1);

	int pipefd = cur_thread->m_pipefd[0];
	addfd(epollfd, pipefd, used_fd);

	// events
	epoll_event events[MAX_EVENT_NUMBER];
	T *users = new T[USER_PER_PROCESS];
	assert(users);

	int ret = -1;
	int number = 0;
	// stop ?
	bool stop = false;
	while (!stop) {
		number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		if ((number < 0) && (errno != EINTR)) {
			printf("epoll failure\n");
			break;
		}
		for (int i = 0; i < number; ++i) {
			int sockfd = events[i].data.fd;
			// from main thread
			if ((sockfd == pipefd) && (events[i].events & EPOLLIN)) {
				int client = 0;
				// read from pipe
				ret = recv(sockfd, (char*)&client, sizeof(client), 0);
				if ((ret <= 0) {
					continue;
				}
				else {
					struct sockaddr_in client_address;
					socklen_t client_addrlength = sizeof(client_address);
					int connfd = accept(get_instance()->m_listenfd, (struct sockaddr*)&client_address, &client_addrlength);
					if (connfd < 0) {
						printf("errno is: %d\n", errno);
						continue;
					}
					else if (connfd >= USER_PER_THREAD) {
						const char *msg = "sorry! max users.";
						send(connfd, msg, sizeof(msg), 0);
						close(connfd);
						continue;
					}
					addfd(epollfd, connfd, used_fd);
					// init how to do?
					users[connfd].init(epollfd, connfd, client_address);
				}
			}
			else if (events[i].events & EPOLLIN) {
				// process how to process?
				users[sockfd].process(used_fd);
			}
		}
	}
	delete [] users;
	users = NULL;
	close_fd(epollfd, used_fd);
	cur_thread->m_tid = -1;
	pthread_exit(NULL);
	// close(pipefd);
	// why no close(m_listenfd) ? no owner?
}




template <typename T>
void processpool<T>::run_parent() {
	setup_sig_pipe();
	// parent listen m_listenfd
	addfd(m_epollfd, m_listenfd);
	epoll_event events[MAX_EVENT_NUMBER];

	// work thread start
	int sub_thread_counter = 0;
	// int new_conn = 1;
	bool stop = false;
	int number = 0;
	// int ret = -1;

	while (!stop) {
		number = epoll_wait(m_epollfd, events, MAX_EVENT_NUMBER, -1);
		if ((number < 0) && (errno != EINTR)) {
			printf("epoll failure\n");
			break;
		}
		for (int i = 0; i < number && !stop; ++i) {
			int sockfd = events[i].data.fd;
			if (sockfd == m_listenfd) {
				// Round Robin distribute a link
				int i = sub_thread_counter;
				do {
					// Round Robin
					// why 
					if (m_work_thread[i].m_tid != -1) {
						break;
					}
					i = (i + 1) % m_thread_number;
				} while (i != sub_thread_counter);

				if (m_work_thread[i].m_tid == -1) {
					stop = true;
					break;
				}
				// why?
				sub_thread_counter = (i + 1) % m_thread_number;

				int msg = -1;
				send(m_work_thread[i].m_pipefd[1], (char*)&msg, sizeof(msg), 0);
			}

			else if ((sockfd == sig_pipefd[0]) && (events[i].events & EPOLLIN)) {
				char signals[1024];
				ret = recv(sig_pipefd[0], signals, sizeof(signals), 0);
				if (ret <= 0) {
					continue;
				}
				else {
					for (int i = 0; i < ret && !stop; ++i) {
						switch (signals[i]) {
							case SIGTERM:
							case SIGINT:
							{
								printf("kill all the work thread now\n");
								for (int j = 0; j < m_thread_number; ++j) {
									unsigned long tid = m_work_thread[j].m_tid;
									if (tid != -1) {
										pthread_cancel(tid);
									}
								}
								stop = true;
								break;
							}
							default:
							{
								break;
							}
						}
					}
				}
			}

			else {
				continue;
			}
		}
	}
	close(m_epollfd);
	close(sig_pipefd[1]);
}

#endif
