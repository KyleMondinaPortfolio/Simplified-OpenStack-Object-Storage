#include "Client.h"

int main(int argc, char *argv[]) {
	// Check for correct usage of the command
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <ipAddress> <portNumber>" << std::endl;
		exit(EXIT_FAILURE);
	}
	int port = atoi(argv[2]);
    Client client(argv[1], port);
    client.run();
	return 0;
}
