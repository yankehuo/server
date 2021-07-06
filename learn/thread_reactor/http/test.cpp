#include <iostream>

int main() {
	const char CRLF[] = "\r\n";
	const char CRLF2[] = "lrln";
	std::cout << sizeof(CRLF) << std::endl;
	std::cout << sizeof(CRLF2) << std::endl;
	return 0;
}
