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
            downloadObj(clientfd, command);
            continue;
        } else if (firstWord == "list") {
            listUser(clientfd, command);
            continue;
        } else if (firstWord == "upload") {
            uploadObj(clientfd, command);
            continue;
        } else if (firstWord == "delete") {
            deleteObj(clientfd, command);
            continue;
        } else if (firstWord == "add") {
            addDisk(clientfd, command);
            continue;
        } else if (firstWord == "remove") {
            removeDisk(clientfd, command);
            continue;
        } else if (firstWord == "clean") {
            cleanDisks(clientfd, command);
            continue;
        } else {
            continue;
        }
    }
    close(clientfd);
}

void Server::downloadObj(int clientfd, const std::string &command) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Received download command from client " << clientfd << std::endl;

    // Parse the file name
    size_t space = command.find(' ');
	std::string fileObject = command.substr(space+1);
	size_t slash = fileObject.find('/');
	std::string fileName = fileObject.substr(slash+1);
    std::string user = fileObject.substr(0,slash);

    // Grab the ipAddress of the main copy
    uint64_t partition = hashAndMap(fileName, power);
    std::string mainMachine = objectManager.partitionMap.lookup(partition);
    std::string backupMachine = objectManager.machineList.find(mainMachine)->next->ipAddress;

    // Grab the file object, if present
    std::string fileDS;
    bool objectFound = false;
    for (const auto &fileObj: objectManager.mainCopies[mainMachine]) {
        if (fileObj.fileName == fileName && fileObj.user == user) {
            objectFound = true;
            fileDS = fileObj.ds;
        }
    }

    std::string serverResponse;
    if (!objectFound) {
        serverResponse = "Server: requested object not found";
        std::cout << serverResponse << std::endl;
        send(clientfd, serverResponse.c_str(), serverResponse.length(), 0);
        return;
    }

    // Download the main copy to the staging directory 
    std::string localFile = "/tmp/stagingDir/" + fileName;
    // Set the file intially blank, scp will overwrite it if it is 
    std::ofstream localMainFile(localFile);
    localMainFile.close();
    transferObj(mainMachine + ":/tmp/kmondina/" + fileObject, "/tmp/stagingDir/.");
    std::string localMainDS = createDigitalSignature(localFile);
    std::cout << "main original ds " << fileDS << std::endl;
    std::cout << "main file ds " << localMainDS << std::endl;

    if (localMainDS != fileDS) {
        std::cout << "Main copy corrupted!" << std::endl;
        std::cout << "Restore main file using backup from " << backupMachine << std::endl;
        // Grab the file from and back up and copy it to localMainFile
        transferObj(backupMachine + ":/tmp/kmondina/" + fileObject, mainMachine + ":/tmp/kmondina/" + fileObject);

        // Perform check again
        std::ofstream localMainFile(localFile);
        localMainFile.close();
        std::cout << "Checking the main copy ds after restoration:";
        transferObj(mainMachine + ":/tmp/kmondina/" + fileObject, "/tmp/stagingDir/.");
        std::string localMainDS = createDigitalSignature(localFile);
        std::cout << "main original ds " << fileDS << std::endl;
        std::cout << "main file ds " << localMainDS << std::endl;
    } else {
        std::cout << "Main copy uncorrupted!" << std::endl;
    }

    std::ofstream localCopyFile(localFile);
    localCopyFile.close();
    transferObj(backupMachine + ":/tmp/kmondina/" + fileObject, "/tmp/stagingDir/.");
    std::string localCopyDS = createDigitalSignature(localFile);
    std::cout << "copy original ds " << fileDS << std::endl;
    std::cout << "copy file ds " << localCopyDS << std::endl;
    if (localCopyDS != fileDS) {
        std::cout << "Backup copy corrupted!" << std::endl;
        std::cout << "Restore backup file using main from " << mainMachine << std::endl;
        // Grab the file from and back up and copy it to localMainFile
        transferObj(mainMachine + ":/tmp/kmondina/" + fileObject, backupMachine + ":/tmp/kmondina/" + fileObject);

        // Perform check again
        std::ofstream localCopyFile(localFile);
        localCopyFile.close();
        std::cout << "Checking the backup copy ds after restoration:";
        transferObj(backupMachine + ":/tmp/kmondina/" + fileObject, "/tmp/stagingDir/.");
        std::string localCopyDS = createDigitalSignature(localFile);
        std::cout << "main original ds " << fileDS << std::endl;
        std::cout << "main file ds " << localCopyDS << std::endl;
    } else {
        std::cout << "Backup copy uncorrupted!" << std::endl;
    }

    std::string fileStatus = "Server: requested object found\n";
    send(clientfd, fileStatus.c_str(), fileStatus.length(), 0);
    char buffer[BUFFER_SIZE] = {0};
    memset(buffer, 0, sizeof(buffer)); // Clear the buffer before recieving messages
	int bytesReceived = recv(clientfd, buffer, BUFFER_SIZE, 0);
	if (bytesReceived < 0){
		std::cout << "Failed to get ack from server" << std::endl;
		return;
	}
	std::cout << buffer << std::endl;

    // File ready to be sent to the client
    // Send the file size!
    std::ifstream infile(localFile, std::ios::binary);
    infile.seekg(0, std::ios::end);
	size_t fileSize = infile.tellg();
	infile.seekg(0, std::ios::beg);
	send(clientfd, &fileSize, sizeof(fileSize), 0);

    // Receive client ack
    memset(buffer, 0, sizeof(buffer)); // Clear the buffer before recieving messages
	bytesReceived = recv(clientfd, buffer, BUFFER_SIZE, 0);
	if (bytesReceived < 0){
		std::cout << "Failed to get ack from server" << std::endl;
		return;
	}
	std::cout << buffer << std::endl;

    // Send to client file
    memset(buffer, 0, sizeof(buffer)); // Clear the buffer before recieving messages
	while (!infile.eof()) {
		infile.read(buffer, BUFFER_SIZE);
		int bytesRead = infile.gcount();
		send(clientfd, buffer, bytesRead, 0);
	}
	infile.close();

    // Clean up the saved file from local machine
    std::string cleanUpFile = "rm " + localFile;
    system(cleanUpFile.c_str());

    std::cout << "Succesfully sent file " << fileObject << " to client" << std::endl;
    return;
}

