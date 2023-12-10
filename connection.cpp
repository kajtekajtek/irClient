// Connection class definitions
// Dependencies:
#include "connection.h"

Connection::Connection()
:nickname { NICKNAME }, username { USERNAME }, realname { REALNAME }
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
		Client::getInAddr(reinterpret_cast<SA*>(iter->ai_addr)),
		ipstr, INET6_ADDRSTRLEN);

	std::cout << "client: connecting to " << ipstr << '\n';

	// no longer needed
	freeaddrinfo(server_info);
}

// send data
void Connection::sendData(const char *data, int data_size)
{
	if ((send(sockfd, data, data_size, 0)) < 0)
		std::cout << "Send error\n";
}

// recieve data
void Connection::recieveData(std::mutex *mtx)
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
			std::lock_guard<std::mutex> lock(*mtx);

			std::cout << std::string(buffer, bytes_recieved);
		} else {
			Client::connectionClosed(bytes_recieved);
			break;
		}
	}
	return;

}

void Connection::registerConnection()
{
	std::string cmd;

	// CAP LS 302
	cmd = Client::command(Command::CAP, {"LS", "302"});
	sendData(cmd.c_str(), cmd.size());

	// NICK <nickname>
	cmd = Client::command(Command::NICK, {nickname});
	sendData(cmd.c_str(), cmd.size());

	// USER <username> 0 * <realname>
	cmd = Client::command(Command::USER, {username, realname});
	sendData(cmd.c_str(), cmd.size());

	// CAP END
	cmd = Client::command(Command::CAP, {"END"});
	sendData(cmd.c_str(), cmd.size());
}

void Connection::joinChannel(char *chnnl)
{
	std::string cmd;

	channel = "#";
	channel += chnnl;

	// JOIN <channel>
	cmd = Client::command(Command::JOIN, {channel});
	sendData(cmd.c_str(), cmd.size());
}
