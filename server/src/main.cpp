#include <iostream>
#include <thread>

#include "server/server.hpp"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage: ./server <port>" << std::endl;
        return 1;
    }

    std::cout << "Starting server on port " << argv[1] << std::endl;

    Server server(atoi(argv[1]));
    std::thread serverThread = server.run();

    std::cout << "Server started" << std::endl;

    serverThread.join();
}