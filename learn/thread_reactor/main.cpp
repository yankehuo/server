#include <unistd.h>
#include "server/webserver.h"

int main() {
	WebServer server(6666, 3, 60000, false, 6, true, 1, 1024);
	server.Start();
}
