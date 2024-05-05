#include "Server.h"
#include "MachineList.h"
#include "SCPFunctions.h"

Server::Server(const std::vector<std::string>& ipAddresses, int power)
    : ipAddresses(ipAddresses), power(power), objectManager(ipAddresses, power)
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

    objectManager.machineList.display();

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
            uploadObj(clientfd, command);
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

void Server::uploadObj(int clientfd, const std::string &command) {
    std::lock_guard<std::mutex> lock(mtx);
	char buffer[BUFFER_SIZE] = {0};

    // Parse the command
    size_t space = command.find(' ');
	std::string fileObject = command.substr(space+1);
	size_t slash = fileObject.find('/');
	std::string fileName = fileObject.substr(slash+1);
    std::string user = fileObject.substr(0,slash);

    // Request file size from the client
    std::string requestFileSize = "Server: request file size";
    send(clientfd, requestFileSize.c_str(), requestFileSize.size(), 0);

    // Receive file size from the server
    size_t fileSize;
	int bytesReceived = recv(clientfd, &fileSize, sizeof(fileSize), 0);
	if (bytesReceived != sizeof(fileSize)) {
		std::cerr << "Failed to receive file size from client" << std::endl;
		return;
	}

    // Request file contents from the client
    std::string requestFileContent = "Server: received file size, requesting file content";
    send(clientfd, requestFileContent.c_str(), requestFileContent.size(), 0);

    // Receive the file from the client
    std::string localFile = "/tmp/stagingDir/" + fileName;
    std::ofstream outfile(localFile.c_str(), std::ios::binary);
    memset(buffer, 0, sizeof(buffer)); // Clear the buffer before recieving messages
	size_t totalReceived = 0;
    while (totalReceived < fileSize) {
        bytesReceived = recv(clientfd, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) {
            std::cerr << "Failed to receive file data from client" << std::endl;
            continue;
        }
        outfile.write(buffer, bytesReceived);
        totalReceived += bytesReceived;
    }
	// Close the file!!! Very important!!!
	outfile.close();

    
    // Create the digital signature of the file and write it to the meta data file
    std::string ds = createDigitalSignature(localFile); 
    std::string localFileMD = "/tmp/stagingDir/." + fileName;
    std::ofstream fileMD(localFileMD);
    fileMD << ds;
    fileMD.close();

    uint64_t partition = hashAndMap(fileName, power); // Find which partition the file is hashed to 
    std::string mainMachine = objectManager.partitionMap.lookup(partition); // Find the ipAddress of the machine the main copy will be store
    std::string backupMachine = objectManager.machineList.find(mainMachine)->next->ipAddress; // Find the ipAddress of the machine the back up copy will be stored
    std::cout << "Main copy of " << fileName << " should be stored in Machine with ipAddress " << mainMachine << std::endl;
    std::cout << "Backup copy of " << fileName << " should be stored in Machine with ipAddress " << backupMachine << std::endl;
    FileObject fileObj = FileObject(fileName, user, ds, partition);

    // Update the map for main copies and backup copies
    objectManager.mainCopies[mainMachine].push_back(fileObj);
    objectManager.backupCopies[backupMachine].push_back(fileObj);

    // Transfer the file and its metadata to the appropriate machines
    createDir(mainMachine, "/tmp/" + user);
    createDir(backupMachine, "/tmp/" + user);
    transferObj(localFile, mainMachine + ":/tmp/" + user);
    transferObj(localFileMD, mainMachine + ":/tmp/" + user);
    transferObj(localFile, backupMachine + ":/tmp/" + user);
    transferObj(localFileMD, backupMachine + ":/tmp/" + user);

    std::string listDir = "ls /tmp/stagingDir";
    system(listDir.c_str());
    return;
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

uint64_t hashAndMap(const std::string& str, int n) {
    // Hash the string using MD5
    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5((const unsigned char*)str.c_str(), str.length(), hash);
    
    // Convert the hash to a uint64_t
    uint64_t hashValue = 0;
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        hashValue |= (uint64_t)hash[i] << (8 * i);
    }
    
    // Map the hash value to a range within 2^n
    uint64_t range = pow(2, n);
    return hashValue % range;
}

std::string createDigitalSignature(const std::string& filename) {
    MD5_CTX md5Context;
    unsigned char digest[MD5_DIGEST_LENGTH];
    char buffer[4096];

    MD5_Init(&md5Context);

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return "";
    }

    while (file.read(buffer, sizeof(buffer))) {
        MD5_Update(&md5Context, buffer, file.gcount());
    }

    MD5_Final(digest, &md5Context);

    std::stringstream ss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }

    return ss.str();
}
