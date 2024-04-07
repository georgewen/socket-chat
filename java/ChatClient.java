import java.io.*;
import java.net.*;
import java.util.Scanner;

public class ChatClient {
    private static Socket clientSocket;
    private static PrintWriter out;
    private static BufferedReader in;
    private static Scanner scanner = new Scanner(System.in);

    public static void main(String[] args) {
        if (args.length != 2) {
            System.out.println("Usage: java ChatClient <host> <port>");
            return;
        }
        
        String host = args[0];
        int port = Integer.parseInt(args[1]);
        boolean sending = false;

        try {
            clientSocket = new Socket(host, port);
            out = new PrintWriter(clientSocket.getOutputStream(), true);
            in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));

            System.out.print("Please enter your username: ");
            String username = scanner.nextLine();
            sendMessage("LOGIN " + username);

            // Start a thread to listen for messages from the server
            new Thread(() -> {
                try {
                    receiveMessages();
                } catch (IOException e) {
                    System.out.println("Error occurred. Exiting...");
                    try {
                        clientSocket.close();
                    } catch (IOException ex) {
                        System.out.println("Error closing the socket");
                    }
                }
            }).start();

            // Main loop for sending messages
            while (true) {
                String message = scanner.nextLine();

                if (sending) {
                    sendMessage(message);
                    sending = false;
                } else {
                    if (message.startsWith("COMPOSE")) {
                        if (message.split(" ").length == 2) {
                            sendMessage(message);
                            sending = true;
                        } else {
                            System.out.println("Error: correct message format is COMPOSE <username>");
                            //continue;
                        }
                    } else if (message.equals("READ")) {
                        sendMessage(message);                    
                    } else if (message.equals("EXIT")) {
                        sendMessage(message);
                        break;
                    } else {
                        System.out.println("Error: unrecognized command");
                    }
                }
            }
        } catch (IOException e) {
            System.out.println("Error connecting to server");
        } finally {
            try {
                if (clientSocket != null) {
                    clientSocket.close();
                }
            } catch (IOException e) {
                System.out.println("Error closing the socket");
            }
        }
    }

    private static void sendMessage(String message) {
        out.println(message);
    }

    private static void receiveMessages() throws IOException {
        String message;
        while ((message = in.readLine()) != null) {
            System.out.println(message);
        }
    }
}
