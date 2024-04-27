#include <iostream>
#include "MachineList.h"

void testMachineList() {
    std::vector<std::string> ipAddresses = {"192.168.1.1", "192.168.1.2", "192.168.1.3"};
    MachineList machineList(ipAddresses);

    std::cout << "Initial Machine List: ";
    machineList.display();

    machineList.insert("192.168.1.4");
    std::cout << "After inserting 192.168.1.4: ";
    machineList.display();

    machineList.remove("192.168.1.2");
    std::cout << "After removing 192.168.1.2: ";
    machineList.display();

    std::cout << "Next Machine after 192.168.1.1 is : " <<  machineList.find("192.168.1.1")->next->ipAddress << std::endl;
    std::cout << "Next Machine after 192.168.1.4 is : " <<  machineList.find("192.168.1.4")->next->ipAddress << std::endl;
}

int main() {

    //testMachineList();
    return 0;
}