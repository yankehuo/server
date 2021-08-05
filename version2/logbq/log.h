#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <cassert>
#include <cstring>
// va_start va_end
#include <cstdarg>
// mkdir
#include <sys/stat.h>
#include "blockqueue.h"
#include "../buffer/buffer.h"

class Log {
private:
	static const int LOG_PATH_LEN = 256;
	static const int LOG_NAME_LEN = 256;
	static const int MAX_LINES = 50000;

	const char *path_;
	const char *suffix_;

	int linecount_;
	int today_;

	bool isopen_;

	Buffer buff_;
	int level_;
	bool isasync_;

	FILE *fp_;
	std::unique_ptr<BlockDeque<std::string>> deque_;
	std::unique_ptr<std::thread> writethread_;
	std::mutex mtx_;
public:
	void Init(int level, const char *path = "./log", const char *suffix = ".log", int maxQueueCapacity = 1024);

	// singleton lazy mode
	static Log *Instance() {
		static Log instance;
		return &instance;
	};

	static void FlushLogThread();

	void Write(int level, const char *format, ...);
	void Flush();

	int GetLevel();
	void SetLevel(int level);
	bool IsOpen() {
		return isopen_;
	}
private:
	// singleton
	Log();
	void AppendLogLevelTitle_(int level);
	~Log();
	void AsynWrite_();
};


 #define LOG_BASE(level, format, ...) \
	do {\
		Log *log = Log::Instance();\
		if (log->IsOpen() && log->GetLevel() <= level) {\
			log->Write(level, format, ##__VA_ARGS__);\
			log->Flush();\
		}\
	} while (0);
#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while (0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while (0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while (0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while (0);

#endif