void Server::listUser(int clientfd, const std::string &command) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Received list command from client " << clientfd << std::endl;
    
    size_t space = command.find(" ");
    std::string user = command.substr(space+1);

    int objCount = 0;
    std::set<std::string> serverSet; // The set of servers that have user object files

    for (const auto &pair:objectManager.mainCopies) {
        for (const auto &fileObj: pair.second) {
            if (fileObj.user == user) {
                objCount += 1;
                serverSet.insert(pair.first);
            }
        }
    }

    for (const auto &pair:objectManager.backupCopies) {
        for (const auto &fileObj: pair.second) {
            if (fileObj.user == user) {
                objCount += 1;
                serverSet.insert(pair.first);
            }
        }
    }

    if (objCount == 0) {
        std::string serverResponse = user + " has no objects in the database";
        send(clientfd, serverResponse.c_str(), serverResponse.length(), 0);
        return;
    }

    std::string serverResponse = "total " + std::to_string(objCount) + "\n";
    for (const auto &server: serverSet) {
        std::cout << server << std::endl;
        serverResponse += listDirectory(server, user);
    }
    send(clientfd, serverResponse.c_str(), serverResponse.length(), 0);
}

void Server::uploadObj(int clientfd, const std::string &command) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Received upload command from client " << clientfd << std::endl;
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
    std::cout << fileName << " has been mapped to partition: " << partition << std::endl;
    std::string mainMachine = objectManager.partitionMap.lookup(partition); // Find the ipAddress of the machine the main copy will be store
    std::string backupMachine = objectManager.machineList.find(mainMachine)->next->ipAddress; // Find the ipAddress of the machine the back up copy will be stored
    std::cout << "Main copy of " << fileName << " should be stored in Machine with ipAddress " << mainMachine << std::endl;
    std::cout << "Backup copy of " << fileName << " should be stored in Machine with ipAddress " << backupMachine << std::endl;
    FileObject fileObj = FileObject(fileName, user, ds, partition);

    // Update the map for main copies and backup copies
    objectManager.mainCopies[mainMachine].push_back(fileObj);
    objectManager.backupCopies[backupMachine].push_back(fileObj);

    // Initialize the directories in the remote machines, will fail if they already exist
    createDir(mainMachine, "/tmp/kmondina");
    createDir(backupMachine, "/tmp/kmondina");
    createDir(mainMachine, "/tmp/kmondina/" + user);
    createDir(backupMachine, "/tmp/kmondina/" + user);

    // Transfer the file and its metadata to the appropriate machines
    transferObj(localFile, mainMachine + ":/tmp/kmondina/" + user);
    transferObj(localFileMD, mainMachine + ":/tmp/kmondina/" + user);
    transferObj(localFile, backupMachine + ":/tmp/kmondina/" + user);
    transferObj(localFileMD, backupMachine + ":/tmp/kmondina/" + user);

    // Clean up the saved file from local machine
    std::string cleanUpFile = "rm " + localFile;
    std::string cleanUpFileMD = "rm " + localFileMD;
    system(cleanUpFile.c_str());
    system(cleanUpFileMD.c_str());

    std::string serverAck = "Server: Main copy of " + fileName + " saved to " + mainMachine + "\n";
    serverAck += "Server: Backup copy of " + fileName + " saved to " + backupMachine + "\n";
    send(clientfd, serverAck.c_str(), serverAck.length(), 0);

    return;
}

