#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include <string>

#include "command/command.hpp"

class ClientSocket {
    int sockfd;
    int port;
    std::string profile;

    public:
        ClientSocket(int port, std::string profile);
        ~ClientSocket();
        void run();

    private:
        Command execute(Command command);
};

#endif