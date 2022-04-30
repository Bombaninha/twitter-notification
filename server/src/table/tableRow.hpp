#ifndef MASTER_TABLE_H
#define MASTER_TABLE_H

#include <string>
#include <list>
#include <map>
#include <vector>

class TableRow {
    public:
        std::vector<std::pair<std::string, int>> activeSessions;
        bool notificationDelivered;
        std::list<std::string> followers;
        std::list<std::string> messagesToReceive;

    public:
        TableRow();
        ~TableRow();
        void startSession(std::string host, int port);
        void closeSession(std::string host, int port);
        int getActiveSessions();
        std::vector<std::pair<std::string, int>> sessions();
        bool getNotificationDelivered();
        void setNotificationDelivered(bool wasNotificationDelivered);
        std::list<std::string> getFollowers();
        bool hasFollower(std::string username);
        void addFollower(std::string username);
        bool hasNewNotification();
        void addNotification(std::string username, std::string message);
        std::string popNotification();
        std::string getNotification();
};

extern std::map<std::string, TableRow*> masterTable;

#endif