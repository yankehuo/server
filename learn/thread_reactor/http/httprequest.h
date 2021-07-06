#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
// regular expressions
#include <regex>
#include <cerrno>

// mysql

#include "../buffer/buffer.h"
#include "../log/log.h"

// sqlpool

class HttpReuqest {
private:
	PARSE_STATE state_;
	std::string method_, path_, version_, body_;
	// 
	std::unordered_map<std::string, std::string> header_;
	//
	std::unordered_map<std::string, std::string> post_;

	static const std::unordered_set<std::string> DEFAULT_HTML;
	static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
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
		NO_REQUEST,
		FORBIDDENT_REQUEST,
		FILE_REQUEST,
		INTERNAL_ERROR,
		CLOSED_CONNECTION,
	};

	HttpReuqest() {
		Init();
	}
	~HttpReuqest() = default;

	void Init();
	bool parse(Buffer &buff);

	std::string path() const;
	std::string &path();
	std::string method() const;
	std::string version() const;
	//
	std::string GetPost(const std::string &key) const;
	std::string GetPost(const char *key) const;

	bool IsKeepAlive() const;
private:
	bool ParseRequestLine_(const std::string &line);
	void ParseHeader_(const std::string &line);
	void ParseBody_(const std::string &line);

	void ParsePath_();
	// ?
	void ParsePost_();
	void ParseFromUrlencoded_();

	// ?
	static bool UserVerify(const std::string &name, const std::string &pwd, bool isLogin);

	static int ConverHex(char ch);
};

#endif
