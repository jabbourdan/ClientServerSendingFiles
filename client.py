# Import necessary modules
import os
import socket
import struct
from tqdm import tqdm  # for progress bar
import shutil

# Define constants
IP = socket.gethostbyname(socket.gethostname())
PORT = 4456
ADDR = (IP, PORT)
SIZE = 1024
FORMAT = "utf-8"
PATH = "backup.info/"
PATH_TEMP = "tmp/"
PATH_FOR_SERVERINFO = "server.info"

# Function to parse server information from a file
def parse_server_info(file_path):
    try:
        with open(file_path, 'r') as file:
            server_info = file.read().strip()  # Read the content of the file and remove leading/trailing whitespaces

            # Split the server info by ':' to separate the IP and port
            ip, port = server_info.split(':')

            # Convert the port to an integer
            port = int(port)

            return ip, port
    except FileNotFoundError:
        print(f"File not found: {file_path}")
    except Exception as e:
        print(f"An error occurred: {str(e)}")

# Function to move a folder to a temporary location
def moverFolderToTmp(file_name, user_id):
    # Define the source and destination folders
    source_folder = "backup.info/"  # Replace with the path to your source folder
    destination_folder = f"tmp/{user_id}/"  # Replace with the path to your destination folder

    # Create the user's folder if it doesn't exist
    if not os.path.exists(destination_folder):
        os.makedirs(destination_folder)

    # Check if the file exists in the source folder
    if os.path.isfile(os.path.join(source_folder, file_name)):
        # Construct the source and destination paths
        source_path = os.path.join(source_folder, file_name)
        destination_path = os.path.join(destination_folder, file_name)

        # Move the file to the user's folder
        shutil.copy(source_path, destination_path)
        print(f"File '{file_name}' moved to user folder '{user_id}'")
    else:
        print(f"File '{file_name}' not found in the source folder")

# Function to send file data with a progress bar
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

# Function to send the user ID to the server
def send_user_id(client, user_id):
    client.send(user_id.encode(FORMAT))
    msg = client.recv(SIZE).decode(FORMAT)
    print(f"SERVER: {msg}")

# Function to load files
def loadTheFiles(client):
    all_files = os.listdir(PATH)
    for fileName in all_files:
        # Sending the filename and filesize to the server.
        fileSize = os.path.getsize(PATH + fileName)
        
        data = f"{fileName}_{fileSize}"
        client.send(data.encode(FORMAT))
        msg = client.recv(SIZE).decode(FORMAT)
        print(f"SERVER: {msg}")
    
        # Sending file data with a progress bar.
        send_file_data(client, PATH + fileName)

    exitOrNot = "END"
    
    client.send(exitOrNot.encode(FORMAT))
    msg = client.recv(SIZE).decode(FORMAT)
    print(f"SERVER: {msg}")

# Function to create and send a request
def sendRequest(userID, version, op, fileName, size):
    if fileName is None:
        fileNameLength = 0
        fileNameBytes = b''  # Empty bytes for file name
    else:
        fileNameLength = len(fileName)
        fileNameBytes = fileName.encode('utf-8')  # Encode file name as bytes

    if(size is None):
        size = 0
    
    # Create the binary struct
    request_struct = struct.pack(f'5sBBh{fileNameLength}sI', userID.encode('utf-8'), version, op, fileNameLength, fileNameBytes, size)

    return request_struct

# Main function
def main():
    try:
        ip, port = parse_server_info(PATH_FOR_SERVERINFO)
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        ADDR = (ip, port)
        client.connect(ADDR)
    except Exception as e:
        print(f"Error connecting to the server: {e}")    

    # Get user ID
    user_id = input("Enter User ID: ")

    # Send a request to list all the data of this user
    req = sendRequest(user_id, 1, 202, None, None)
    client.send(req)

    msg = client.recv(SIZE).decode(FORMAT)
    print(f"SERVER: {msg}")

    # Send a request to save the files
    req1 = sendRequest(user_id, 1, 100, None, None)
    client.send(req1)

    msg1 = client.recv(SIZE).decode(FORMAT)
    print(f"SERVER: {msg1}")

    all_files = os.listdir(PATH)
    first_file = all_files[0]
    loadTheFiles(client)
    
    req3 = sendRequest(user_id, 1, 202, None, None)
    client.send(req3)

    msg2 = client.recv(SIZE).decode(FORMAT)
    print(f"SERVER: {msg2}")

    # Steps 8 - 10

    req4 = sendRequest(user_id, 1, 200, first_file, None)
    client.send(req4)

    moverFolderToTmp(all_files[0], user_id)

    msg4 = client.recv(SIZE).decode(FORMAT)
    print(f"SERVER: {msg4}")

    # Close the connection

    # Delete the file
    req5 = sendRequest(user_id, 1, 201, first_file, None)
    client.send(req5)

    msg5 = client.recv(SIZE).decode(FORMAT)
    print(f"SERVER: {msg5}")

    # Sending the filename and filesize to the server.
    fileSize = os.path.getsize(PATH + first_file)
        
    data = f"{first_file}_{fileSize}"
    client.send(data.encode(FORMAT))
    msg = client.recv(SIZE).decode(FORMAT)
    print(f"SERVER: {msg}")
    
    # Sending file data with a progress bar.
    send_file_data(client, PATH_TEMP + user_id + "/" + first_file)
    
    # Close the connection
    client.close()

# Entry point
if __name__ == "__main__":
    main()
