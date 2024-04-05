import socket
import threading
import sys

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

def main(host, port):
    username = input("Please enter your username: ")
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
        try:
            client_socket.connect((host, port))
        except:
            print("error connecting to server") 
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
                        continue

                elif message.startswith("READ"):
                    pass
                elif message == "EXIT":
                    print("EXIT")
                    client_socket.send(message.encode('ascii'))
                    break
                else:
                    print("error command")
                    continue
                client_socket.send(message.encode('ascii'))
        print("Outside Loop")
        client_socket.close()
        thread.join()

# Main function to connect to the server and send messages
if __name__ == '__main__':

    args = sys.argv[1:]
    if len(args) == 2 and args[1].isdigit(): 
        main(args[0], int(args[1]))
    else:
        print("Please supply host address and port number as parameters!")    

        
