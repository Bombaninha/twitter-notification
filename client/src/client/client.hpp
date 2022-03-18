#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <queue>
#include <thread>
#include <ncurses.h>

#include "connection/connection.hpp"

class Client {
    std::queue<std::string> commands;
    std::queue<std::string> responses;
    std::thread uiThread;
    std::thread serverThread;
    std::string serverAddress;
    int serverPort;
    std::string profile;
    bool running;
    Connection* serverConnection;

    public:
        Client(std::string profile, std::string serverHost, int serverPort);
        ~Client();
        void run();

    private:
        void handleUI();
        void handleServer();
        void processCommand(std::string command);
        void updateOutput(WINDOW* output, std::vector<std::string>& outputContent);
        void connectToServer();
};

#endif