// Main client header file
// Dependencies:
#include <iostream>  // cin; cout
#include <string>    // std::string
#include <vector>    // std::vector
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

// Macros:
#define MAXMSGLEN 512 // max message length specified by IRC protocol

#define PORT_PLAIN "6667" // TCP/6667 port for plaintext connection

// nickname, username and realname passed to the server by the client
#define NICKNAME "ncknm_1"
#define	USERNAME "usrnm_1"
#define REALNAME "rlnm_1"

using SA = struct sockaddr;
using SA_in = struct sockaddr_in;
using SA_in6 = struct sockaddr_in6;
using AI = struct addrinfo;

enum class Command { CAP, NICK, USER, JOIN, PRIVMSG };

namespace Client {
	// convert sockaddr to sockaddr_in or sockaddr_in6 (IPv4 or IPv6)
	void *getInAddr(SA *sa);

	// closed connection message
	void connectionClosed(int connection); 

	void getUserInput(std::string& message);

	std::string command(Command cmd, std::vector<std::string> params);
}
