#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
// regular expressions
#include <regex>
#include <cerrno>

#include "../buffer/buffer.h"
#include "../logbq/log.h"

class HttpRequest {
public:
	enum PARSE_STATE {
		REQUEST_LINE = 0,
		HEADERS,
		BODY,
		FINISH,
	};

	enum HTTP_CODE {
		NO_REQUEST = 0,
		GET_REQUEST,
		BAD_REQUEST,
		NO_RESOURCE,
		FORBIDDENT_REQUEST,
		FILE_REQUEST,
		INTERNAL_ERROR,
		CLOSED_CONNECTION,
	};

	HttpRequest() {
		Init();
	}
	~HttpRequest() = default;

	void Init();
	bool Parse(Buffer &buff);

	std::string Path() const;
	std::string &Path();
	std::string Method() const;
	std::string Version() const;

	bool IsKeepAlive() const;
private:
	bool ParseRequestLine_(const std::string &line);
	void ParseHeader_(const std::string &line);
	void ParseBody_(const std::string &line);
	void ParsePath_();
private:
	PARSE_STATE state_;
	std::string method_, path_, version_, body_;
	std::unordered_map<std::string, std::string> header_;

	static const std::unordered_set<std::string> DEFAULT_HTML;
};

#endif
