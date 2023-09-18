// Main client definitions
// Dependencies:
#include "client.h"

// convert sockaddr to sockaddr_in or sockaddr_in6 (IPv4 or IPv6)
void *Client::getInAddr(SA *sa)
{
	if (sa->sa_family == AF_INET)
		return &((reinterpret_cast<SA_in*>(sa))->sin_addr);

	return &((reinterpret_cast<SA_in6*>(sa))->sin6_addr);
}

// closed connection message
void Client::connectionClosed(int connection) 
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

void Client::getUserInput(std::string& message)
{
	// clear character input stream
	std::cin >> std::ws;
	// get line of characters from user
	std::getline(std::cin, message);
}

std::string Client::command(Command cmd, std::vector<std::string> params)
{
	// string variable to be returned
	std::string cmd_rtrnd;

	switch(cmd) {
	// CAP <subcommand> [:<capabilities>]
	case Command::CAP:
		if (params.size() < 1)
			throw std::runtime_error("Error: CAP command parameters\n");
		else
			cmd_rtrnd = "CAP ";
			for (std::string param : params) {
				cmd_rtrnd += " " + param;
			}
		break;
	// NICK <nickname>
	case Command::NICK:
		if (params.size() != 1)
			throw std::runtime_error("Error: NICK command parameters\n");
		else
			cmd_rtrnd = "NICK " + params[0];
		break;
	// USER <username> 0 * <realname>
	case Command::USER:
		if (params.size() != 2)
			throw std::runtime_error("Error: USER command parameters\n");
		else
			cmd_rtrnd = "USER " + params[0] + " 0 * " + params[1];
		break;
	// JOIN <channel>{, <channel>} [<key>{,<key>}]
	case Command::JOIN:
		if (params.size() < 1)
			throw std::runtime_error("Error: JOIN command parameters\n");
		else
			cmd_rtrnd = "JOIN " + params[0];
		break;
	// PRIVMSG <target>{,<target>} <text to be sent>
	case Command::PRIVMSG:
		break;
	}

	// CR-LF (Carriage return - Line feed) characters
	cmd_rtrnd.push_back(0x0D);
	cmd_rtrnd.push_back(0x0A);

	return cmd_rtrnd;
}