void Server::deleteObj(int clientfd, const std::string &command) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Received download command from client " << clientfd << std::endl;

    size_t space = command.find(" ");
    std::string fileObject = command.substr(space+1);
    size_t slash = fileObject.find("/");
    std::string fileName = fileObject.substr(slash+1);
    std::string user = fileObject.substr(0,slash);

    uint64_t partition = hashAndMap(fileName, power);
    std::string mainMachine = objectManager.partitionMap.lookup(partition);
    std::string backupMachine = objectManager.machineList.find(mainMachine)->next->ipAddress;

    std::cout << "Main file mappings before object deletion" << std::endl;
    objectManager.printMapping(objectManager.mainCopies);
    std::cout << "Backup file mappings before object deletion" << std::endl;
    objectManager.printMapping(objectManager.backupCopies);

    std::vector<FileObject> mainMachineCopies = std::vector<FileObject>();
    std::vector<FileObject> backupMachinCopies = std::vector<FileObject>();
    std::map<std::string, std::vector<FileObject>> newMainCopies;
    std::map<std::string, std::vector<FileObject>> newBackupCopies;

    // Initialize machine copies
    for (const auto &pair : objectManager.mainCopies) {
        if (pair.first != mainMachine) {
            newMainCopies.insert(pair);
        }
        else {
            newMainCopies[mainMachine] = std::vector<FileObject>();
        }
    }
    for (const auto &pair : objectManager.backupCopies) {
        if (pair.first != backupMachine) {
            newBackupCopies.insert(pair);
        }
        else {
            newBackupCopies[backupMachine] = std::vector<FileObject>();
        }
    }

    bool objectFound = false;
    for (const auto &fileObject: objectManager.mainCopies[mainMachine]) {
        if (fileObject.fileName == fileName && fileObject.user == user) {
            objectFound = true;
        } else {
            mainMachineCopies.push_back(fileObject);
        }
    }
    std::string serverResponse;
    if (objectFound == false) {
        serverResponse = "Server: object requested not found";
        send(clientfd, serverResponse.c_str(), serverResponse.length(), 0);
        return;
    }
    
    newMainCopies[mainMachine] = mainMachineCopies;
    objectManager.mainCopies = newMainCopies;

    for (const auto &fileObject: objectManager.backupCopies[backupMachine]) {
        if (fileObject.fileName == fileName && fileObject.user == user) {
            objectFound = true;
        } else {
            backupMachinCopies.push_back(fileObject);
        }
    }

    newBackupCopies[backupMachine] = backupMachinCopies;
    objectManager.backupCopies = newBackupCopies;
    
    std::cout << "Main file mappings after object deletion" << std::endl;
    objectManager.printMapping(objectManager.mainCopies);
    std::cout << "Backup file mappings after object deletion" << std::endl;
    objectManager.printMapping(objectManager.backupCopies);
    serverResponse = "Server: requested object found and deleted";
    send(clientfd, serverResponse.c_str(), serverResponse.length(), 0);
    return;

}

void Server::addDisk(int clientfd, const std::string &command) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Received add command from client " << clientfd << std::endl;

    size_t space = command.find(" ");
    std::string ipAddress = command.substr(space+1);

    auto it = objectManager.servers.find(ipAddress);
    if (it != objectManager.servers.end()) {
        std::string serverResponse = ipAddress + " already present in the database\n";
        std::cout << serverResponse << std::endl;
        send(clientfd, serverResponse.c_str(), serverResponse.length(), 0);
        return;
    }

    std::string serverResponse = objectManager.addDisk(ipAddress);
    send(clientfd, serverResponse.c_str(), serverResponse.length(), 0);
    return;
}

void Server::removeDisk(int clientfd, const std::string &command) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Received remove command from client " << clientfd << std::endl;

    size_t space = command.find(" ");
    std::string ipAddress = command.substr(space+1);

    auto it = objectManager.servers.find(ipAddress);
    if (it == objectManager.servers.end()) {
        std::string serverResponse = ipAddress + " is not present in the database\n";
        std::cout << serverResponse << std::endl;
        send(clientfd, serverResponse.c_str(), serverResponse.length(), 0);
        return;
    }

    if (objectManager.machineCount <= 2) {
        std::string serverResponse = "Unable to remove machine, database needs at least two machines";
        std::cout << serverResponse << std::endl;
        send(clientfd, serverResponse.c_str(), serverResponse.length(), 0);
        return;
    }

    std::string serverResponse = objectManager.removeDisk(ipAddress);
    send(clientfd, serverResponse.c_str(), serverResponse.length(), 0);
    return;
}

void Server::cleanDisks(int clientfd, const std::string &command) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Received clean command from client " << clientfd << std::endl;

    std::string serverAck = "Server: \n";
    for (const auto &server: objectManager.servers) {
        deleteDir(server, "/tmp/kmondina");
        serverAck += server + ":/tmp/kmondina cleared\n";
    }

    send(clientfd, serverAck.c_str(), serverAck.length(), 0);
    return;
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

std::string createDigitalSignature(const std::string& filePath) {
    std::ifstream file(filePath.c_str(), std::ios::binary); // Convert to C-style string
    if (!file) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return "";
    }

    MD5_CTX mdContext;
    MD5_Init(&mdContext);

    const int bufferSize = 1024;
    char buffer[bufferSize];
    while (file.good()) {
        file.read(buffer, bufferSize);
        MD5_Update(&mdContext, buffer, file.gcount());
    }
    file.close();

    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5_Final(hash, &mdContext);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        ss << std::setw(2) << static_cast<unsigned>(hash[i]);
    }
    return ss.str();

}

