import os
import socket
from tqdm import tqdm

IP = socket.gethostbyname(socket.gethostname())
PORT = 4456
ADDR = (IP, PORT)
SIZE = 1024
FORMAT = "utf-8"
FILENAME = "friends-final.txt"

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

def main():
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(ADDR)

    # user ID
    user_id = input("Enter User ID: ")
    send_user_id(client, user_id)

    # Sending the filename and filesize to the server.
    file_size = os.path.getsize(FILENAME)
    data = f"{FILENAME}_{file_size}"
    client.send(data.encode(FORMAT))
    msg = client.recv(SIZE).decode(FORMAT)
    print(f"SERVER: {msg}")
    
    # Sending file data with a progress bar.
    send_file_data(client, FILENAME)

    # Closing the connection.
    client.close()

if __name__ == "__main__":
    main()
