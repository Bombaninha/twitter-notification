#include "command.hpp"

#include <iostream>

#define CONNECT_STRING "CONNECT"
#define FOLLOW_STRING "FOLLOW"
#define SEND_STRING "SEND"
#define REDIRECT_STRING "REDIRECT"
#define ERROR_STRING "ERROR"

Command::Command(std::string command) {
    int splitPosition = command.find_first_of(" ");

    if (splitPosition == std::string::npos) {
        throw std::invalid_argument("Invalid command");
    } else {
        if (command.substr(0, splitPosition) == CONNECT_STRING) {
            this->type = COMMAND_CONNECT;
            this->data = command.substr(splitPosition + 1);
        } else if (command.substr(0, splitPosition) == FOLLOW_STRING) {
            this->type = COMMAND_FOLLOW;
            this->data = command.substr(splitPosition + 1);
        } else if (command.substr(0, splitPosition) == SEND_STRING) {
            this->type = COMMAND_SEND;
            this->data = command.substr(splitPosition + 1);
        } else if (command.substr(0, splitPosition) == REDIRECT_STRING) {
            this->type = COMMAND_REDIRECT;
            this->data = command.substr(splitPosition + 1);
        } else if (command.substr(0, splitPosition) == ERROR_STRING) {
            this->type = COMMAND_ERROR;
            this->data = command.substr(splitPosition + 1);
        } else {
            throw std::invalid_argument("Invalid command");
        }

        this->data = command.substr(splitPosition + 1);
    }
}

Command::Command(CommandType type, std::string data) {
    this->type = type;
    this->data = data;
}

Command::operator std::string() {
    std::string command = "";

    switch (this->type) {
        case COMMAND_CONNECT:
            command = std::string(CONNECT_STRING) + " " + this->data;
            break;
        case COMMAND_FOLLOW:
            command = std::string(FOLLOW_STRING) + " " + this->data;
            break;
        case COMMAND_SEND:
            command = std::string(SEND_STRING) + " " + this->data;
            break;
        case COMMAND_REDIRECT:
            command = std::string(REDIRECT_STRING) + " " + this->data;
            break;
        case COMMAND_ERROR:
            command = std::string(ERROR_STRING) + " " + this->data;
            break;
    }

    return command;
}

CommandType Command::getType() {
    return this->type;
}

std::string Command::getData() {
    return this->data;
}