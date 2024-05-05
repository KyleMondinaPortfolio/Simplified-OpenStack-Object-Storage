#ifndef SCP_FUNCTIONS_H
#define SCP_FUNCTIONS_H

#include <string>
#include <cstdlib>
#include <cstdio> 

bool testConnection(const std::string &ipAddress);
bool createDir(const std::string &ipAddress, const std::string &directory);
bool deleteDir(const std::string &ipAddress, const std::string &directory);
bool sshDeleteObj(const std::string &ipAddress, const std::string &filePath);
bool transferObj(const std::string &src, const std::string &dest);
std::string listDirectory(const std::string& ipAddress, const std::string& user);

#endif /* SCP_FUNCTIONS_H*/