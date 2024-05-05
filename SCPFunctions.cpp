#include "SCPFunctions.h"
#include <iostream> // Include for std::cout and std::endl

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

bool deleteDir(const std::string &ipAddress, const std::string &directory) {
    std::string command = "ssh " + ipAddress + " 'rm -r " + directory + "'";
    
    int result = system(command.c_str());
    if (result == 0) {
        std::cout << ipAddress << ":" + directory + " successfully deleted" << std::endl;
        return true;
    } else {
        std::cout <<  "Failed to delete " << ipAddress << ":" + directory  << std::endl;
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

std::string listDirectory(const std::string& ipAddress, const std::string& user) {
    // SSH command
    std::string command = "ssh " + ipAddress + " 'ls -lrt /tmp/kmondina/" + user + "'";

    // Open a pipe to the command
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Error: popen failed!" << std::endl;
        return "";
    }

    // Read the output into a string
    std::string result;
    char buffer[128];
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr)
            result += buffer;
    }

    // Close the pipe
    pclose(pipe);
    // Remove the header e.g : "total: count"
    size_t dash = result.find("-");
    result = result.substr(dash);
    return result + "\n";
}