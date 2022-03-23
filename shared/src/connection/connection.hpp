#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <netdb.h>
#include "../command/command.hpp"

class Connection {
    public:
        Connection(std::string serverHost, int serverPort);
        ~Connection();
        void sendCommand(Command command);
        std::string listenToServer();
    private:
        int sockfd;
        struct sockaddr_in serverConnProps;
};

#endif