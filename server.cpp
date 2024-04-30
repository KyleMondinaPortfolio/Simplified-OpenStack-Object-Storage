#include "Server.h"
#include "Client.h"
#include <thread>

int main(int argc, char *argv[]) {
    // Check if there are at least two arguments (power and at least one IP address)
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <power> <ip_address1> [<ip_address2> ...]" << std::endl;
        return 1;
    }

    int power = std::stoi(argv[1]);

    std::vector<std::string> ipAddresses;
	// Iterate over the command line arguments starting from the second one
    for (int i = 2; i < argc; ++i) {
        // Add each argument (IP address) to the vector
        ipAddresses.push_back(argv[i]);
    }

    Server server(ipAddresses, power);
    std::thread serverThread([&](){
        server.acceptClients();
    });

    Client client ("127.0.0.1", server.port);
    std::thread clientThread([&](){
        std::cout << "test" << std::endl;
        client.run();
    });

    serverThread.join();
    clientThread.join();

    return 0;
}