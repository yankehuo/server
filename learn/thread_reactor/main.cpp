#include <unistd.h>
#include "server/webserver.h"
#include "config/config.h"

int main(int argc, char **argv) {
	Config config;
	config.parse_arg(argc, argv);

	// WebServer server(6666, 3, 60000, false, 6, true, 1, 1024);
	WebServer server(config.PORT, config.TRIGMODE, config.TIMEOUTMS, config.OPTLINGER, config.THREADNUM, config.OPENLOG, config.LOGLEVEL, config.LOGQUESIZE);
	server.Start();
}
