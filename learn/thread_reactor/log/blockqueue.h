#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include <sys/time.h>

template <typename T>
class BlockDeque {
private:
	std::deque<T> deq_;
	size_t capacity_;
	std::mutex mtx_;
	bool isClose_;
	std::condition_variable condConsumer_;
	std::condition_variable condProducer_;
public:
	explicit BlockDeque(size_t MaxCapacity = 1000);
	~BlockDeque();
	void clear();
	bool empty();
	bool full();
	void Close();

	size_t size();
	size_t capacity();

	T front();
	T back();

	void push_back(const T &item);
	void push_front(const T &item);
	bool pop(T &item);
	bool pop(T &item, int timeout);
	void flush();
};

template <typename T>
BlockDeque<T>::BlockDeque(size_t MaxCapacity) : capacity_(MaxCapacity) {
	assert(MaxCapacity > 0);
	isClose_ = false;
}
template <typename T>
BlockDeque<T>::~BlockDeque() {
	Close();
}

template <typename T>
void BlockDeque<T>::Close() {
	{
		std::lock_guard<std::mutex> locker(mtx_);
		deq_.clear();
		isClose_ = true;
	}
	condProducer_.notify_all();
	condConsumer_.notify_all();
}

template <typename T>
void BlockDeque<T>::flush() {
	condConsumer_.notify_one();
}

template <typename T>
void BlockDeque<T>::clear() {
	std::lock_guard<std::mutex> locker(mtx_);
	deq_.clear();
}

template <typename T>
T BlockDeque<T>::front() {
	std::lock_guard<std::mutex> locker(mtx_);
	return deq_.front();
}
template <typename T>
T BlockDeque<T>::back() {
	std::lock_guard<std::mutex> locker(mtx_);
	return deq_.back();
}

template <typename T>
size_t BlockDeque::size() {
	std::lock_guard<std::mutex> locker(mtx_);
	return deq_.size();
}

template <typename T>
size_t BlockDeque::capacity() {
	std::lock_guard<std::mutex> locker(mtx_);
	return capacity_;
}

template <typename T>
void BlockDeque::push_back(const T &item) {
	std::unique_lock<std::mutex> locker(mtx_);
	// prevent supurious wake-up calls
	while (deq_.size() >= capacity_) {
		condProducer_.wait(locker);
	}
	deq_.push_back(item);
	condConsumer_.notify_one();
}
template <typename T>
void BlockDeque<T>::push_front(const T &item) {
	std::unique_lock<std::mutex> locker(mtx_);
	while (deq_.size() >= capacity_) {
		condProducer_.wait(locker);
	}
	condConsumer_.notify_one();
}

template <typename T>
bool BlockDeque<T>::empty() {
	std::lock_guard<std::mutex> locker(mtx_);
	return deq_.empty();
}

template <typename T>
bool BlockDeque<T>::full() {
	std::lock_guard<std::mutex> locker(mtx_);
	return deq_.size() >= capacity_;
}

template <typename T>
bool BlockDeque<T>::pop(T &item) {
	std::unique_lock<std::mutex> locker(mtx_);
	while (deq_.empty()) {
		condConsumer_.wait(locker);
		if (isClose_) {
			return false;
		}
	}
	item = deq_.front();
	deq_.pop_front();
	condProducer_.notify_one();
	return true;
}

template <typename T>
bool BlockDeque<T>::pop(T &item, int timeout) {
	std::unique_lock<std::mutex> locker(mtx_);
	while (deq_.empty()) {
		if (condConsumer_.wait_for(locker, std::chrono::seconds(timeout)) == std::cv_status::timeout) {
			return false;
		}
		if (isClose_) {
			return false;
		}
	}
	item = deq_.front();
	deq_.pop_front();
	condProducer_.notify_one();
	return true;
}

#endif
