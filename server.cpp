#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <io.h> // For _access
#include <filesystem>
#include <thread>


#define SIZE 1024
#define SIZENAME 1024

struct RequestData {
    char userID[5];
    char version;
    char op;
    short fileNameLength;
    char filename[SIZENAME]; // You'll need to dynamically allocate this
    int size;
    char payload[SIZE]; // You'll need to dynamically allocate this
};
// NOTE : need to load the last data from the tmp to the folder user.

struct answer {
    char version;
    char status;
    short fileNameLength;
    char filename[SIZENAME];
    int size;
    char payload[SIZE];
};
namespace fs = std::filesystem;

void parseServerInfo(const std::string& filePath, std::string& ip, int& port) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("File not found: " + filePath);
    }

    std::string serverInfo;
    std::getline(file, serverInfo);
    file.close();

    std::istringstream ss(serverInfo);
    std::string ipPart, portPart;

    if (std::getline(ss, ipPart, ':') && std::getline(ss, portPart)) {
        ip = ipPart;
        port = std::stoi(portPart);
    } else {
        throw std::runtime_error("Invalid server info format in the file: " + filePath);
    }
}

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

void loadFile(SOCKET newSocket, const std::string& fullUserFolderPath, const std::string& filename, long fileSize) {
    // Construct the full file path
    std::string fullFilePath = fullUserFolderPath + "\\" + filename;

    // Open the file in binary mode for writing
    std::ofstream file(fullFilePath.c_str(), std::ios::out | std::ios::binary);

    char buffer[SIZE];
    long totalBytesReceived = 0;

    while (totalBytesReceived <  fileSize) {
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

void deleteFile(const std::string& filePath) {
    if (std::remove(filePath.c_str()) == 0) {
        std::cout << "File '" << filePath << "' deleted successfully." << std::endl;
    } else {
        std::cerr << "Error deleting file '" << filePath << "'" << std::endl;
    }
}
void loadAndDelete(SOCKET newSocket, const std::string& basePath){
    RequestData requestData1;
    std::string userID,filename;
    int dataSize;
    if (recv(newSocket, (char *) &requestData1, sizeof(RequestData), 0) != SOCKET_ERROR) {
        // Assign each field to separate variables
        userID = requestData1.userID;
        filename = requestData1.filename;
    }
    send(newSocket, "Loading the data for tmp folder", sizeof("Loading the data for tmp folder"), 0);
    std:: string userPath =basePath + userID;
    std::string fullUserFolderPath = basePath + userID + "\\" + filename;
    RequestData requestData2;
    if (recv(newSocket, (char *) &requestData2, sizeof(RequestData), 0) != SOCKET_ERROR) {
        // Assign each field to separate variables
        userID = requestData2.userID;
        filename = requestData2.filename;
        char version = requestData2.version;
        char op = requestData2.op;
    }
    deleteFile(fullUserFolderPath);
    send(newSocket, "Deleted the first file", sizeof("Deleted the first file"), 0);
    char fileInfo[SIZE];
    memset(fileInfo, 0, SIZE);
    recv(newSocket, fileInfo, SIZE, 0);


    // Parse file information
    char *filenamestr = strtok(fileInfo, "_");
    char *fileSizeStr = strtok(nullptr, "_");
    long fileSize = std::stol(fileSizeStr);

    std::string message = "Load the data agian" + std::string(filenamestr);
    // Send the message
    send(newSocket, message.c_str(), message.size(), 0);

    // Receive and save the file as a binary file
    loadFile(newSocket,userPath,filenamestr,fileSize);

}
std::vector<std::string> listFilesInDirectory(const std::string& directoryPath) {
    std::vector<std::string> fileNames;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
            if (entry.is_regular_file()) {
                fileNames.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }

    return fileNames;
}

void handleList1(SOCKET newSocket, const std::string& basePath){

    RequestData requestData;
    std::string userID;
    if (recv(newSocket, (char *) &requestData, sizeof(RequestData), 0) != SOCKET_ERROR) {
        // Assign each field to separate variables
        userID = requestData.userID;
        char version = requestData.version;
        char op = requestData.op;
    }
    std::string fullUserFolderPath = basePath + userID;
    send(newSocket, "211 : These are the file for", sizeof("211 : These are the file for"), 0);
    std::thread(loadAndDelete, newSocket, basePath).detach();
}
void handleList(SOCKET newSocket, const std::string& basePath){
    RequestData requestData;
    answer answer;
    std::string userID;
    if (recv(newSocket, (char *) &requestData, sizeof(RequestData), 0) != SOCKET_ERROR) {
        // Assign each field to separate variables
        answer.version = requestData.version;
        answer.status = 211;
        userID = requestData.userID;
    }
    std::string fullUserFolderPath = basePath + userID;

    send(newSocket, "211 : These are the file", sizeof("211 : These are the file"), 0);
}
void handleClient(SOCKET newSocket, const std::string& basePath) {
    // Receive the userID
    RequestData requestData;
    std::string userID;
    char version,op;
    short fileNameLength;
    int size;
    // Receive the struct as binary data
    if (recv(newSocket, (char *) &requestData, sizeof(RequestData), 0) != SOCKET_ERROR) {
        // Assign each field to separate variables
        userID = requestData.userID;
        version = requestData.version;
        op = requestData.op;

    }
    send(newSocket, "212 : The folder user is created", sizeof("212 : The folder user is created"), 0);

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

    while (true) {


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
        loadFile(newSocket,fullUserFolderPath,filename,fileSize);

    }
    std::thread(handleList1, newSocket, basePath).detach();
}


void startServer(const std::string &basePath) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }
    std::string filePath = basePath + "/server.info"; // Replace with the actual path to your server info file
    std::string ip;
    int port;
    parseServerInfo(filePath,ip,port);

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
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    // Bind the socket
    if (bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error binding socket." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 10) == 0) {
        std::cout << "Server listening on port " << port << " ..." << std::endl;
    } else {
        std::cerr << "Error listening on port " << port << "." << std::endl;
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

        std::thread(handleList, newSocket, basePath).detach();

        std::thread(handleClient, newSocket, basePath).detach();

    }

    // Cleanup code (if needed) will be executed after breaking out of the loop
    closesocket(serverSocket);
    WSACleanup();
}

int main() {
    std::string basePath = "C:\\Users\\jabbour.dandan\\CLionProjects\\serverProject1\\";
    startServer(basePath);
    std::cout << "Server stopped." << std::endl;
    return 0;
}
