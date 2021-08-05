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
	bool isclose_;
	std::condition_variable condconsumer_;
	std::condition_variable condproducer_;
public:
	explicit BlockDeque(size_t maxcapacity = 1000);
	~BlockDeque();
	void Clear();
	bool Empty();
	bool Full();
	void Close();

	size_t Size();
	size_t Capacity();

	T Front();
	T Back();

	void PushBack(const T &item);
	void PushFront(const T &item);
	bool Pop(T &item);
	bool Pop(T &item, int timeout);
	void Flush();
};

template <typename T>
BlockDeque<T>::BlockDeque(size_t MaxCapacity) : capacity_(MaxCapacity) {
	assert(MaxCapacity > 0);
	isclose_ = false;
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
		isclose_ = true;
	}
	condproducer_.notify_all();
	condconsumer_.notify_all();
}

template <typename T>
void BlockDeque<T>::Flush() {
	condconsumer_.notify_one();
}

template <typename T>
void BlockDeque<T>::Clear() {
	std::lock_guard<std::mutex> locker(mtx_);
	deq_.clear();
}

template <typename T>
T BlockDeque<T>::Front() {
	std::lock_guard<std::mutex> locker(mtx_);
	return deq_.front();
}
template <typename T>
T BlockDeque<T>::Back() {
	std::lock_guard<std::mutex> locker(mtx_);
	return deq_.back();
}

template <typename T>
size_t BlockDeque<T>::Size() {
	std::lock_guard<std::mutex> locker(mtx_);
	return deq_.size();
}

template <typename T>
size_t BlockDeque<T>::Capacity() {
	std::lock_guard<std::mutex> locker(mtx_);
	return capacity_;
}

template <typename T>
void BlockDeque<T>::PushBack(const T &item) {
	std::unique_lock<std::mutex> locker(mtx_);
	// prevent spurious wake-up calls
	while (deq_.size() >= capacity_) {
		condproducer_.wait(locker);
	}
	deq_.push_back(item);
	condconsumer_.notify_one();
}
template <typename T>
void BlockDeque<T>::PushFront(const T &item) {
	std::unique_lock<std::mutex> locker(mtx_);
	while (deq_.size() >= capacity_) {
		condproducer_.wait(locker);
	}
	deq_.push_front(item);
	condconsumer_.notify_one();
}

template <typename T>
bool BlockDeque<T>::Empty() {
	std::lock_guard<std::mutex> locker(mtx_);
	return deq_.empty();
}

template <typename T>
bool BlockDeque<T>::Full() {
	std::lock_guard<std::mutex> locker(mtx_);
	return deq_.size() >= capacity_;
}

template <typename T>
bool BlockDeque<T>::Pop(T &item) {
	std::unique_lock<std::mutex> locker(mtx_);
	while (deq_.empty()) {
		condconsumer_.wait(locker);
		if (isclose_) {
			return false;
		}
	}
	item = deq_.front();
	deq_.pop_front();
	condproducer_.notify_one();
	return true;
}

template <typename T>
bool BlockDeque<T>::Pop(T &item, int timeout) {
	std::unique_lock<std::mutex> locker(mtx_);
	while (deq_.empty()) {
		if (condconsumer_.wait_for(locker, std::chrono::seconds(timeout)) == std::cv_status::timeout) {
			return false;
		}
		if (isclose_) {
			return false;
		}
	}
	item = deq_.front();
	deq_.pop_front();
	condproducer_.notify_one();
	return true;
}

#endif
