#include "config.h"

Config::Config() {
	PORT = 6666;
	// ET
	TRIGMODE = 3;
	TIMEOUTMS = 60000;
	// elegant
	OPTLINGER = false;
	// threadpool number
	THREADNUM = 8;
	// open log ?
	OPENLOG = true;
	LOGLEVEL = 1;
	// blockqueue size about log
	LOGQUESIZE = 1024;
}

void Config::parse_arg(int argc, char **argv) {
	int opt;
	const char *str = "p:m:t:e:n:o:";
	while ((opt = getopt(argc, argv, str)) != -1) {
		switch (opt) {
			case 'p':
				PORT = atoi(optarg);
				break;
			case 'm':
				TRIGMODE = atoi(optarg);
				break;
			case 't':
				TIMEOUTMS = atoi(optarg);
				break;
			case 'e':
				OPTLINGER = atoi(optarg);
				break;
			case 'n':
				THREADNUM = atoi(optarg);
				break;
			case 'o':
				OPENLOG = atoi(optarg);
				break;
			default:
				break;
		}
	}
}
