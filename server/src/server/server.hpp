#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <thread>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <tuple>

#include "command/command.hpp"
#include "connection/connection.hpp"

#include "../clientSocket/clientSocket.hpp"

#include "../table/tableRow.hpp"

class Server {
    int sockfd;
    int nextClientPort = 10000;
    std::vector<ClientSocket*> clients;
    std::vector<std::thread*> clientThreads;
    bool isPrimary = false;
    int primaryPort = 0;
    Connection *primaryConnection;
    std::vector<std::tuple<int,std::string,int>> backupServers;

    public:
        Server(int port);
        Server(int port, std::string primaryHost, int primaryPort);
        ~Server();
        void setPrimary();
        std::thread run();
        void replicate(Command command, std::string username);
        void replicateDisconnect(std::string username, std::string host, int port);
        void replicateFollow(std::string username, std::string follow);
        void replicateSend(std::string username, std::string message);

    private:
        void serverLoop();
        Command execute(Command command, struct sockaddr_in client_addr);
        void createClientSocket(std::string host, int port, std::string profile, struct sockaddr_in client_addr);
        void backupLoop();
        void primaryLoop();
        void electionLoop();
        void changePort();
};

#endif