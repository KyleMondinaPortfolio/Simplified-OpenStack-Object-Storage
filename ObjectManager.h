#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include "MachineList.h"
#include "PartitionMap.h"
#include <set>

class FileObject {
public:
    std::string fileName;
    std::string user;
    std::string ds;
    uint64_t partition;
    FileObject(std::string fileName, std::string user, std::string ds, uint64_t partition): fileName(fileName), user(user), ds(ds), partition(partition) {}
    bool operator==(const FileObject& other) const {
        return fileName == other.fileName;
    }
};

class ObjectManager {
public:
    ObjectManager(const std::vector<std::string> &ipAddresses, uint64_t power);
    std::string downloadObj();
    std::string listUser();
    std::string uploadObj();
    std::string deleteObj();
    std::string addDisk(const std::string &ipAddress);
    std::string removeDisk(const std::string &ipAddress);
    MachineList machineList;
    PartitionMap partitionMap;
    std::set<std::string> servers;
    std::map<std::string, std::vector<FileObject>> mainCopies;
    std::map<std::string, std::vector<FileObject>> backupCopies;
    std::string printMapping(const std::map<std::string, std::vector<FileObject>> &mapping);
    int machineCount;
};

#endif /* OBJECT_MANAGER_H*/