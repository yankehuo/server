#ifndef HEAP_TIME_H
#define HEAP_TIME_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h>
#include <functional>
#include <cassert>
#include <chrono>
#include "../logbq/log.h"

// call back function
typedef std::function<void()> TimeoutCallBack;
// clock set
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;

typedef Clock::time_point TimeStamp;

struct TimerNode {
	int id;
	TimeStamp expires;
	TimeoutCallBack cb;
	bool operator<(const TimerNode &t) {
		return expires < t.expires;
	}
};

class HeapTimer {
private:
	std::vector<TimerNode> heap_;
	// int to size_t
	std::unordered_map<int, size_t> ref_;
public:
	HeapTimer() {
		heap_.reserve(64);
	}
	~HeapTimer() {
		Clear();
	}
	void Adjust(int id, int newExpires);
	void Add(int id, int timeOut, const TimeoutCallBack &cb);
	void DoWork(int id);
	void Clear();
	void Tick();
	void Pop();
	int GetNextTick();
private:
	void Del_(size_t i);
	void SiftUp_(size_t i);
	bool SiftDown_(size_t index, size_t n);
	void SwapNode_(size_t i, size_t j);
};

#endif
