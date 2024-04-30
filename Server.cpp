#include "Server.h"

Server::Server(const std::vector<std::string>& ipAddresses, int power)
    : ipAddresses(ipAddresses), power(power)
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        // Socket creation failed
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int yes = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt error");
		exit(EXIT_FAILURE);
	}

    std::cout << "Socket " << sockfd << " successfully created" << std::endl;

    memset(&serverAddress, 0, sizeof serverAddress);
    serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    // Generate a random available port
	port = generateRandomPortNumber();
	serverAddress.sin_port = htons(port);

    while ((bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress))) != 0 ) {
        std::cout << "Port " << port << " unavailable, generating a new port..." << std::endl;
		port = generateRandomPortNumber();
		serverAddress.sin_port = htons(port);
	}

    std::cout << "Socket " << sockfd << " successfully binded to port " << port << std::endl;

    // Grab IP Address
    FILE* pipe = popen("ifconfig eth0 | grep 'inet ' | awk '{print $2}'", "r");
    if (!pipe) {
        std::cerr << "Error: Failed to execute command." << std::endl;
    }
    char Ipbuffer[128];
    std::string Ipresult = "";
    while (!feof(pipe)) {
        if (fgets(Ipbuffer, 128, pipe) != NULL)
            Ipresult += Ipbuffer;
    }
    pclose(pipe);

    std::cout << "Server succesfully initialized! To run client, type ./Client " << Ipresult << " " << port << std::endl;

}

void Server::acceptClients() {
    // Listen to the binded socket
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listening error");
        exit(EXIT_FAILURE);
    }

    while(true) {
        struct sockaddr_storage clientAddress;
        socklen_t sin_size;

        int clientfd;
        sin_size = sizeof clientAddress;
		clientfd = accept(sockfd, (struct sockaddr *)&clientAddress, &sin_size); // Accept client connection
    
    	std::cout << "Server accepted connection from client " << clientfd << std::endl;

    	std::thread clientThread(&Server::handleClient, this, clientfd);
        clientThread.detach(); // Detach the thread to let it run independently
    }
}

void Server::handleClient(int clientfd) {
    char buffer[BUFFER_SIZE] = {0};
    while (true) {
        std::cout << "Server waiting for input from client " << clientfd << std::endl;

        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientfd, buffer, BUFFER_SIZE, 0);
        if (bytesReceived == 0) {
            std::cout << "Client " << clientfd << " ended their session" << std::endl;
            break;
        }

        // Parse client message
        std::string command(buffer);
        size_t space = command.find(" ");
        std::string firstWord;
        if (space != std::string::npos) {
            firstWord = command.substr(0, space);
        } else {
            firstWord = command;
        }

        if (firstWord == "download") {
            downloadObj();
            continue;
        } else if (firstWord == "list") {
            listUser();
            continue;
        } else if (firstWord == "upload") {
            uploadObj();
            continue;
        } else if (firstWord == "delete") {
            deleteObj();
            continue;
        } else if (firstWord == "add") {
            addDisk();
            continue;
        } else if (firstWord == "remove") {
            removeDisk();
            continue;
        } else if (firstWord == "clean") {
            cleanDisks();
            continue;
        } else {
            continue;
        }
    }
    close(clientfd);
}

void Server::downloadObj() {
    std::lock_guard<std::mutex> lock(mtx);

    std::cout << "Download command" << std::endl;
}

void Server::listUser() {
    std::lock_guard<std::mutex> lock(mtx);

    std::cout << "List command" << std::endl;
}

void Server::uploadObj() {
    std::lock_guard<std::mutex> lock(mtx);

    std::cout << "Upload command" << std::endl;
}

void Server::deleteObj() {
    std::lock_guard<std::mutex> lock(mtx);

    std::cout << "Delete command" << std::endl;
}

void Server::addDisk() {
    std::lock_guard<std::mutex> lock(mtx);

    std::cout << "Add command" << std::endl;
}

void Server::removeDisk() {
    std::lock_guard<std::mutex> lock(mtx);

    std::cout << "Remove command" << std::endl;
}

void Server::cleanDisks() {
    std::lock_guard<std::mutex> lock(mtx);

    std::cout << "Clean command" << std::endl;
}

int generateRandomPortNumber() {
    srand(time(NULL));
	return rand() % (65535 - 1024 + 1) + 1024;
}