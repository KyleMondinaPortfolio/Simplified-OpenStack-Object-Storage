#ifndef SCP_FUNCTIONS_H
#define SCP_FUNCTIONS_H

#include <string>
#include <cstdlib>

bool testConnection(const std::string &ipAddress);
bool createDir(const std::string &ipAddress, const std::string &directory);
bool transferObj(const std::string &src, const std::string &dest);

#endif /* SCP_FUNCTIONS_H*/