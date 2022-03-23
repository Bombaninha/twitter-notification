#include "connection.hpp"

#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>

Connection::Connection(std::string serverHost, int serverPort) {
    struct hostent* host = gethostbyname(serverHost.c_str());

    if (host == NULL) {
        throw std::runtime_error("Could not resolve hostname");
        exit(EXIT_FAILURE);
    }

    this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    // TIMEOUT
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    if (setsockopt(this->sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        perror("Error");
    }

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

void Connection::sendCommand(Command command) {
    int n = sendto(
        this->sockfd,
        std::string(command).c_str(),
        std::string(command).length(),
        0,
        (struct sockaddr*) &serverConnProps,
        sizeof(serverConnProps));

    if (n < 0) {
        //return Command(COMMAND_ERROR, "Could not send command");
        std::cout << "Could not send command" << std::endl;
    }
}

std::string Connection::listenToServer() {
    char buffer[256];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    bzero(buffer, 256);

    int recieve = recvfrom(this->sockfd, buffer, 256, 0, (struct sockaddr *) &client_addr, &client_addr_len);

    if (recieve < 0) {
        return "";
    }

    return std::string(buffer);
}