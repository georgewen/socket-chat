import socket
import threading
from collections import defaultdict
import sys

# Server IP and port
HOST = '127.0.0.1'
#PORT = 12345
# store messages for users
messages = defaultdict(list)

# Handle communication with the client
def handle_client(client, address):
    print(f"Connected with {address}")
    client.send('You are connected to the server.'.encode('ascii'))

    # first message must be LOGIN
    # reading incoming message after received a COMPOSE command
    global messages
    receiving = False
    currentUser = None
    toUser = None
    while True:
        try:
            message: str = client.recv(1024).decode('ascii')
            if not message:
                break
            print(f"Received from {address}: {message}")
            #if not loggedin and message.start
            if currentUser is None:
                if  message.startswith("LOGIN") and len(message.split()) == 2:
                    currentUser = message.split()[1]
                    msg = str(len(messages[currentUser]))
                    client.send(msg.encode('ascii'))
                else:
                    print("error1")
                    break   
            else:
                if receiving:
                    messages[toUser].append({'from': currentUser, 'message': message})
                    receiving = False
                    response = "MESSAGE SENT"
                    client.send(response.encode('ascii'))
                else:
                    # READ or COMPOSE
                    if message.startswith("READ"):
                        if len(messages[currentUser]) > 0 :
                            response = messages[currentUser].pop()
                            user = response['from'] #+ "\n"
                            client.send(user.encode('ascii'))
                            msg = response['message']
                            client.send(msg.encode('ascii'))
                        else:
                            msg = "READ ERROR"
                            client.send(msg.encode('ascii'))                        
                    elif message.startswith("COMPOSE") and len(message.split()) == 2:
                        toUser = message.split()[1]
                        receiving = True
                    elif message == "EXIT":
                        print("Exit")
                        break                   
                    else:
                        print("error2")
                        break

            #response = f"Server received your message: {message}"
            #client.send(response.encode('ascii'))

        except Exception as e:
            print(f"Error with {address}: {e}")
            break

    print(f"Connection closed with {address}")
    client.close()

def main(port):

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.bind((HOST, port))
        server_socket.listen()
        print(f"Server listening on {HOST}:{port}")

        while True:
            client, address = server_socket.accept()

            #print ('Got connection from', address )
            #client.send('Thank you for connecting'.encode()) 
            thread = threading.Thread(target=handle_client, args=(client, address))
            thread.start()

if __name__ == '__main__':
    args = sys.argv[1:]
    print(args[0])
    main(int(args[0]))