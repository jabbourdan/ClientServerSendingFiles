import os
import socket
import struct
from tqdm import tqdm

IP = socket.gethostbyname(socket.gethostname())
PORT = 4456
ADDR = (IP, PORT)
SIZE = 1024
FORMAT = "utf-8"
PATH="backup.info/"

def send_file_data(client, file_path):
    with open(file_path, "rb") as f:
        bar = tqdm(total=os.path.getsize(file_path), unit="B", unit_scale=True, unit_divisor=SIZE)
        while True:
            data = f.read(SIZE)
            if not data:
                break
            client.send(data)
            bar.update(len(data))
        bar.close()

def send_user_id(client, user_id):
    client.send(user_id.encode(FORMAT))
    msg = client.recv(SIZE).decode(FORMAT)
    print(f"SERVER: {msg}")

def sendRequest(userID, version, op, fileName, size):
    if fileName is None:
        fileNameLength = 0
        fileNameBytes = b''  # Empty bytes for file name
    else:
        fileNameLength = len(fileName)
        fileNameBytes = fileName.encode('utf-8')  # Encode file name as bytes
    if(size == None):
        size = 0
    # Create the binary struct
    request_struct = struct.pack(f'5sbbh{fileNameLength}sI', userID.encode('utf-8'), version, op, fileNameLength, fileNameBytes, size)

    return request_struct

def main():
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(ADDR)
    # user ID
    user_id = input("Enter User ID: ")


    #send reques for list all the data of this user



    # send a request for save the files.
    req = sendRequest(user_id,1,100,None,None)
    client.send(req)

    msg = client.recv(SIZE).decode(FORMAT)
    print(f"SERVER: {msg}")


    all_files = os.listdir(PATH)
    for fileName in all_files:
        # Sending the filename and filesize to the server.
        file_size = os.path.getsize(PATH + fileName)
        data = f"{fileName}_{file_size}"
        client.send(data.encode(FORMAT))
        msg = client.recv(SIZE).decode(FORMAT)
        print(f"SERVER: {msg}")
    
        # Sending file data with a progress bar.
        send_file_data(client, PATH + fileName)

    exitOrNot = "END"
    client.send(exitOrNot.encode(FORMAT))
    msg = client.recv(SIZE).decode(FORMAT)
    print(f"SERVER: {msg}")
    # Closing the connection.
    client.close()

if __name__ == "__main__":
    main()
