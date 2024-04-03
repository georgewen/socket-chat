import socket
import threading
from collections import defaultdict

# Server IP and port
HOST = '127.0.0.1'
PORT = 12345

# store messages for users
messages = defaultdict(list)


# Handle communication with the client
def handle_client(client, address):
    print(f"Connected with {address}")
    client.send('You are connected to the server.'.encode('ascii'))

    # first message must be LOGIN
    # reading incoming message after received a COMPOSE command
    global messages
    loggedin = False
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
            if not loggedin:
                if not message.startswith("LOGIN"):
                    print("error1")
                    break
                else:
                    if len(message.split()) == 2:
                        currentUser = message.split()[1]
                        loggedin = True
                        continue
                    else:
                        print("error2")
                        break
            # read or compose
            if message.startswith("READ"):

                if len(messages[currentUser]) > 0 :
                    response = messages[currentUser].pop()
                    msg = response['message']
                    client.send(msg.encode('ascii'))
                else:
                    msg = "no more message"
                    client.send(msg.encode('ascii'))

                continue        

            if not receiving:
                if not message.startswith("COMPOSE"):
                    print("error3")
                    break
                else:
                    if len(message.split()) == 2:
                        toUser = message.split()[1]
                        receiving = True
                        continue
                    else:
                        print("error4")
                        break
            else: # receiving
                messages[toUser].append({'from': currentUser, 'message': message})
            

            response = f"Server received your message: {message}"
            client.send(response.encode('ascii'))

        except Exception as e:
            print(f"Error with {address}: {e}")
            break

    print(f"Connection closed with {address}")
    client.close()

if __name__ == '__main__':


    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.bind((HOST, PORT))
        server_socket.listen()
        print(f"Server listening on {HOST}:{PORT}")

        while True:
            client, address = server_socket.accept()

            #print ('Got connection from', address )
            #client.send('Thank you for connecting'.encode()) 

            thread = threading.Thread(target=handle_client, args=(client, address))
            thread.start()