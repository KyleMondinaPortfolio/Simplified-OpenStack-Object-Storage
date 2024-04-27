#ifndef MACHINE_LIST_H
#define MACHINE_LIST_H

#include <iostream>
#include <string>
#include <vector>

class Machine {
public:
    std::string ipAddress;
    Machine *next;
    Machine *prev;

    Machine(const std::string &ipAddress) : ipAddress(ipAddress), next(nullptr), prev(nullptr) {}
};

// Circular Linked List object representing a network of machines. 
// When a file is uploaded to a machine, its copy should be uploaded to the next machine in this circular list. 
// The machine list must contain a minimum of two machines, as we require at least two machines for storing the original and backup files.
class MachineList {
private:
    Machine *head;
    int count;

public:
    MachineList(const std::vector<std::string> &ipAddresses);
    void insert(const std::string &ipAddress);
    void remove(const std::string &ipAddress);
    Machine* find(const std::string &ipAddress);
    void display();
};

#endif /* MACHINE_LIST_H */