# This is the chat server program written in Python.
# Author: George Wen
# Usage: Execute this script with a port number as an argument to start the server.

import socket
import threading
from collections import defaultdict
import sys

# Server IP address and port number are set here.
# '127.0.0.1' is the loopback address, meaning the server will only be accessible locally.
HOST = '127.0.0.1'

# 'messages' dictionary stores messages for users. The keys are usernames, and the values are lists of messages.
messages = defaultdict(list)

def handle_client(client, address):
    """
    Handles communication with a single client in a dedicated thread.
    :param client: The client socket.
    :param address: The address of the client.
    """
    print(f"Connected with {address}")
    client.send('You are connected to the server.'.encode('ascii'))

    # Variables to track the state of the current user and message reception.
    global messages
    receiving = False  # Flag to indicate if the server is expecting message content.
    currentUser = None  # The username of the client.
    toUser = None  # The recipient of the message being composed.

    while True:
        try:
            message: str = client.recv(1024).decode('ascii')
            if not message:
                break  # Break out of the loop if no message is received.
            print(f"Received from {address}: {message}")

            if currentUser is None:
                # The first command must be LOGIN with a username.
                if message.startswith("LOGIN") and len(message.split()) == 2:
                    currentUser = message.split()[1]
                    # Send the number of unread messages to the user upon login.
                    msg = str(len(messages[currentUser])) + '\n'
                    client.send(msg.encode('ascii'))
                else:
                    print("Please issue LOGIN command first")
            else:
                if receiving:
                    # If in receiving mode, store the incoming message for the recipient.
                    messages[toUser].append({'from': currentUser, 'message': message})
                    receiving = False
                    client.send("MESSAGE SENT\n".encode('ascii'))
                else:
                    # Process commands: READ, COMPOSE, or EXIT.
                    if message.startswith("READ"):
                        if len(messages[currentUser]) > 0:
                            response = messages[currentUser].pop(0)
                            user = response['from'] + "\n"
                            client.send(user.encode('ascii'))
                            msg = response['message'] + '\n'
                            client.send(msg.encode('ascii'))
                        else:
                            client.send("READ ERROR\n".encode('ascii'))
                    elif message.startswith("COMPOSE") and len(message.split()) == 2:
                        toUser = message.split()[1]
                        receiving = True
                    elif message.startswith("EXIT"):
                        client.send("Exiting...".encode('ascii'))
                        break
                    else:
                        client.send("Unknown command".encode('ascii'))
        except Exception as e:
            print(f"Error with {address}: {e}")
            break

    print(f"Connection closed with {address}")
    client.close()

def main(port):
    """
    The main function that sets up the server.
    :param port: The port number on which the server will listen for incoming connections.
    """
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        try:
            server_socket.bind((HOST, port))
        except socket.error as exception:
            print(f'Bind failed. Error Code : {str(exception[0])} Message {exception[1]}')
            sys.exit()

        server_socket.listen()
        print(f"Server listening on {HOST}:{port}")

        while True:
            client, address = server_socket.accept()
            thread = threading.Thread(target=handle_client, args=(client, address))
            thread.start()

if __name__ == '__main__':
    args = sys.argv[1:]
    if len(args) == 1 and args[0].isdigit():
        main(int(args[0]))
    else:
        print("Correct usage: script.py <PORT>")

