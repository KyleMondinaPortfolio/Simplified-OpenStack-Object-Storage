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
    void downloadObj();
    void listUser();
    void uploadObj(int clientfd, const std::string &command);
    void deleteObj();
    void addDisk();
    void removeDisk();
    void cleanDisks();
};

int generateRandomPortNumber();
uint64_t hashAndMap(const std::string& str, int n);

#endif /* SERVER_H */