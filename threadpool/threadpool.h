#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <cassert>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>

class ThreadPool {
private:
	struct Pool {
		std::mutex mtx;
		std::condition_variable cond;
		bool isclosed;
		std::queue<std::function<void()>> tasks;
	};
	std::shared_ptr<Pool> pool_;
public:
	explicit ThreadPool(size_t threadcount = 8) : pool_(std::make_shared<Pool>()) {
		assert(threadcount > 0);
		for (size_t i = 0; i < threadcount; ++i) {
			std::thread([pool = pool_] {
					while (true) {
						std::unique_lock<std::mutex> locker(pool->mtx);
						if (pool->isclosed) {
							locker.unlock();
							break;
						}
						while (pool->tasks.empty()) {
							pool->cond.wait(locker);
						}
						auto task = std::move(pool->tasks.front());
						pool->tasks.pop();
						locker.unlock();
						task();
					}
				}).detach();
		}
	}
	ThreadPool() = default;
	ThreadPool(ThreadPool &&) = default;

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool &operator=(const ThreadPool &) = delete;

	~ThreadPool() {
		if (static_cast<bool>(pool_)) { 
			{
				std::lock_guard<std::mutex> locker(pool_->mtx);
				pool_->isclosed = true;
			}
			pool_->cond.notify_all();
		}
	}

	// add fucntion object
	template <typename T>
	void AddTask(T &&task) {
		{
			std::lock_guard<std::mutex> locker(pool_->mtx);
			pool_->tasks.emplace(std::forward<T>(task));
		}
		pool_->cond.notify_one();
	}
};

#endif
