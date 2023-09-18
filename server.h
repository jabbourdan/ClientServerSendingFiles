#pragma once

#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <io.h>
#include <filesystem>
#include <thread>
#include <vector>
#include <sstream>

#define SIZE 1024
#define SIZENAME 1024

// Structure to hold request data
struct RequestData {
    char userID[5];
    char version;
    char op;
    char filename[SIZENAME]; // You'll need to dynamically allocate this
    // You'll need to dynamically allocate this
};

// NOTE: need to load the last data from the tmp to the folder user.

namespace fs = std::filesystem;

// Function to parse server information from a file
void parseServerInfo(const std::string& filePath, std::string& ip, int& port);

// Function to check if a user ID is valid and the user's folder exists
bool isValidUserIDAndFolderExist(const std::string& userId, const std::string& basePath);

// Function to load a file from a socket and save it to a folder
void loadFile(SOCKET newSocket, const std::string& fullUserFolderPath, const std::string& filename, long fileSize);

// Function to delete a file
void deleteFile(const std::string& filePath);

// Function to handle loading and deleting a file
void loadAndDelete(SOCKET newSocket, const std::string& basePath);

// Function to handle listing files in a directory
void handleList1(SOCKET newSocket, const std::string& basePath);

// Function to handle listing files in a directory
void handleList(SOCKET newSocket, const std::string& basePath);

// Function to handle a client's connection
void handleClient(SOCKET newSocket, const std::string& basePath);

// Function to start the server
void startServer(const std::string& basePath);
