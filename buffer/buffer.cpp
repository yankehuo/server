#include "buffer.h"

Buffer::Buffer(int initbuffsize) : buffer_(initbuffsize), readpos_(0), writepos_(0) {}

size_t Buffer::ReadableBytes() const {
	return writepos_ - readpos_;
}
size_t Buffer::WritableBytes() const {
	return buffer_.size() - writepos_;
}
size_t Buffer::PrependableBytes() const {
	return readpos_;
}

const char *Buffer::Peek() const {
	return BeginPtr_() + readpos_;
}

// relocate the readpos_
void Buffer::Retrieve(size_t len) {
	assert(len <= ReadableBytes());
	readpos_ += len;
}
void Buffer::RetrieveUntil(const char *end) {
	assert(Peek() <= end);
	Retrieve(end - Peek());
}
void Buffer::RetrieveAll() {
	bzero(&buffer_[0], buffer_.size());
	readpos_ = 0;
	writepos_ = 0;
}

std::string Buffer::RetrieveAllToStr() {
	std::string str(Peek(), ReadableBytes());
	RetrieveAll();
	return str;
}

const char *Buffer::BeginWriteConst() const {
	return BeginPtr_() + writepos_;
}
char *Buffer::BeginWrite() {
	return BeginPtr_() + writepos_;
}

void Buffer::HasWritten(size_t len) {
	writepos_ += len;
}

void Buffer::Append(const std::string &str) {
	// a point to the c-string :data()
	Append(str.data(), str.length());
}
void Buffer::Append(const void *data, size_t len) {
	assert(data);
	Append(static_cast<const char *>(data), len);
}
// adjust the space of buffer_, and relocate mainly the writepos_
void Buffer::Append(const char *str, size_t len) {
	assert(str);
	EnsureWritable(len);
	std::copy(str, str + len, BeginWrite());
	HasWritten(len);
}

void Buffer::EnsureWritable(size_t len) {
	if (WritableBytes() < len) {
		MakeSpace_(len);
	}
	assert(WritableBytes() >= len);
}

ssize_t Buffer::ReadFd(int fd, int *saveErrno) {
	char buff[65535];
	struct iovec iov[2];
	const size_t writable = WritableBytes();
	// buffer_
	iov[0].iov_base = BeginPtr_() + writepos_;
	iov[0].iov_len = writable;
	// buff
	iov[1].iov_base = buff;
	iov[1].iov_len = sizeof(buff);
	const ssize_t len = readv(fd, iov, 2);
	if (len < 0) {
		*saveErrno = errno;
	}
	else if (static_cast<size_t>(len) <= writable) {
		writepos_ += len;
	}
	else {
		writepos_ = buffer_.size();
		Append(buff, len - writable);
	}
	return len;
}

ssize_t Buffer::WriteFd(int fd, int *saveErrno) {
	size_t readSize = ReadableBytes();
	ssize_t len = write(fd, Peek(), readSize);
	if (len < 0) {
		*saveErrno = errno;
		return len;
	}
	readpos_ += len;
	return len;
}

char *Buffer::BeginPtr_() {
	return &*buffer_.begin();
}
const char *Buffer::BeginPtr_() const {
	return &*buffer_.begin();
}

void Buffer::MakeSpace_(size_t len) {
	if (WritableBytes() + PrependableBytes() < len) {
		buffer_.resize(writepos_ + len + 1);
	}
	else {
		size_t readable = ReadableBytes();
		std::copy(BeginPtr_() + readpos_, BeginPtr_() + writepos_, BeginPtr_());
		readpos_ = 0;
		writepos_ = readpos_ + readable;
		assert(readable == ReadableBytes());
	}
}

