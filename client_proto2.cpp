/*
 * IRC Client prototype 2
 * - OOP, modularity
 * - handle 1 connection at once
 * - send IRC messages
 * - print recieved messages
 * - encode and decode commands and messages
 */
#include "client.h"  // cin; cout

// mutex to synchronize access to shared data
std::mutex mtx;

int main(int argc, char *argv[])
try {
	if (argc != 2) {
		throw std::runtime_error("usage: client hostname\n");
	}

	Client::Connection connection;

	// connect to host name specified
	connection.connectToServer(argv[1]);

	// create a separate thread for recieving and displaying data
	std::thread recieve_thread(connection.recieveData);
	// main loop for user input
	while (true) {
		// handle user input
	}

	// join separate thread
	recieve_thread.join();

} catch (std::runtime_error& e) {
	std::cout << e.what();
}
