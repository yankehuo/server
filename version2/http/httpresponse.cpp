#include "httpresponse.h"

const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

const std::unordered_map<int, std::string> HttpResponse::CODE_STATUS = {
	{ 200, "OK" },
	{ 400, "Bad Request" },
	{ 403, "Forbidden" },
	{ 404, "Not Found" },
};

const std::unordered_map<int, std::string> HttpResponse::CODE_PATH = {
	{ 400, "/400.html" },
	{ 403, "/403.html" },
	{ 404, "/404.html" },
};

HttpResponse::HttpResponse() {
	code_ = -1;
	path_ = srcdir_ = "";
	iskeepalive_ = false;
	mmfile_ = nullptr;
	mmfilestat_ = {0};
}

HttpResponse::~HttpResponse() {
	UnmapFile();
}

void HttpResponse::Init(const std::string &srcdir, std::string &path, bool iskeepalive, int code) {
	assert(srcdir != "");
	if (mmfile_) {
		UnmapFile();
	}
	code_ = code;
	iskeepalive_ = iskeepalive;
	path_ = path;
	srcdir_ = srcdir;
	mmfile_ = nullptr;
	mmfilestat_ = {0};
}

void HttpResponse::MakeResponse(Buffer &buff) {
	if (stat((srcdir_ + path_).data(), &mmfilestat_) < 0 || S_ISDIR(mmfilestat_.st_mode)) {
		code_ = 404;
	}
	else if (!(mmfilestat_.st_mode & S_IROTH)) {
		code_ = 403;
	}
	else if (code_ == -1) {
		code_ = 200;
	}
	ErrorHtml_();
	AddStateLine_(buff);
	AddHeader_(buff);
	AddContent_(buff);
}

char *HttpResponse::File() {
	return mmfile_;
}
size_t HttpResponse::FileLen() const {
	return mmfilestat_.st_size;
}

void HttpResponse::ErrorHtml_() {
	if (CODE_PATH.count(code_) == 1) {
		path_ = CODE_PATH.find(code_)->second;
		stat((srcdir_ + path_).data(), &mmfilestat_);
	}
}

void HttpResponse::UnmapFile() {
	if (mmfile_) {
		munmap(mmfile_, mmfilestat_.st_size);
		mmfile_ = nullptr;
	}
}

void HttpResponse::AddStateLine_(Buffer &buff) {
	std::string status;
	if (CODE_STATUS.count(code_) == 1) {
		status = CODE_STATUS.find(code_)->second;
	}
	else {
		code_ = 400;
		status = CODE_STATUS.find(400)->second;
	}
	buff.Append("HTTP/1.1 " + std::to_string(code_) + " " + status + "\r\n");
}

void HttpResponse::AddHeader_(Buffer &buff) {
	buff.Append("Connection: ");
	if (iskeepalive_) {
		buff.Append("keep-alive\r\n");
		buff.Append("keep-alive: max=6, timeout=120\r\n");
	}
	else {
		buff.Append("close\r\n");
	}
	buff.Append("Content-type: " + GetFileType_() + "\r\n");
}

void HttpResponse::AddContent_(Buffer &buff) {
	int srcFd = open((srcdir_ + path_).data(), O_RDONLY);
	if (srcFd < 0) {
		ErrorContent(buff, "File NotFound!");
		return;
	}
	// map to memory
	// private
	LOG_DEBUG("file path %s", (srcdir_ + path_).data());
	// kernel choose the mapped address
	int *mmRet = (int *)mmap(0, mmfilestat_.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
	if (*mmRet == -1) {
		ErrorContent(buff, "File NotFound!");
		return;
	}
	mmfile_ = (char *)mmRet;
	close(srcFd);
	buff.Append("Content-length: " + std::to_string(mmfilestat_.st_size) + "\r\n\r\n");
}

std::string HttpResponse::GetFileType_() {
	std::string::size_type idx = path_.find_last_of('.');
	if (idx == std::string::npos) {
		return "text/plain";
	}
	std::string suffix = path_.substr(idx);
	if (SUFFIX_TYPE.count(suffix) == 1) {
		return SUFFIX_TYPE.find(suffix)->second;
	}
	return "text/plain";
}

void HttpResponse::ErrorContent(Buffer &buff, std::string message) {
	std::string body;
	std::string status;
	body += "<html><title>Error</title>";
	body += "<body bgcolor=\"ffffff\">";
	if (CODE_STATUS.count(code_) == 1) {
		status = CODE_STATUS.find(code_)->second;
	}
	else {
		status = "Bad Request";
	}
	body += std::to_string(code_) + " : " + status + "\n";
	body += "<p>" + message + "</p>";
	body += "<hr><em>TinyServer</em></body></html>";

	buff.Append("Content-length: " + std::to_string(body.size()) + "\r\n\r\n");
	buff.Append(body);
}

