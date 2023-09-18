# Describtion of the Project:

In this project, we will implement a server software for backing up and retrieving files and a client software that will work against it.

The server will be written in C++ and the client in Python.

# server:

The server will allow any client to send files to it for backup and retrieve these files at a later time.
Server characteristics:

A. The server will support a stateless protocol, that is, will not save information between requests 1
(Each request stands on its own).

B. The server will support multiple users through threads.

# client:

The client will work against the server according to the protocol.
At the beginning of the run, each customer generated a unique 4-byte random number. This number will be used in all requests
to be sent to the server.

# address:
The server address and port will be read from a file in the following way:
- File name: info.server
- File location: in the same folder as the main Python file
- File content: IP address + colon + port number

NOTE: you can change the IP and the PORT in the server.info file.

# backup:

The backup folder is a folder for storing data like .txt pdf and so on you can add or remove files as you wish.

NOTE: you need to keep at least one file in the backup.info folder.


## Installation
First of all, because I work with a work computer I can not make any folder in the c: directly so I made the folders user on the server so please do the same.
just change the basePath to the server path it's important otherwise the server will not be able to read the server.info (I could change it but it's my work computer)

![init](https://github.com/jabbourdan/ClientServerSendingFiles/assets/111487226/463e7ae1-7592-4a29-b3b4-73e64acc4302)

Install my-project with npm

```bash
  git clone https://github.com/jabbourdan/ClientServerSendingFiles.git
  pip freeze > requirements.txt
```
If the pip freeze don't work you can just download it on your computer this library for python:

```bash
  tqdm==4.62.0
```

IMPORTANT NOTE: This project was made with the clone IDV make sure to copy the makefile and the server.info to the CLION and the server.info to where you want to run the python file (I ran the python with the VSCode)and the fodlers should be with the pythhon file.
NOTE: run the server first and then the client.


## Screen shot for installation:

The server folder should be:

![The_server](https://github.com/jabbourdan/ClientServerSendingFiles/assets/111487226/858aca29-3b41-4d8b-b318-29bec207656a)

The client folder should be:

![The client](https://github.com/jabbourdan/ClientServerSendingFiles/assets/111487226/605902b8-cdbc-4652-9b02-762a8cdb1072)

## Screenshots:
After we run the server and run the client the client will be asked from you and ID:
![first step](https://github.com/jabbourdan/ClientServerSendingFiles/assets/111487226/c84f9c03-d975-4c82-9a32-aef06f15fb58)

The client will send the data and the server will delete it after hey save the data in the tmp folder.
This are the output of the client:
![ClientOutput](https://github.com/jabbourdan/ClientServerSendingFiles/assets/111487226/e75f8a47-e4ce-4da2-bab2-04ff3cdb6842)



