/*
 * IRC Client prototype 2
 * - OOP, modularity
 * - handle 1 connection at once
 * - send IRC messages
 * - print recieved messages
 * - encode and decode commands and messages
 */
#include "connection.h"

// mutex to synchronize access to shared data
std::mutex mtx;

int main(int argc, char *argv[])
try {
	if (argc != 3) {
		throw std::runtime_error("usage: irClient hostname channel\n");
	}

	Connection connection;

	// join the server specified in first argument
	connection.connectToServer(argv[1]);

	// create a separate thread for recieving and displaying data
	std::thread recieve_thread(&Connection::recieveData,
			&connection, &mtx);

	// IRC client-server connection registration sequence
	connection.registerConnection();

	// join the channel specified in second argument
	connection.joinChannel(argv[2]);

	// main loop for user input
	std::string message;
	while (true) {
		Client::getUserInput(message);

		// prepare IRC compatible message command
		message = Client::command(Command::PRIVMSG,
				{connection.channelName(), message});

		connection.sendData(message.c_str(), message.size());

	}

	// join separate thread
	recieve_thread.join();

} catch (std::runtime_error& e) {
	std::cout << e.what();
}
