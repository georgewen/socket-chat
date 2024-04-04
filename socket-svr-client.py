import socket
import threading

# Server IP and port to connect to
HOST = '127.0.0.1'
PORT = 12345

# Function to receive messages from the server
def receive_messages(client):
    while True:
        try:
            message = client.recv(1024).decode('ascii')
            print(message)
        except:
            print("Error occurred. Exiting...")
            client.close()
            break

# Main function to connect to the server and send messages
if __name__ == '__main__':

    username = input("Please enter your username: ")
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
        client_socket.connect((HOST, PORT))
        # send LOGIN command
        message = f'LOGIN {username}'
        client_socket.send(message.encode('ascii'))

        thread = threading.Thread(target=receive_messages, args=(client_socket,))
        thread.start()

        sending = False
        while True:
            message = input("")
            # message should be COMPOSE <username> => message, READ, EXIT
            if sending:
                # input is message content
                client_socket.send(message.encode('ascii'))
                sending = False
            else:
                if message.startswith("COMPOSE"):
                    if len(message.split()) == 2:
                        sending = True
                    else:
                        # error
                        print("error: correct message format is COMPOSE <username>")
                        break

                elif message.startswith("READ"):
                    pass
                elif message == "EXIT":
                    print("EXIT")
                    client_socket.send(message.encode('ascii'))
                    break
                else:
                    print("error command")
                    break
                client_socket.send(message.encode('ascii'))
        print("Outside Loop")
        client_socket.close()
        thread.join()
        

