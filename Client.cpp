#include "Client.h"
#include "SCPFunctions.h"

Client::Client(const std::string &userIp, int port) 
    : userIp(userIp), port(port) 
{

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		// Socket creation failed
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof serverAddress);
    serverAddress.sin_family = AF_INET;

    std::string inputAddress(userIp);
	std::string ipAddress;
	if (inputAddress[0] == 'l') {
		std::cout << inputAddress << std::endl;
		ipAddress = resolveHostname(inputAddress);
		std::cout << ipAddress << std::endl;
	} else {
		ipAddress = inputAddress;
	}

    // Convert IPv4 and IPv6 addresses from text to binary form
	if (inet_pton(AF_INET, ipAddress.c_str(), &serverAddress.sin_addr) <= 0) {
		std::cerr << "Invalid address/ Address not supported" << std::endl;
		exit(EXIT_FAILURE);
	}

    serverAddress.sin_port = htons(port);

	// Connect the client socket to server socket
	if (connect(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) != 0) {
		perror("Server connection failed");
		exit(EXIT_FAILURE);
	}

	std::cout << "Client successfully connected to the server" << std::endl;
}

void Client::run() {

	displayHelp();
	std::string command;
	do {
		std::cout << std::endl << "Type command to send to server. To see all available commands, type help" << std::endl;
		std::getline(std::cin, command);

		// Parse the command
		size_t space = command.find(' ');
		std::string firstWord;

		if (space != std::string::npos) {
			firstWord = command.substr(0, space);
		} else {
			firstWord = command;
		}

		if (firstWord == "download") {
			downloadObj();
		} else if (firstWord == "list") {
			listUser(command);
		} else if (firstWord == "upload") {
			uploadObj(command);
		} else if (firstWord == "delete") {
			deleteObj();
		} else if (firstWord == "add") {
			addDisk(command);
		} else if (firstWord == "remove") {
			removeDisk();
		} else if (firstWord == "clean") {
			cleanDisks(command);
		} else {
			displayHelp();
		}
	} while (command != "exit");
	
}

void Client::downloadObj() {
	std::string test = "download ";
	send(sockfd, test.c_str(), test.length(), 0);
}

void Client::listUser(const std::string &command) {
	send(sockfd, command.c_str(), command.length(), 0);

	char buffer[BUFFER_SIZE] = {0};
	int bytesReceived = recv(sockfd, buffer, BUFFER_SIZE, 0);
	if (bytesReceived < 0){
		std::cout << "Failed to get ack from server" << std::endl;
		return;
	}
	std::cout << buffer << std::endl;
}

void Client::uploadObj(const std::string &command) {
	// Parse the supplied file object
	size_t space = command.find(' ');
	std::string fileObject = command.substr(space+1);
	size_t slash = fileObject.find('/');
	std::string fileName = fileObject.substr(slash+1);

	// Verify if the supplied file object exists
	std::ifstream infile(fileName, std::ios::binary);
	if (!infile){
		std::cout << "Unable to open " << fileName << ". Make sure you input a valid file" << std::endl;
		return;
	}

	// Send the command to the server
	send(sockfd, command.c_str(), command.length(), 0);

	// Receive file size request from server
	char buffer[BUFFER_SIZE] = {0};
	int bytesReceived = recv(sockfd, buffer, BUFFER_SIZE, 0);
	if (bytesReceived < 0){
		std::cout << "Failed to get ack from server" << std::endl;
		return;
	}
	std::cout << buffer << std::endl;

	// Send file size to server
	infile.seekg(0, std::ios::end);
	size_t fileSize = infile.tellg();
	infile.seekg(0, std::ios::beg);
	send(sockfd, &fileSize, sizeof(fileSize), 0);
	std::cout << "File size: " << fileSize << std::endl;
	std::cout << "File size sent to server" << std::endl;

	// Receive file content request from the server
	memset(buffer, 0, sizeof(buffer)); // Clear the buffer before recieving messages
	bytesReceived = recv(sockfd, buffer, BUFFER_SIZE, 0);
	if (bytesReceived < 0){
		std::cout << "Failed to get ack from server" << std::endl;
		return;
	}
	std::cout << buffer << std::endl;

	// Send file contents to the server
	memset(buffer, 0, sizeof(buffer)); // Clear the buffer before recieving messages
	while (!infile.eof()) {
		infile.read(buffer, BUFFER_SIZE);
		int bytesRead = infile.gcount();
		send(sockfd, buffer, bytesRead, 0);
	}
	infile.close();

	// Receive Server ack
	memset(buffer, 0, sizeof(buffer)); // Clear the buffer before recieving messages
	bytesReceived = recv(sockfd, buffer, BUFFER_SIZE, 0);
	if (bytesReceived < 0){
		std::cout << "Failed to get ack from server" << std::endl;
		return;
	}
	std::cout << buffer << std::endl;
	
}

void Client::deleteObj() {
	std::string test = "delete ";
	send(sockfd, test.c_str(), test.length(), 0);
}

void Client::addDisk(const std::string &command) {
	size_t space = command.find(" ");
	std::string ipAddress = command.substr(space+1);

	bool validIp = testConnection(ipAddress);
	if (!validIp) {
		std::cout << "Cannot connect to " << ipAddress << std::endl;
		std::cout << "Provide a machine you can connect to " << std::endl;
		return;
	}

	send(sockfd, command.c_str(), command.length(), 0);

	char buffer[BUFFER_SIZE] = {0};
	int bytesReceived = recv(sockfd, buffer, BUFFER_SIZE, 0);
	if (bytesReceived < 0){
		std::cout << "Failed to get ack from server" << std::endl;
		return;
	}
	std::cout << buffer << std::endl;
}

void Client::removeDisk() {
	std::string test = "remove ";
	send(sockfd, test.c_str(), test.length(), 0);
}

void Client::cleanDisks(const std::string &command) {
	send(sockfd, command.c_str(), command.length(), 0);

	// Receive server acknowledgement
	char buffer[BUFFER_SIZE] = {0};
	int bytesReceived = recv(sockfd, buffer, BUFFER_SIZE, 0);
	if (bytesReceived < 0){
		std::cout << "Failed to get ack from server" << std::endl;
		return;
	}
	std::cout << buffer << std::endl;
}

void Client::displayHelp() {
	std::cout << std::endl << "Available Commands" << std::endl;
	std::cout << "- download <user/object> : display the context of <user/object>" << std::endl;
	std::cout << "- list <user> : display the <user>’s objects/files in “ls –lrt” format" << std::endl;
	std::cout << "- upload <user/object> : display which disks the <user/object> is saved" << std::endl;
	std::cout << "- delete <user/object> : display a confirmation or error message" << std::endl;
	std::cout << "- add <disk> : display new partitions with all <user/object> within" << std::endl;
	std::cout << "- remove <disk> : display where old partitions went to" << std::endl;
	std::cout << "- exit : end the session" << std::endl;
	return;
}

std::string Client::resolveHostname(const std::string &hostname) {
    std::string command = "host -t A " + hostname + " | awk '/has address/ { print $4 }'";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "popen failed!" << std::endl;
        return "";
    }

    char buffer[128];
    std::string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr) {
            result += buffer;
        }
    }
    pclose(pipe);

    // Remove newline characters from the result
    result.erase(std::remove_if(result.begin(), result.end(), [](char c) { return c == '\n' || c == '\r'; }), result.end());
    return result;
}

    
