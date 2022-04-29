#ifndef COMMAND_H
#define COMMAND_H

#include <string>

enum CommandType {
    COMMAND_CONNECT,
    COMMAND_FOLLOW,
    COMMAND_SEND,
    COMMAND_REDIRECT,
    COMMAND_ERROR,
    COMMAND_EXIT,
    COMMAND_NOTIFICATION,
    NO_OPERATION,
    COMMAND_ALIVE,
    COMMAND_BACKUP,
    COMMAND_TIC,
    COMMAND_TOC
};

class Command {
    CommandType type;
    std::string data;

    public:
        Command() {}
        Command(std::string command);
        Command(CommandType type, std::string data);
        operator std::string();
        CommandType getType();
        std::string getData();
};

#endif