#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <io.h> // For _access
#include <filesystem>
#include <thread>


#define PORT 4456
#define SIZE 1024

struct RequestData {
    char userID[5];
    char version;
    char op;
    short fileNameLength;
    char* filename; // You'll need to dynamically allocate this
    int size;
    char* payload; // You'll need to dynamically allocate this
};


namespace fs = std::filesystem;

bool isValidUserIDAndFolderExist(const std::string& userId, const std::string& basePath) {
    // Check if userID is exactly 4 characters
    if (userId.length() != 4) {
        return false;
    }
    // Check if the user's folder already exists
    std::string fullUserFolderPath = basePath + userId;
    if(fs::exists(fullUserFolderPath)){
        return false;
    }
    return true;
}

void handleClient(SOCKET newSocket, const std::string& basePath) {
    // Receive the userID
    /*char userId[SIZE];
    recv(newSocket, userId, SIZE, 0);
    std::string userIdStr(userId);*/
    RequestData requestData;
    std::string userID;

    // Receive the struct as binary data
    if (recv(newSocket, (char *) &requestData, sizeof(RequestData), 0) != SOCKET_ERROR) {
        // Assign each field to separate variables
        userID = requestData.userID;
        char version = requestData.version;
        char op = requestData.op;
        short fileNameLength = requestData.fileNameLength;
        int size = requestData.size;
        requestData.filename = new char[requestData.fileNameLength];
        requestData.payload = new char[requestData.size];
    }


        std::string fullUserFolderPath = basePath + userID;
        if (!isValidUserIDAndFolderExist(userID, basePath)) {
            std::cerr << "1003 : Invalid userID format or user folder does exist." << std::endl;
            send(newSocket, "1003 : User folder does exist.", sizeof("1003 : User folder does exist."), 0);
            closesocket(newSocket);
            return;  // Return to terminate the thread
        } else if (!fs::create_directory(fullUserFolderPath)) {
            std::cerr << "Failed to create user folder." << std::endl;
            send(newSocket, "1003 : Failed to create user folder.", sizeof("1003 : Failed to create user folder."), 0);
            closesocket(newSocket);
            return;
        }

        send(newSocket, "212 : The folder user is created", sizeof("212 : The folder user is created"), 0);

        while (true) {
            // Receive file information (filename and size) from the client
            char fileInfo[SIZE];
            memset(fileInfo, 0, SIZE);
            recv(newSocket, fileInfo, SIZE, 0);

            // Check if the client has indicated the end of file transfers
            if (strcmp(fileInfo, "END") == 0) {
                send(newSocket, "File transfer completed.", sizeof("File transfer completed."), 0);
                break;
            }


            // Parse file information
            char *filename = strtok(fileInfo, "_");
            char *fileSizeStr = strtok(nullptr, "_");
            long fileSize = std::stol(fileSizeStr);

            std::string message = "212 : File info received for " + std::string(filename);
            // Send the message
            send(newSocket, message.c_str(), message.size(), 0);
            // Receive and save the file as a binary file
            std::string fullFilePath = fullUserFolderPath + "\\" + std::string(filename);
            std::ofstream file(fullFilePath.c_str(), std::ios::out | std::ios::binary); // Open in binary mode

            char buffer[SIZE];
            long totalBytesReceived = 0;
            while (totalBytesReceived < fileSize) {
                int bytesReceived = recv(newSocket, buffer, SIZE, 0);
                if (bytesReceived <= 0) {
                    break; // Error or end of data
                }
                file.write(buffer, bytesReceived);
                totalBytesReceived += bytesReceived;
            }

            file.close();

            std::cout << "File received and saved as: " << filename << std::endl;
        }

        closesocket(newSocket);
    }


    void startServer(const std::string &basePath) {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed." << std::endl;
            return;
        }

        SOCKET serverSocket, newSocket;
        struct sockaddr_in serverAddr, clientAddr;
        int addr_size = sizeof(clientAddr);

        // Create socket
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Error creating socket." << std::endl;
            WSACleanup();
            return;
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PORT);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        // Bind the socket
        if (bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Error binding socket." << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return;
        }

        // Listen for incoming connections
        if (listen(serverSocket, 10) == 0) {
            std::cout << "Server listening on port " << PORT << "..." << std::endl;
        } else {
            std::cerr << "Error listening on port " << PORT << "." << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return;
        }

        while (true) {  // Infinite loop to keep the server running
            newSocket = accept(serverSocket, (struct sockaddr *) &clientAddr, &addr_size); // Accept connection

            if (newSocket == INVALID_SOCKET) {
                std::cerr << "Error accepting connection." << std::endl;
                continue;  // Skip the rest of the loop and wait for the next connection
            }
            // Create a new thread to handle the client for save the file.
            std::thread(handleClient, newSocket, basePath).detach();
        }

        // Cleanup code (if needed) will be executed after breaking out of the loop
        closesocket(serverSocket);
        WSACleanup();
    }

    int main() {
        std::string basePath = "C:\\Users\\jabbour.dandan\\CLionProjects\\server2\\";
        startServer(basePath);
        std::cout << "Server stopped." << std::endl;
        return 0;
    }
