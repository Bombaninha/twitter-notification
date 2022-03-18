#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include "command/command.hpp"

class ClientSocket {
    int sockfd;
    int port;

    public:
        ClientSocket(int port);
        ~ClientSocket();
        void run();

    private:
        Command execute(Command command);
};

#endif