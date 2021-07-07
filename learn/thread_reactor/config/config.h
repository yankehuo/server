#ifndef CONFIG_H
#define CONFIG_H

#include "../server/webserver.h"

class Config {
public:
	Config();
	~Config() {}

	void parse_arg(int argc, char **argv);

	int PORT; // 6666
	int TRIGMODE; // 3 --> ET 
	int TIMEOUTMS; // 60000 ms
	bool OPTLINGER; // false --> elegant exit
	int THREADNUM; // 6
	bool OPENLOG; // true
	int LOGLEVEL; // 1
	int LOGQUESIZE; // 1024 --> log queue size
};

#endif
