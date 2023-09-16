#include "client.h"

// convert sockaddr to sockaddr_in or sockaddr_in6 (IPv4 or IPv6)
void *Client::getInAddr(SA *sa)
{
	if (sa->sa_family == AF_INET)
		return &((reinterpret_cast<SA_in*>(sa))->sin_addr);

	return &((reinterpret_cast<SA_in6*>(sa))->sin6_addr);
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

Client::Connection::Connection()
{
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// unspecified IP version
	hints.ai_socktype = SOCK_STREAM; 	// TCP
}

void Client::Connection::connectToServer(char *hstnm)
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

// send data
void Client::Connection::sendData(char *data, int data_size)
{
	if ((send(sockfd, data, data_size, 0)) < 0)
		std::cout << "Send error\n";
}

// recieve data
void Client::Connection::recieveData()
{
	int connection_state = 1; 
	char state_buffer[1];
	// while connection is up
	while (true) {
		// recieve data, feed it into a buffer and display it as string
		int bytes_recieved;
		char buffer[MAXMSGLEN];
		if ((bytes_recieved = recv(sockfd, buffer, MAXMSGLEN, 0)) > 0) {
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
