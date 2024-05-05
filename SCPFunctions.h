#ifndef SCP_FUNCTIONS_H
#define SCP_FUNCTIONS_H

#include <string>
#include <cstdlib>

bool testConnection(const std::string &ipAddress) {
    std::string command = "ssh -o StrictHostKeyChecking=no -o BatchMode=yes -o ConnectTimeout=5 " + ipAddress + " 'echo success'";
    int result = system(command.c_str());
    return result == 0;
}

bool createDir(const std::string &ipAddress, const std::string &directory) {
    std::string command = "ssh " + ipAddress + " 'mkdir " + directory + "  && chmod 777 " + directory + "'";
    
    int result = system(command.c_str());
    if (result == 0) {
        std::cout << ipAddress << ":" + directory + " successfully created with 777 permissions" << std::endl;
        return true;
    } else {
        std::cout <<  "Failed to create " << ipAddress << ":" + directory + " with 777 permissions" << std::endl;
        return false;
    }
}

bool transferObj(const std::string &src, const std::string &dest) {
    std::string command = "scp " + src + " " + dest;

    int result = system(command.c_str());
    if (result == 0) {
        std::cout << "File transfer from " << src << " to " << dest << " succeeded!" << std::endl;
        return true;
    } else {
        std::cout << "File transfer from " << src << " to " << dest << " failed!" << std::endl;
        return false;
    }
}

#endif /* SCP_FUNCTIONS_H*/