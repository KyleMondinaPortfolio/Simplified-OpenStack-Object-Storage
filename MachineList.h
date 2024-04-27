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