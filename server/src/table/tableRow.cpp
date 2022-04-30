#include <iostream>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <vector>
#include <list>
#include "tableRow.hpp"

extern pthread_mutex_t readMutex;
extern pthread_mutex_t readAndWriteMutex;
extern int readers;
extern void sharedReaderLock();
extern void sharedReaderUnlock();

TableRow::TableRow() {
    this->activeSessions = {};
    this->notificationDelivered = false;
	this->followers = {};
	this->messagesToReceive = {};
}

TableRow::~TableRow() {
    //std::cout << "estou sendo destruido" << std::endl;
}

void TableRow::startSession(std::string host, int port) {
    pthread_mutex_lock(&readAndWriteMutex);
	this->activeSessions.push_back(std::make_pair(host, port));
	pthread_mutex_unlock(&readAndWriteMutex);
}

void TableRow::closeSession(std::string host, int port) {
	pthread_mutex_lock(&readAndWriteMutex);
	for (std::vector<std::pair<std::string, int>>::iterator it = this->activeSessions.begin(); it != this->activeSessions.end(); ++it) {
		if (it->first == host && it->second == port) {
			this->activeSessions.erase(it);
			break;
		}
	}
	pthread_mutex_unlock(&readAndWriteMutex);
}

int TableRow::getActiveSessions() {
	sharedReaderLock();
	int activeSessions = this->activeSessions.size();
	sharedReaderUnlock();

	return activeSessions;
}

std::vector<std::pair<std::string, int>> TableRow::sessions() {
	sharedReaderLock();
	std::vector<std::pair<std::string, int>> sessions = this->activeSessions;
	sharedReaderUnlock();

	return sessions;
}

bool TableRow::getNotificationDelivered() {
    sharedReaderLock();
	bool notificationDelivered = this->notificationDelivered;
	sharedReaderUnlock();
    
	return notificationDelivered;
}

void TableRow::setNotificationDelivered(bool wasNotificationDelivered) {
    pthread_mutex_lock(&readAndWriteMutex);
	this->notificationDelivered = wasNotificationDelivered;
	pthread_mutex_unlock(&readAndWriteMutex);
}

std::list<std::string> TableRow::getFollowers() {
	sharedReaderLock();
	std::list<std::string> followers = this->followers;
	sharedReaderUnlock();
	
    return followers;
}

bool TableRow::hasFollower(std::string username) {
	sharedReaderLock();
	std::list<std::string>::iterator followerIterator;
	followerIterator = std::find(this->followers.begin(), this->followers.end(), username);
	bool followerWasFound = followerIterator != this->followers.end();
	sharedReaderUnlock();

	return followerWasFound;
}

void TableRow::addFollower(std::string username) {
	pthread_mutex_lock(&readAndWriteMutex);
	this->followers.push_back(username);
	fflush(stdout);
	pthread_mutex_unlock(&readAndWriteMutex);
}

bool TableRow::hasNewNotification() {
	sharedReaderLock();
	bool hasNotifications = !this->messagesToReceive.empty();
	sharedReaderUnlock();
	
    return hasNotifications;
}

void TableRow::addNotification(std::string username, std::string message) {
	pthread_mutex_lock(&readAndWriteMutex);

	auto now = std::chrono::system_clock::now();
	//std::time_t now_time = std::chrono::system_clock::to_time_t(now);
	
	//std::string payload = std::string(std::ctime(&now_time)) + " @" + username + ": " + message;
	std::string payload = "@" + username + ": " + message;


	//std::cout << std::string(std::ctime(&now_time)) + " @" + username + ": " + message << std::endl;

	this->messagesToReceive.push_back(payload);

	pthread_mutex_unlock(&readAndWriteMutex);
}

std::string TableRow::popNotification() {
	sharedReaderLock();
	std::string notification = this->messagesToReceive.front();
	this->messagesToReceive.pop_front();
	sharedReaderUnlock();

	this->setNotificationDelivered(false);
	
    return notification;
}

std::string TableRow::getNotification() {
	sharedReaderLock();
	std::string notification = this->messagesToReceive.front();
	sharedReaderUnlock();
	
    return notification;
}