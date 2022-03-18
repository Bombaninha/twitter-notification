#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <thread>
#include <vector>

#include "command/command.hpp"

#include "../clientSocket/clientSocket.hpp"

class Server {
    int sockfd;
    int nextClientPort = 10000;
    std::vector<ClientSocket*> clients;
    std::vector<std::thread*> clientThreads;

    public:
        Server(int port);
        ~Server();
        std::thread run();

    private:
        void serverLoop();
        Command execute(Command command);
        void createClientSocket(int port);
};

#endif