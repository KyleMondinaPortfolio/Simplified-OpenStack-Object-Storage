#include "ObjectManager.h"
#include "SCPFunctions.h"
#include "PartitionMap.h"
#include <cmath>

ObjectManager::ObjectManager(const std::vector<std::string> &ipAddresses, uint64_t power) 
    : machineList(ipAddresses), 
      partitionMap(ipAddresses, Range(0,std::pow(2,power))), 
      machineCount(ipAddresses.size()),
      servers(ipAddresses.begin(),ipAddresses.end())
{
    // Create staging directory
    std::string command = "mkdir /tmp/stagingDir ; chmod 777 /tmp/stagingDir";
    system(command.c_str());

    // /tmp/kmondina on all of the servers
    for (const auto &server: servers) {
        createDir(server, "/tmp/kmondina");
    }
}

std::string ObjectManager::downloadObj() {
    std:: cout << "downloadObj() function" << std::endl;
    std::string test = "test";
    return test;
}

std::string ObjectManager::listUser() {
    std:: cout << "listUser() function" << std::endl;
    std::string test = "test";
    return test;
}

std::string ObjectManager::uploadObj() {
    std:: cout << "uploadObj() function" << std::endl;
    std::string test = "test";
    return test;
}

std::string ObjectManager::deleteObj() {
    std:: cout << "deleteObj() function" << std::endl;
    std::string test = "test";
    return test;
}

std::string ObjectManager::addDisk() {
    std:: cout << "addDisk() function" << std::endl;
    std::string test = "test";
    return test;
}

std::string ObjectManager::removeDisk() {
    std:: cout << "removeDisk() function" << std::endl;
    std::string test = "test";
    return test;
}

