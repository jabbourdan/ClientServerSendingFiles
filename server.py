#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <direct.h>
#include <io.h> // For _access

#define PORT 4456
#define SIZE 1024

bool isValidUserID(const std::string& userId) {
    // Check if userID is exactly 4 characters
    return userId.length() == 4;
}

bool folderExists(const std::string& path) {
    return _access(path.c_str(), 0) == 0;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }

    SOCKET serverSocket, newSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int addr_size = sizeof(clientAddr);

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket." << std::endl;
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error binding socket." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 10) == 0) {
        std::cout << "Server listening on port " << PORT << "..." << std::endl;
    } else {
        std::cerr << "Error listening on port " << PORT << "." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    newSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addr_size); // Accept connection

    // Receive the userID
    char userId[SIZE];
    recv(newSocket, userId, SIZE, 0);
    std::string userIdStr(userId);

    std::string path = "C:\\Users\\jabbour.dandan\\CLionProjects\\server2\\";
    std::string fullUserFolderPath = path + userIdStr;

    if (!isValidUserID(userIdStr)) {
        std::cerr << "Invalid userID format." << std::endl;
        closesocket(newSocket);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Check if the user's folder already exists
    if (!folderExists(fullUserFolderPath)) {
        // Create the user's folder
        if (_mkdir(fullUserFolderPath.c_str()) != 0) {
            std::cerr << "Error creating user folder." << std::endl;
            closesocket(newSocket);
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }
    }

    send(newSocket, "The folder user is created", sizeof("The folder user is created"), 0);

    // Receive file information (filename and size) from the client
    char fileInfo[SIZE];
    recv(newSocket, fileInfo, SIZE, 0);
    send(newSocket, "File info received", sizeof("File info received"), 0);

    // Parse file information
    char* filename = strtok(fileInfo, "_");
    char* fileSizeStr = strtok(nullptr, "_");
    long fileSize = std::stol(fileSizeStr);

    // Receive and save the file as a text file
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
    closesocket(newSocket);
    closesocket(serverSocket);

    WSACleanup();

    std::cout << "File received and saved as text file: " << filename << std::endl;

    return 0;
}
