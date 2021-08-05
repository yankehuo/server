#include "config.h"

Config::Config() {
	PORT = 6666;
	// ET
	TRIGMODE = 3;
	TIMEOUTMS = 5000;
	// elegant
	OPENLINGER = false;
	// threadpool number
	THREADNUM = 8;
	// open log ?
	OPENLOG = true;
	LOGLEVEL = 1;
	// blockqueue size about log
	LOGQUESIZE = 1024;
	// modified 1-->reactor
	ACTOR = 1;
}

void Config::parse_arg(int argc, char **argv) {
	int opt;
	const char *str = "p:m:t:e:n:o:a:";
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
				OPENLINGER = atoi(optarg);
				break;
			case 'n':
				THREADNUM = atoi(optarg);
				break;
			case 'o':
				OPENLOG = atoi(optarg);
				break;
			case 'a':
				ACTOR = atoi(optarg);
				break;
			default:
				break;
		}
	}
}
