#ifndef SERVER_H
#define SERVER_H

#include "networkHeaders.h"
#include <thread>
#include <mutex>
#include <vector>
#include <time.h>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cmath>
#include <iostream>

class Server {
public:
    int sockfd;
    int port;
    Server(const std::vector<std::string>& ipAddresses, int power);
    void acceptClients();
    void handleClient(int clientfd);

private:
    struct sockaddr_in serverAddress;
    std::vector<std::string> ipAddresses;
    int power;
    std::mutex mtx;
    void downloadObj();
    void listUser();
    void uploadObj();
    void deleteObj();
    void addDisk();
    void removeDisk();
    void cleanDisks();
};

int generateRandomPortNumber();

#endif /* SERVER_H */