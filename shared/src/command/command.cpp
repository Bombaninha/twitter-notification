#include "command.hpp"

#include <iostream>

#define CONNECT_STRING "CONNECT"
#define FOLLOW_STRING "FOLLOW"
#define SEND_STRING "SEND"
#define REDIRECT_STRING "REDIRECT"
#define EXIT_STRING "EXIT"
#define ERROR_STRING "ERROR"
#define TWEET_STRING "TWEET"
#define NOOP_STRING "NOOP"
#define NOTIFICATION_STRING "NOTIFICATION"
#define ALIVE_STRING "ALIVE"
#define BACKUP_STRING "BACKUP"
#define TIC_STRING "TIC"
#define TOC_STRING "TOC"
#define ELECTION_STRING "ELECTION"
#define COORDINATOR_STRING "COORDINATOR"
#define ANSWER_STRING "ANSWER"
#define REPLICATE_STRING "REPLICATE"
#define REPLICATE_CONNECT_STRING "REPLICATE_CONNECT"

Command::Command(std::string command) {
    if (command == std::string(NOOP_STRING)) {
        this->type = NO_OPERATION;
        this->data = "";
        return;
    }

    int splitPosition = command.find_first_of(" ");

    if (splitPosition == std::string::npos) {
        if (command == TIC_STRING) {
            this->type = COMMAND_TIC;
        } else if (command == TOC_STRING) {
            this->type = COMMAND_TOC;
        } else if (command == NOOP_STRING) {
            this->type = NO_OPERATION;
        } else {
            throw std::invalid_argument("Invalid command");
        }
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
        } else if (command.substr(0, splitPosition) == TWEET_STRING){
            this->type = COMMAND_ERROR;
            this->data = command.substr(splitPosition + 1);
        } else if (command.substr(0, splitPosition) == EXIT_STRING) {
            this->type = COMMAND_EXIT;
            this->data = command.substr(splitPosition + 1);
        } else if (command.substr(0, splitPosition) == NOTIFICATION_STRING) {
            this->type = COMMAND_NOTIFICATION;
            this->data = command.substr(splitPosition + 1);
        } else if (command.substr(0, splitPosition) == ALIVE_STRING) {
            this->type = COMMAND_ALIVE;
            this->data = command.substr(splitPosition + 1);
        } else if (command.substr(0, splitPosition) == BACKUP_STRING) {
            this->type = COMMAND_BACKUP;
            this->data = command.substr(splitPosition + 1);
        } else if (command.substr(0, splitPosition) == TIC_STRING) {
            this->type = COMMAND_TIC;
        } else if (command.substr(0, splitPosition) == TOC_STRING) {
            this->type = COMMAND_TOC;
        } else if (command.substr(0, splitPosition) == NOOP_STRING) {
            this->type = NO_OPERATION;
        } else if (command.substr(0, splitPosition) == ELECTION_STRING) {
            this->type = COMMAND_ELECTION;
            this->data = command.substr(splitPosition + 1);
        } else if (command.substr(0, splitPosition) == COORDINATOR_STRING) {
            this->type = COMMAND_COORDINATOR;
            this->data = command.substr(splitPosition + 1);
        } else if (command.substr(0, splitPosition) == ANSWER_STRING) {
            this->type = COMMAND_ANSWER;
        } else if (command.substr(0, splitPosition) == REPLICATE_STRING) {
            this->type = COMMAND_REPLICATE;
            this->data = command.substr(splitPosition + 1);
        } else if (command.substr(0, splitPosition) == REPLICATE_CONNECT_STRING) {
            this->type = COMMAND_REPLICATE_CONNECT;
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
        case COMMAND_EXIT:
            command = std::string(EXIT_STRING) + " " + this->data;
            break;
        case NO_OPERATION:
            command = std::string(NOOP_STRING);
            break;
        case COMMAND_NOTIFICATION:
            command = std::string(NOTIFICATION_STRING) + " " + this->data;
            break;
        case COMMAND_ALIVE:
            command = std::string(ALIVE_STRING) + " " + this->data;
            break;
        case COMMAND_BACKUP:
            command = std::string(BACKUP_STRING) + " " + this->data;
            break;
        case COMMAND_TIC:
            command = std::string(TIC_STRING);
            break;
        case COMMAND_TOC:
            command = std::string(TOC_STRING);
            break;
        case COMMAND_ELECTION:
            command = std::string(ELECTION_STRING) + " " + this->data;
            break;
        case COMMAND_COORDINATOR:
            command = std::string(COORDINATOR_STRING) + " " + this->data;
            break;
        case COMMAND_ANSWER:
            command = std::string(ANSWER_STRING);
            break;
        case COMMAND_REPLICATE:
            command = std::string(REPLICATE_STRING) + " " + this->data;
            break;
        case COMMAND_REPLICATE_CONNECT:
            command = std::string(REPLICATE_CONNECT_STRING) + " " + this->data;
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