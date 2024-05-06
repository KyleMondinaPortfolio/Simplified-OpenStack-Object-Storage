#ifndef SERVER_H
#define SERVER_H

#include "networkHeaders.h"
#include "ObjectManager.h"
#include <thread>
#include <mutex>
#include <vector>
#include <time.h>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <openssl/md5.h>
#include <iomanip>
#include <algorithm>


class Server {
public:
    int sockfd;
    int port;
    Server(const std::vector<std::string>& ipAddresses, int power);
    void acceptClients();
    void handleClient(int clientfd);
    ObjectManager objectManager;


private:
    struct sockaddr_in serverAddress;
    std::vector<std::string> ipAddresses;
    int power;
    std::mutex mtx;
    void downloadObj(int clientfd, const std::string &command);
    void listUser(int clientfd, const std::string &command);
    void uploadObj(int clientfd, const std::string &command);
    void deleteObj(int clientfd, const std::string &command);
    void addDisk(int clientfd, const std::string &command);
    void removeDisk(int clientfd, const std::string &command);
    void cleanDisks(int clientfd, const std::string &command);
};

int generateRandomPortNumber();
uint64_t hashAndMap(const std::string& str, int n);
std::string createDigitalSignature(const std::string& filename);
#endif /* SERVER_H */