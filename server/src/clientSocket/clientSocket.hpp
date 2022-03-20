#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include <string>
#include <map>

#include "command/command.hpp"
#include "../table/tableRow.hpp"

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