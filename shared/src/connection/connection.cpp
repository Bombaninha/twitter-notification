#include "connection.hpp"

#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

Connection::Connection(std::string serverHost, int serverPort) {
    struct hostent* host = gethostbyname(serverHost.c_str());

    if (host == NULL) {
        throw std::runtime_error("Could not resolve hostname");
        exit(EXIT_FAILURE);
    }

    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (this->sockfd < 0) {
        throw std::runtime_error("Could not create socket");
        exit(EXIT_FAILURE);
    }

    serverConnProps.sin_family = AF_INET;
    serverConnProps.sin_port = htons(serverPort);
    serverConnProps.sin_addr = *((struct in_addr*) host->h_addr);
    bzero(&(serverConnProps.sin_zero), 8);
}

Connection::~Connection() {
    close(this->sockfd);
}

Command Connection::sendCommand(Command command) {
    int n = sendto(
        this->sockfd,
        std::string(command).c_str(),
        std::string(command).length(),
        0,
        (struct sockaddr*) &serverConnProps,
        sizeof(serverConnProps));

    if (n < 0) {
        return Command(COMMAND_ERROR, "Could not send command");
    }

    char buffer[256];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    bzero(buffer, 256);

    int recieve = recvfrom(this->sockfd, buffer, 256, 0, (struct sockaddr *) &client_addr, &client_addr_len);

    if (recieve < 0) {
        return Command(COMMAND_ERROR, "Could not recieve response");
    }

    return Command(std::string(buffer));
}