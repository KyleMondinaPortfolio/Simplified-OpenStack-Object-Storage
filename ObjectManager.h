#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include "MachineList.h"
#include "PartitionMap.h"
#include <set>

class FileObject {
public:
    std::string fileName;
    uint64_t partition;
    FileObject(std::string fileName, uint64_t partition): fileName(fileName), partition(partition) {}
};

class ObjectManager {
public:
    ObjectManager(const std::vector<std::string> &ipAddresses, uint64_t power);
    std::string downloadObj();
    std::string listUser();
    std::string uploadObj();
    std::string deleteObj();
    std::string addDisk();
    std::string removeDisk();
    MachineList machineList;
    PartitionMap partitionMap;
private:
    std::set<std::string> servers;
    std::map<std::string, std::vector<FileObject>> mainCopies;
    std::map<std::string, std::vector<FileObject>> backupCopies;
    int machineCount;
};

#endif /* OBJECT_MANAGER_H*/