#include <iostream>
#include <thread>
#include <string>

#include "server/server.hpp"

Server* server;

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 5) {
        std::cout << "Usage: ./server [-p <port>] [-b <port> <primary_host> <primary_port>]" << std::endl;
        return 1;
    }

    if (std::string(argv[1]).compare("-p") == 0) {
        std::cout << "Starting server as primary on port " << argv[2] << std::endl;
        
        server = new Server(atoi(argv[2]));
        server->setPrimary();
    } else {
        std::cout << "Starting server as backup on port " << argv[2] << std::endl;
        
        server = new Server(atoi(argv[2]), std::string(argv[3]), atoi(argv[4]));
    }

    std::thread serverThread = server->run();

    std::cout << "Server started" << std::endl;

    serverThread.join();

    delete server;
}