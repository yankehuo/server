#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <unordered_map>
// open
#include <fcntl.h>
// close
#include <unistd.h>
//stat
#include <sys/stat.h>
// mmap, munmap
#include <sys/mman.h>

#include "../buffer/buffer.h"
#include "../logbq/log.h"

class HttpResponse {
private:
	int code_;
	bool isKeepAlive_;

	std::string path_;
	std::string srcDir_;

	char *mmFile_;
	// get information of file into struct
	struct stat mmFileStat_;

	static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
	static const std::unordered_map<int, std::string> CODE_STATUS;
	static const std::unordered_map<int, std::string> CODE_PATH;
public:
	HttpResponse();
	~HttpResponse();

	void Init(const std::string &srcDir, std::string &path, bool isKeepAlive = false, int code = -1);
	void MakeResponse(Buffer &buff);
	void UnmapFile();
	char *File();
	size_t FileLen() const;
	void ErrorContent(Buffer &buff, std::string message);
	int Code() const {
		return code_;
	}
private:
	void AddStateLine_(Buffer &buff);
	void AddHeader_(Buffer &buff);
	void AddContent_(Buffer &buff);

	void ErrorHtml_();
	std::string GetFileType_();
};


#endif
