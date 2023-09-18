// Connection class header file
// Dependencies:
#include "client.h"

class Connection {
	public:
		// constructor
		Connection();
		// establish new connection
		void connectToServer(char *hstnm);
		// connection registration
		void registerConnection();
		// joining channel
		void joinChannel(char *chnnl);
		// send data
		void sendData(const char *data, int data_size);
		// recieve data
		void recieveData(std::mutex *mtx);
	private:
		// socket file descriptor
		int sockfd = 0;
		// server host name
		std::string host_name;
		// connection specification
		AI hints;

		std::string nickname;
		std::string username;
		std::string realname;

		// channel to join after connection registration
		std::string channel;
};
