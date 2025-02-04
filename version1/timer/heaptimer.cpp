#include "heaptimer.h"

void HeapTimer::SwapNode_(size_t i, size_t j) {
	assert(i >= 0 && i < heap_.size()); 
	assert(j >= 0 && j < heap_.size()); 
	std::swap(heap_[i], heap_[j]);
	ref_[heap_[i].id] = i;
	ref_[heap_[j].id] = j;
}

void HeapTimer::SiftUp_(size_t i) {
	assert(i >= 0 && i < heap_.size()); 
	size_t j = (i - 1) / 2;
	while (j >= 0) {
		if (heap_[j] < heap_[i]) {
			break;
		}
		SwapNode_(i, j);
		i = j;
		j = (i - 1) / 2;
	}
}
bool HeapTimer::SiftDown_(size_t index, size_t n) {
	assert(index >= 0 && index < heap_.size());
	assert(n >= 0 && n <= heap_.size());
	size_t i = index;
	size_t j = i * 2 + 1;
	while (j < n) {
		if (j + 1 < n && heap_[j + 1] < heap_[j])
			++j;
		if (heap_[i] < heap_[j])
			break;
		SwapNode_(i, j);
		i = j;
		j = i * 2 + 1;
	}
	return i > index;
}

void HeapTimer::Add(int id, int timeout, const TimeoutCallBack &cb) {
	assert(id >= 0);
	size_t i;
	if (ref_.count(id) == 0) {
		i = heap_.size();
		ref_[id] = i;
		heap_.push_back({id, Clock::now() + MS(timeout), cb});
		SiftUp_(i);
	}
	else {
		i = ref_[id];
		heap_[i].expires = Clock::now() + MS(timeout);
		heap_[i].cb = cb;
		if (!SiftDown_(i, heap_.size())) {
			SiftUp_(i);
		}
	}
}

void HeapTimer::Del_(size_t index) {
	// del the specified node
	assert(!heap_.empty() && index >= 0 && index < heap_.size());
	// change the i into the last
	size_t i = index;
	size_t n = heap_.size() - 1;
	assert(i <= n);
	if (i < n) {
		SwapNode_(i, n);
		if (!SiftDown_(i, n)) {
			SiftUp_(i);
		}
	}
	// del the last
	ref_.erase(heap_.back().id);
	heap_.pop_back();
}

void HeapTimer::DoWork(int id) {
	// remove the specified id
	if (heap_.empty() || ref_.count(id) == 0) {
		return;
	}
	size_t i = ref_[id];
	TimerNode node = heap_[i];
	node.cb();
	Del_(i);
}

void HeapTimer::Adjust(int id, int timeout) {
	assert(!heap_.empty() && ref_.count(id) > 0);
	heap_[ref_[id]].expires = Clock::now() + MS(timeout);
	SiftDown_(ref_[id], heap_.size());
}

void HeapTimer::Tick() {
	// clear the expired node
	if (heap_.empty()) {
		return;
	}
	while (!heap_.empty()) {
		TimerNode node = heap_.front();
		if (std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) {
			break;
		}
		node.cb();
		Pop();
	}
}

void HeapTimer::Pop() {
	assert(!heap_.empty());
	Del_(0);
}

void HeapTimer::Clear() {
	ref_.clear();
	heap_.clear();
}

int HeapTimer::GetNextTick() {
	Tick();
	size_t res = -1;
	if (!heap_.empty()) {
		res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
		if (res < 0) {
			res = 0;
		}
	}
	return res;
}


