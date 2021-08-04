#include "httprequest.h"

const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML {
	"/index", "/home",
};

void HttpRequest::Init() {
	method_ = path_ = version_ = body_ = "";
	state_ = REQUEST_LINE;
	header_.clear();
}

bool HttpRequest::IsKeepAlive() const {
	if (header_.count("Connection") == 1) {
		return header_.find("Connection")->second == "keep_alive" && version_ == "1.1";
	}
	return false;
}

bool HttpRequest::parse(Buffer &buff) {
	// size : 3 : 1 2 \0
	const char CRLF[] = "\r\n";
	if (buff.ReadableBytes() <= 0) {
		return false;
	}
	while (buff.ReadableBytes() && state_ != FINISH) {
		const char *lineEnd = std::search(buff.Peek(), buff.BeginWriteConst(), CRLF, CRLF + 2);
		std::string line(buff.Peek(), lineEnd);
		switch (state_) {
			case REQUEST_LINE:
				if (!ParseRequestLine_(line)) {
					return false;
				}
				ParsePath_();
				break;
			case HEADERS:
				ParseHeader_(line);
				if (buff.ReadableBytes() <= 2) {
					state_ = FINISH;
				}
				break;
			case BODY:
				ParseBody_(line);
				break;
			default:
				break;
		}
		if (lineEnd == buff.BeginWrite()) {
			break;
		}
		buff.RetrieveUntil(lineEnd + 2);
	}
	LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
	return true;
}

void HttpRequest::ParsePath_() {
	if (path_ == "/") {
		path_ = "/index.html";
	}
	else {
		for (auto &item : DEFAULT_HTML) {
			if (item == path_) {
				path_ += ".html";
				break;
			}
		}
	}
}

bool HttpRequest::ParseRequestLine_(const std::string &line) {
	std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
	std::smatch subMatch;
	if (std::regex_match(line, subMatch, pattern)) {
		method_ = subMatch[1];
		path_ = subMatch[2];
		version_ = subMatch[3];
		state_ = HEADERS;
		return true;
	}
	LOG_ERROR("RequestLine Error");
	return false;
}

void HttpRequest::ParseHeader_(const std::string &line) {
	std::regex pattern("^([^:]*): ?(.*)$");
	std::smatch subMatch;
	if (std::regex_match(line, subMatch, pattern)) {
		header_[subMatch[1]] = subMatch[2];
	}
	else {
		state_ = BODY;
	}
}
void HttpRequest::ParseBody_(const std::string &line) {
	body_ = line;
	state_ = FINISH;
	LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}


std::string HttpRequest::path() const {
	return path_;
}
std::string &HttpRequest::path() {
	return path_;
}
std::string HttpRequest::method() const {
	return method_;
}
std::string HttpRequest::version() const {
	return version_;
}

