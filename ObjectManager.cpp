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

    // Initialize the main copies and backup copies maps
    for (const auto& server : servers) {
        mainCopies[server] = std::vector<FileObject>();
        backupCopies[server] = std::vector<FileObject>();
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

std::string ObjectManager::addDisk(const std::string &ipAddress) {
    partitionMap.add(ipAddress);
    machineList.insert(ipAddress);
    machineCount += 1;
    servers.insert(ipAddress);

    std::string serverResponse;

    std::map<std::string, std::vector<FileObject>> newMainCopies;
    for (auto &pair: mainCopies) {
        newMainCopies[pair.first] = std::vector<FileObject>();
    }
    std::cout << "Main copy to server mappings before addition: " << std::endl;
    serverResponse += "Main copy to server mappings before addition: \n";
    serverResponse += printMapping(mainCopies);

    for (auto &pair: mainCopies) {
        std::cout << "in Machine " << pair.first << std::endl;
        newMainCopies[pair.first] = std::vector<FileObject>();
        for (auto &fileObj: pair.second) {
            std::cout << "looking at " << fileObj.fileName << std::endl;
            std::string fileName = fileObj.fileName;
            std::string user = fileObj.user;
            std::string correctMachine = partitionMap.lookup(fileObj.partition);
            if (pair.first != correctMachine) {
                // The file object needs to be transfered
                std::cout << fileName << " needs to be moved to " << correctMachine << std::endl;

                // Transfer the file
                createDir(correctMachine, "/tmp/kmondina");
                createDir(correctMachine, "/tmp/kmondina/" + user);
                transferObj(pair.first + ":/tmp/kmondina/" + user + "/" + fileName, correctMachine + ":/tmp/kmondina/" + user + "/" + fileName);
                transferObj(pair.first + ":/tmp/kmondina/" + user + "/." + fileName, correctMachine + ":/tmp/kmondina/" + user + "/" + fileName);

                newMainCopies[correctMachine].push_back(fileObj);

                std::string fileStatus = fileName + " transfered from " + pair.first + " to " + correctMachine + "\n";
                std::cout << fileStatus << std::endl;
                serverResponse += fileStatus;
            } else {
                newMainCopies[pair.first].push_back(fileObj);
            }
        }
    }
    mainCopies = newMainCopies;
    std::cout << "Main copy to server mappings after addition: " << std::endl;
    serverResponse += "Main copy to server mappings after addition: \n";
    auto m_it = mainCopies.find(ipAddress);
    if (m_it == mainCopies.end()) {
        mainCopies[ipAddress] = std::vector<FileObject>();
    } 
    serverResponse += printMapping(mainCopies);


    std::map<std::string, std::vector<FileObject>> newBackupCopies;
    for (auto &pair: backupCopies) {
        newBackupCopies[pair.first] = std::vector<FileObject>();
    }
    std::cout << "Backup copy to server mappings before addition: " << std::endl;
    serverResponse += "Backup copy to server mappings before addition: \n";
    serverResponse += printMapping(backupCopies);

    for (auto &pair: backupCopies) {
        std::cout << "in Machine " << pair.first << std::endl;

        for (auto &fileObj: pair.second) {
            std::cout << "looking at " << fileObj.fileName << std::endl;
            std::string fileName = fileObj.fileName;
            std::string user = fileObj.user;
            std::string mainMachine = partitionMap.lookup(fileObj.partition);
            std::string correctMachine = machineList.find(mainMachine)->next->ipAddress; // Find the ipAddress of the machine the back up copy will be stored
            if (pair.first != correctMachine) {
                // The file object needs to be transfered
                std::cout << fileName << " needs to be moved to " << correctMachine << std::endl;

                // Transfer the file
                createDir(correctMachine, "/tmp/kmondina");
                createDir(correctMachine, "/tmp/kmondina/" + user);
                transferObj(pair.first + ":/tmp/kmondina/" + user + "/" + fileName, correctMachine + ":/tmp/kmondina/" + user + "/" + fileName);
                transferObj(pair.first + ":/tmp/kmondina/" + user + "/." + fileName, correctMachine + ":/tmp/kmondina/" + user + "/" + fileName);
                std::string filePath = "/tmp/kmondina/" + user + "/" + fileName;
                std::string fileMD = "/tmp/kmondina/" + user + "/." + fileName;
                sshDeleteObj(pair.first, filePath);
                sshDeleteObj(pair.first, fileMD);

                newBackupCopies[correctMachine].push_back(fileObj);

                std::string fileStatus = fileName + " transfered from " + pair.first + " to " + correctMachine + "\n";
                std::cout << fileStatus << std::endl;
                serverResponse += fileStatus;
            }
            else {
                newBackupCopies[pair.first].push_back(fileObj);
            }
        }
    }
    backupCopies = newBackupCopies;
    std::cout << "Backup copy to server mappings after addition: " << std::endl;
    serverResponse += "Backup copy to server mappings after addition: \n";
    auto b_it = backupCopies.find(ipAddress);
    if (b_it == backupCopies.end()) {
        backupCopies[ipAddress] = std::vector<FileObject>();
    } 
    serverResponse += printMapping(backupCopies);
    serverResponse += "New Partition Map After disk addition:\n";
    serverResponse += partitionMap.display();
    return serverResponse;
}

std::string ObjectManager::removeDisk(const std::string &ipAddress) {
    // Display the state of the mappings before the operation
    std::string serverResponse;
    std::cout << "Main copy to server mappings before deletion: " << std::endl;
    serverResponse += "Main copy to server mappings before deletion: \n";
    serverResponse += printMapping(mainCopies);
    std::cout << "Backup copy to server mappings before deletion: " << std::endl;
    serverResponse += "Main copy to server mappings before deletion: \n";
    serverResponse += printMapping(backupCopies);

    // Save the now displaced copies
    std::vector<FileObject> mainCopiesToMove = mainCopies[ipAddress];
    std::vector<FileObject> backupCopiesToMove = backupCopies[ipAddress];

    // Save the remaining copies in the remaining servers
    std::map<std::string, std::vector<FileObject>> remainingMainCopies;
    std::map<std::string, std::vector<FileObject>> remainingBackupCopies;

    for (const auto &pair: mainCopies) {
        if (pair.first != ipAddress) {
            remainingMainCopies[pair.first] = pair.second;
        }
    }

    for (const auto &pair: backupCopies) {
        if (pair.first != ipAddress) {
            remainingBackupCopies[pair.first] = pair.second;
        }
    }

    mainCopies = remainingMainCopies;
    backupCopies = remainingBackupCopies;

    // Perform update of the parition mappings
    partitionMap.remove(ipAddress);
    machineList.remove(ipAddress);
    machineCount -= 1;
    servers.erase(ipAddress);

    // Update the remaining main copies
    std::cout << "[debug]: ---------transferring remaining main copies start---------" << std::endl;
    std::map<std::string, std::vector<FileObject>> newMainCopies;
    // Properly initialize
    for (auto &pair: mainCopies) {
        newMainCopies[pair.first] = std::vector<FileObject>();
    }
    for (auto &pair: mainCopies) {
        std::cout << "in Machine " << pair.first << std::endl;
        for (auto &fileObj: pair.second) {
            std::cout << "looking at " << fileObj.fileName << std::endl;
            std::string fileName = fileObj.fileName;
            std::string user = fileObj.user;
            std::string correctMachine = partitionMap.lookup(fileObj.partition);
            if (pair.first != correctMachine) {
                // The file object needs to be transfered
                std::cout << fileName << " needs to be moved to " << correctMachine << std::endl;

                // Transfer the file
                createDir(correctMachine, "/tmp/kmondina");
                createDir(correctMachine, "/tmp/kmondina/" + user);
                transferObj(pair.first + ":/tmp/kmondina/" + user + "/" + fileName, correctMachine + ":/tmp/kmondina/" + user + "/" + fileName);
                transferObj(pair.first + ":/tmp/kmondina/" + user + "/." + fileName, correctMachine + ":/tmp/kmondina/" + user + "/" + fileName);

                newMainCopies[correctMachine].push_back(fileObj);

                std::string fileStatus = fileName + " transfered from " + pair.first + " to " + correctMachine + "\n";
                std::cout << fileStatus << std::endl;
                serverResponse += fileStatus;
            } else {
                newMainCopies[pair.first].push_back(fileObj);
            }
        }
    }
    std::cout << "[debug]: ---------transferring remaining main copies end---------" << std::endl;
    mainCopies = newMainCopies;

    // Update the remaining backup copies
    std::cout << "[debug]: ---------transferring remaining backup copies start---------" << std::endl;
    std::map<std::string, std::vector<FileObject>> newBackupCopies;
    // Properly initialize
    for (auto &pair: backupCopies) {
        newBackupCopies[pair.first] = std::vector<FileObject>();
    }
    for (auto &pair: backupCopies) {
        std::cout << "in Machine " << pair.first << std::endl;

        for (auto &fileObj: pair.second) {
            std::cout << "looking at " << fileObj.fileName << std::endl;
            std::string fileName = fileObj.fileName;
            std::string user = fileObj.user;
            std::string mainMachine = partitionMap.lookup(fileObj.partition);
            std::string correctMachine = machineList.find(mainMachine)->next->ipAddress; // Find the ipAddress of the machine the back up copy will be stored
            if (pair.first != correctMachine) {
                // The file object needs to be transfered
                std::cout << fileName << " needs to be moved to " << correctMachine << std::endl;

                // Transfer the file
                createDir(correctMachine, "/tmp/kmondina");
                createDir(correctMachine, "/tmp/kmondina/" + user);
                transferObj(pair.first + ":/tmp/kmondina/" + user + "/" + fileName, correctMachine + ":/tmp/kmondina/" + user + "/" + fileName);
                transferObj(pair.first + ":/tmp/kmondina/" + user + "/." + fileName, correctMachine + ":/tmp/kmondina/" + user + "/" + fileName);
                std::string filePath = "/tmp/kmondina/" + user + "/" + fileName;
                std::string fileMD = "/tmp/kmondina/" + user + "/." + fileName;
                sshDeleteObj(pair.first, filePath);
                sshDeleteObj(pair.first, fileMD);

                newBackupCopies[correctMachine].push_back(fileObj);

                std::string fileStatus = fileName + " transfered from " + pair.first + " to " + correctMachine + "\n";
                std::cout << fileStatus << std::endl;
                serverResponse += fileStatus;
            }
            else {
                newBackupCopies[pair.first].push_back(fileObj);
            }
        }
    }
    std::cout << "[debug] newBackupCopies" << std::endl;
    printMapping(newBackupCopies);
    std::cout << "[debug]: ---------transferring remaining backup copies end---------" << std::endl;
    backupCopies = newBackupCopies;

    // Loop through the copies to move
    std::cout << "[debug]: ---------transferring main copies to move start---------" << std::endl;
    for (const auto &fileObj: mainCopiesToMove) {
        std::string fileName = fileObj.fileName;
        std::string user = fileObj.user;
        std::string correctMachine = partitionMap.lookup(fileObj.partition);

        createDir(correctMachine, "/tmp/kmondina");
        createDir(correctMachine, "/tmp/kmondina/" + user);
        transferObj(ipAddress + ":/tmp/kmondina/" + user + "/" + fileName, correctMachine + ":/tmp/kmondina/" + user + "/" + fileName);
        transferObj(ipAddress + ":/tmp/kmondina/" + user + "/." + fileName, correctMachine + ":/tmp/kmondina/" + user + "/" + fileName);

        mainCopies[correctMachine].push_back(fileObj);
        std::string fileStatus = fileName + " transfered from " + ipAddress + " to " + correctMachine + "\n";
        std::cout << fileStatus << std::endl;
        serverResponse += fileStatus;
    }
    std::cout << "[debug]: ---------transferring main copies to move end---------" << std::endl;

    std::cout << "[debug]: ---------transferring backup copies to move start---------" << std::endl;
    for (const auto &fileObj: backupCopiesToMove) {
        std::string fileName = fileObj.fileName;
        std::string user = fileObj.user;
        std::string mainMachine = partitionMap.lookup(fileObj.partition);
        std::string correctMachine = machineList.find(mainMachine)->next->ipAddress; // Find the ipAddress of the machine the back up copy will be stored

        createDir(correctMachine, "/tmp/kmondina");
        createDir(correctMachine, "/tmp/kmondina/" + user);
        transferObj(ipAddress + ":/tmp/kmondina/" + user + "/" + fileName, correctMachine + ":/tmp/kmondina/" + user + "/" + fileName);
        transferObj(ipAddress + ":/tmp/kmondina/" + user + "/." + fileName, correctMachine + ":/tmp/kmondina/" + user + "/" + fileName);

        backupCopies[correctMachine].push_back(fileObj);
        std::string fileStatus = fileName + " transfered from " + ipAddress + " to " + correctMachine + "\n";
        std::cout << fileStatus << std::endl;
        serverResponse += fileStatus;
    }
    std::cout << "[debug]: ---------transferring backup copies to move end---------" << std::endl;


    std::cout << "Main copy to server mappings after deletion: " << std::endl;
    serverResponse += "Main copy to server mappings after deletion: \n";
    serverResponse += printMapping(mainCopies);
    std::cout << "Backup copy to server mappings after deletion: " << std::endl;
    serverResponse += "Backup copy to server mappings after deletion: \n";
    serverResponse += printMapping(backupCopies);

    deleteDir(ipAddress, "/tmp/kmondina");
    serverResponse += "New Partition Map After disk deletion:\n";
    serverResponse += partitionMap.display();
    return serverResponse;
}

std::string ObjectManager::printMapping(const std::map<std::string, std::vector<FileObject>> &mapping) {
    std::string display;
    for (const auto& pair : mapping) {
        const std::string& key = pair.first;  // Key of the map
        const std::vector<FileObject>& files = pair.second;  // Corresponding vector of FileObjects
        // Print the key (string)
        std::cout << "Machine: " << key << std::endl;
        display += "Machine: " + key + "\n";
        // Iterate over the vector of FileObjects
        for (const auto& file : files) {
            // Print each fileName
            std::cout << file.fileName << std::endl;
            display += file.fileName + "\n";
        }
    }
    return display;
}


