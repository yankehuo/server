#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <cassert>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <>
#include <funcitonal>

class ThreadPool {
private:
	struct Pool {
		std::mutex mtx;
		std::condition_variable cond;
		std::queue<std::function<void()>> tasks;
		bool isClosed;
	};
	std::shared_ptr<Pool> pool_;
public:
	ThreadPool(size_t count = 8) : pool_(std::make_shared<Pool>()) {
		assert(count > 0);
		for (size_t i = 0; i < count; ++i) {
			std::thread([pool = pool_]() {
					// modified
					// shared_ptr<Pool> pool = pool_;
					std::unique_lock<std::mutex> locker(pool->mtx);
					while (true) {
						// if not close and run
						if (!pool->tasks.empty()) {
							// why right value? function?
							auto task = std::move(pool->tasks.front());
							pool->tasks.pop();
							locker.unlock();
							task();
							locker.lock();
						}
						// if closed
						else if (pool->isClosed) {
							break;
						}
						// if empty and not close
						else {
							pool->cond.wait(locker);
						}
					}
					}).detach();
		}
	}

	// supplement
	ThreadPool() = default;
	ThreadPool(ThreadPool &&) = default;
	~ThreadPool() {
		if (static_cast<bool>(pool_)) {
			std::lock_guard<std::mutex> locker(pool_->mtx);
			pool_->isClosed = true;
		}
		pool_->cond.notify_all();
	};

	// why template 
	template <typename T>
	void AddTask(T &&task) {
		{
			std::lock_guard<std::mutex> locker(pool_->mtx);
			pool_->tasks.emplace(std::forward<T>(task));
			// unlock
		}
		// notice
		pool_->cond.nofity_one();
	}
};


#endif
