#include <iostream>
#include <thread>

void Hello() {
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout << "Hello, World!" << std::endl;
}

int main() {
	std::thread t(&Hello);
	t.join();
	return 0;

}
