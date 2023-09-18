/*
 * IRC Client prototype
 * - handle 1 connection at once
 * - send raw data from user input
 * - print recieved raw data
 */
#include <iostream>  // cin; cout
#include <string>    // std::string
#include <stdexcept> // throwing exceptions
#include <string.h>  // memset()
#include <unistd.h>  // close()
// multithreading
#include <mutex>
#include <thread>
// socket & networking headers
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXMSGLEN 512 // max message length specified by IRC protocol

#define PORT_PLAIN "6667" // TCP/6667 port for plaintext connection

using SA = struct sockaddr;
using SA_in = struct sockaddr_in;
using SA_in6 = struct sockaddr_in6;
using AI = struct addrinfo;

// mutex to synchronize access to shared data
std::mutex mtx;

// convert sockaddr to sockaddr_in or sockaddr_in6 (IPv4 or IPv6)
void *getInAddr(SA *sa)
{
	if (sa->sa_family == AF_INET)
		return &((reinterpret_cast<SA_in*>(sa))->sin_addr);

	return &((reinterpret_cast<SA_in6*>(sa))->sin6_addr);
}

// get server information and try to establish successfull connection
void connectToServer(int& sockfd, char *host, AI hints)
{
	// address info structs
	AI *server_info, *iter;
	// ip address string
	char ipstr[INET6_ADDRSTRLEN];

	// get address information of chosen server
	int rv = getaddrinfo(host, PORT_PLAIN, &hints, &server_info);

	if (rv != 0) {
		throw std::runtime_error("error: couldn't get addrinfo\n");
	}

	// try each address from results until successfull connection
	for (iter = server_info; iter != nullptr; iter = iter->ai_next) {
		// create socket file descriptor
		if ((sockfd = socket(iter->ai_family, iter->ai_socktype,
			iter->ai_protocol)) == -1) {
			continue;
		}	
		// connect socket file descriptor
		if (connect(sockfd, iter->ai_addr, iter->ai_addrlen) == -1) {
			close(sockfd);
			continue;
		}
		break;
	}

	if(!iter) {
		throw std::runtime_error("client: failed to connect\n");
	}

	// translate ip address from bytes to readable representation
	inet_ntop(iter->ai_family,
		getInAddr(reinterpret_cast<SA*>(iter->ai_addr)),
		ipstr, INET6_ADDRSTRLEN);

	std::cout << "client: connecting to " << ipstr << '\n';

	// no longer needed
	freeaddrinfo(server_info);	
}

// get input from the user, prepare it and send to the peer
void handleUserInput(int socket)
{
	// clear character input stream
	std::cin >> std::ws;
	// get line of characters from user
	std::string data;
	std::getline(std::cin, data);
	// truncate data string to MAXMSGLEN - 2 characters
	if (data.size() > MAXMSGLEN - 2)
		data.erase(MAXMSGLEN - 2, std::string::npos);
	// "IRC messages are always lines of characters terminated with
	// CR-LF (Carriage Return - Line Feed) pair"
	data.push_back(0x0D);
	data.push_back(0x0A);	
	// send data
	if ((send(socket, data.c_str(), data.size(), 0)) < 0)
		std::cout << "Send error\n";
}

// closed connection message
void connectionClosed(const int& connection) 
{
	if (connection == 0) {
		std::cout << "Connection closed by the server\n";
	} else if (connection == -1) {
		switch (errno) {
		case ECONNRESET:
			std::cout << "Connection reset by the server\n";
		case ETIMEDOUT:
			std::cout << "Connection time out\n";
		default:
			std::cout << "Connection error\n";
		}
	}
}

// handle incoming data
void recieveAndDisplay(int socket)
{
	int connection_state = 1; 
	char state_buffer[1];
	// while connection is up
	while (true) {
		// recieve data, feed it into a buffer and display it as string
		int bytes_recieved;
		char buffer[MAXMSGLEN];
		if ((bytes_recieved = recv(socket, buffer, MAXMSGLEN, 0)) > 0) {
			// mutex used to protect the cout stream
			std::lock_guard<std::mutex> lock(mtx);

			std::cout << std::string(buffer, bytes_recieved);
		} else {
			connectionClosed(bytes_recieved);
			break;
		}
	}
	return;
}



int main(int argc, char *argv[])
try {
	// socket
	int sockfd;
	// server host name string
	char *host_name = argv[1];
	// address info struct for connection options
	AI hints;

	if (argc != 2) {
		throw std::runtime_error("usage: client hostname\n");
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// unspecified IP version
	hints.ai_socktype = SOCK_STREAM; 	// TCP
	
	// get server information and connect the socket
	connectToServer(sockfd, host_name, hints);
		
	// create a separate thread for recieving and displaying data
	std::thread recieve_thread(recieveAndDisplay, sockfd);

	// main loop for user input
	while (true) {
		handleUserInput(sockfd);
	}

	// join separate thread
	recieve_thread.join();

	close(sockfd);

} catch (std::runtime_error& e) {
	std::cerr << e.what();
}
