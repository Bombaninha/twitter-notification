#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <netdb.h>
#include "../command/command.hpp"

class Connection {
    public:
        Connection(std::string serverHost, int serverPort);
        ~Connection();
        Command sendCommand(Command command);
    private:
        int sockfd;
        struct sockaddr_in serverConnProps;
};

#endif