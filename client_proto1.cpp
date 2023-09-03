/*
 * IRC Client prototype
 * - handle 1 connection at once
 * - send raw data from user input
 * - print recieved raw data
 */
#include <iostream>  // cin; cout
#include <string>    // std::string
#include <stdexcept> // std::runtime_error()
#include <string.h>  // memset()
#include <unistd.h>   // close()
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

// convert sockaddr to sockaddr_in or sockaddr_in6 (IPv4 or IPv6)
void *get_in_addr(SA *sa)
{
	if (sa->sa_family == AF_INET)
		return &((reinterpret_cast<SA_in*>(sa))->sin_addr);

	return &((reinterpret_cast<SA_in6*>(sa))->sin6_addr);
}

void get_user_input(char *buf)
{
	std::string data;
	std::getline(std::cin, data);
	if (data.size() <= MAXMSGLEN)
		strcpy(buf,data.c_str());
}

void connection_closed(const int& connection) 
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

int main(int argc, char *argv[])
try {
	// socket, actual size of recieved data
	int sockfd, msgsize;
	// message (server-to-client) & command (client-to-server) buffers
	char msgbuffer[MAXMSGLEN], cmmndbuffer[MAXMSGLEN];
	// ip address string
	char ipstr[INET6_ADDRSTRLEN];

	AI hints, *server_info, *iter;

	if (argc != 2) {
		throw std::runtime_error("usage: client hostname/n");
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// unspecified IP version
	hints.ai_socktype = SOCK_STREAM; 	// TCP
	
	// get address information of chosen server
	int rv;
	if ((rv = getaddrinfo(argv[1], PORT_PLAIN,
		&hints, &server_info)) != 0) {
		throw std::runtime_error("error: couldn't get addrinfo");
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
		get_in_addr(reinterpret_cast<SA*>(iter->ai_addr)),
		ipstr, INET6_ADDRSTRLEN);

	std::cout << "client: connecting to " + std::string(ipstr) + '\n';

	// all done with server_info struct; free memory
	freeaddrinfo(server_info);
	
	// buffer & connection state variable for connection state checking
	char buffer;
	int connection = 1;
	// main loop
	while ((connection = recv(sockfd, &buffer, 1, MSG_PEEK)) > 0) {
		// send data from input
		memset(cmmndbuffer,0,MAXMSGLEN);
		get_user_input(cmmndbuffer);
		if ((send(sockfd, cmmndbuffer, MAXMSGLEN, 0)) <= 0)
			std::cout << "Message not sent\n";
		// if any data recieved
		if ((msgsize = recv(sockfd, msgbuffer, MAXMSGLEN, 0)) > 0) {
			// print the message
			std::cout << msgbuffer;
			// empty the message buffer
			memset(msgbuffer, 0, MAXMSGLEN);
		}
	}

	connection_closed(connection);	
	close(sockfd);

} catch (std::runtime_error& e) {
	std::cerr << e.what();
}
