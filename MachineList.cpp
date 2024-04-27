#include "MachineList.h"

// Constructor for MachineList class
// Creates a MachineList with the provided list of IP addresses
// ipAddresses: vector containing IP addresses of machines
MachineList::MachineList(const std::vector<std::string> &ipAddresses) {
    head = nullptr;
    count = ipAddresses.size();

    if (count < 2) {
        std::cout << "Cannot create MachineList! You need at least two machines to create a MachineList" << std::endl;
        return;
    }

    for (const auto &ipAddress : ipAddresses) {
        insert(ipAddress);
    }
}

// Insert a machine with the given IP address into the MachineList
// ipAddress: IP address of the machine to be inserted
void MachineList::insert(const std::string &ipAddress) {
    Machine *newMachine = new Machine(ipAddress);
    if (!head) {
        head = newMachine;
        head->next = head;
        head->prev = head;
    } else {
        Machine *last = head->prev;
        last->next = newMachine;
        newMachine->prev = last;
        newMachine->next = head;
        head->prev = newMachine;
    }
    count+=1;
}

// Remove a machine with the given IP address from the MachineList
// ipAddress: IP address of the machine to be removed
void MachineList::remove(const std::string &ipAddress) {
    if (count == 2) {
        std::cout << "Cannot remove machine with IP Address: " << ipAddress << " from MachineList. MachineList needs at least two machines" << std::endl;
    }
    if (!head) {
        return;
    }
        
    Machine* current = head;
    do {
        if (current->ipAddress == ipAddress) {
            if (current == head) {
                head = head->next;
            }
            current->prev->next = current->next;
            current->next->prev = current->prev;
            delete current;
            return;
        }
        current = current->next;
    } while (current != head);
    count -= 1;
}

// Find a machine with the given IP address in the MachineList
// ipAddress: IP address of the machine to be found
// Returns a pointer to the machine if found, otherwise nullptr
Machine* MachineList::find(const std::string &ipAddress) {
    if (!head) {
        std::cout << "Machine with IP Address: " << ipAddress << " not found." << std::endl;
        return nullptr;
    }

    Machine* current = head;
    do {
        if (current->ipAddress == ipAddress) {
            return current;
        }
        current = current->next;
    } while (current != head);

    std::cout << "Machine with IP Address: " << ipAddress << " not found." << std::endl;
    return nullptr;   
}

// Display the IP addresses of all machines in the MachineList
void MachineList::display() {
    if (!head) {
        std::cout << "Error: uninitialized MachineList" << std::endl;
        return;
    }

    Machine* current = head;
    std::cout << std::endl;
    do {
        std::cout << current->ipAddress << std::endl;
        current = current->next;
    } while (current != head);
    std::cout << std::endl;
}