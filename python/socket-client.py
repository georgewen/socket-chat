# This is the chat client program written in Python.
# Author: George Wen
# Usage: Execute this script with the server's IP address and port number as arguments to connect to the chat server.

import socket
import threading
import sys

def receive_messages(client):
    """
    Function to continuously receive messages from the server and print them out.
    It runs in a separate thread to allow the user to type messages simultaneously.
    :param client: The socket object connected to the server.
    """
    while True:
        try:
            message = client.recv(1024).decode('ascii')
            print(message)
        except:
            # This block catches any exceptions that may occur during reception of messages.
            # It could be due to server disconnection or network issues.
            print("Error occurred. Exiting...")
            client.close()
            break

def main(host, port):
    """
    The main function where the client prompts the user for a username,
    connects to the server, and handles sending messages.
    :param host: The IP address of the server to connect to.
    :param port: The port number of the server.
    """
    username = input("Please enter your username: ")
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
        try:
            client_socket.connect((host, port))
        except:
            # Handles exceptions that may occur while trying to connect to the server.
            print("Error connecting to server")
            return

        # Once connected, the client sends a LOGIN command followed by the username.
        message = f'LOGIN {username}\n'
        client_socket.send(message.encode('ascii'))

        # Start a thread to receive messages from the server without blocking input from the user.
        thread = threading.Thread(target=receive_messages, args=(client_socket,))
        thread.start()

        sending = False  # A flag to check if the next message from the user should be sent as a message content.
        while True:
            message = input("")  # Input message from the user.
            if sending:
                # If sending flag is True, the message is considered as content for the previous COMPOSE command.
                client_socket.send((message+'\n').encode('ascii'))
                sending = False
            else:
                # Handle different commands: COMPOSE, READ, EXIT.
                if message.startswith("COMPOSE"):
                    if len(message.split()) == 2:
                        client_socket.send((message+'\n').encode('ascii'))
                        sending = True  # Next message will be sent as content.
                    else:
                        print("Error: correct message format is COMPOSE <username>")
                elif message.startswith("READ") or message.startswith("EXIT"):
                    client_socket.send((message+'\n').encode('ascii'))
                    if message.startswith("EXIT"):
                        break  # Exit the loop and close the connection if EXIT command is issued.
                else:
                    print("Error: unknown command")
                    continue

        # Clean up the connection and wait for the receiving thread to finish.
        client_socket.close()
        thread.join()

if __name__ == '__main__':
    # Check command line arguments for server IP and port.
    args = sys.argv[1:]
    if len(args) == 2 and args[1].isdigit():
        main(args[0], int(args[1]))
    else:
        print("Correct usage: script.py <Server IP> <PORT>")

