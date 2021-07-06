#include "httprequest.h"

const std::unordered_set<std::string> HttpReuqest::DEFAULT_HTML {
	"/index", "picture",
};
// 
const std::unordered_map<std::string, int> HttpReuqest::DEFAULT_HTML_TAG {
};

void HttpReuqest::Init() {
	method_ = path_ = version_ = body_ = "";
	state_ = REQUEST_LINE;
	header_.clear();
	// 
	post_.clear();
}

bool HttpReuqest::IsKeepAlive() const {
	if (header_.count("Connection") == 1) {
		return head_.find("Connection")->second == "keep_alive" && version_ == "1.1";
	}
	return false;
}

bool HttpReuqest::parse(Buffer &buff) {
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

void HttpReuqest::ParsePath_() {
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

bool HttpReuqest::ParseRequestLine_(const string &line) {
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

void HttpReuqest::ParseHeader_(const string &line) {
	std::regex pattern("^([^:]*): ?(.*)$");
	std::smatc subMatch;
	if (std::regex_match(line, subMatch, pattern)) {
		header_[subMatch[1]] = subMatch[2];
	}
	else {
		state_ = BODY;
	}
}
void HttpReuqest::ParseBody_(const string &line) {
	body_ = line;
	// 
	ParsePost_();
	state_ = FINISH;
	LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());

}
