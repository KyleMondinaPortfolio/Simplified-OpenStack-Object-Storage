#ifndef CLIENT_H
#define CLIENT_H

#include "networkHeaders.h"
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <libgen.h>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <array>
#include <algorithm>
#include <fstream>

class Client {
public:
    std::string userIp;
    int port;
    Client (const std::string &userIp, int port);
    void run();
private: 
    int sockfd;
    void downloadObj();
    void listUser(const std::string &command);
    void uploadObj(const std::string &command);
    void deleteObj();
    void addDisk();
    void removeDisk();
    void cleanDisks(const std::string &command);
    void displayHelp();
    std::string resolveHostname(const std::string &hostname);
};

#endif /* CLIENT_H */