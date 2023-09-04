/*
 * IRC Client prototype 2
 * - OOP, modularity
 * - handle 1 connection at once
 * - send IRC messages
 * - print recieved messages
 * - encode and decode commands and messages
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

class Connection {
public:
	// constructor
	Connection();
	// establish new connection
	void connectToServer(char *hstnm);
private:
	// socket file descriptor
	int sockfd = 0;
	// server host name
	std::string host_name;
	// connection specification
	AI hints;
};

Connection::Connection()
{
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// unspecified IP version
	hints.ai_socktype = SOCK_STREAM; 	// TCP
}

void Connection::connectToServer(char *hstnm)
{
	// address info structs
	AI *server_info, *iter;
	// ip address string
	char ipstr[INET6_ADDRSTRLEN];

	host_name = hstnm;

	// get address information of chosen server
	int rv = getaddrinfo(host_name.c_str(), PORT_PLAIN,
		&hints, &server_info);

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

int main(int argc, char *argv[])
try {
	if (argc != 2) {
		throw std::runtime_error("usage: client hostname\n");
	}

	Connection connection;

	connection.connectToServer(argv[1]);

} catch (std::runtime_error& e) {
	std::cout << e.what();
}
