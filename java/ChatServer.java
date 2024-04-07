import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

public class ChatServer {
    private static final String HOST = "127.0.0.1";
    private static Map<String, Queue<Map<String, String>>> messages = new ConcurrentHashMap<>();

    public static void main(String[] args) {
        if (args.length != 1) {
            System.out.println("Usage: java ChatServer <port number>");
            return;
        }
        
        int port = Integer.parseInt(args[0]);
        try (ServerSocket serverSocket = new ServerSocket(port)) {
            System.out.println("Server listening on " + HOST + ":" + port);
            
            while (true) {
                Socket clientSocket = serverSocket.accept();
                new ClientHandler(clientSocket).start();
            }
        } catch (IOException e) {
            System.out.println("Server exception: " + e.getMessage());
        }
    }

    private static class ClientHandler extends Thread {
        private Socket clientSocket;
        
        public ClientHandler(Socket socket) {
            this.clientSocket = socket;
        }

        @Override
        public void run() {
            String currentUser = null;
            boolean receiving = false;
            String toUser = null;

            try (BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                 PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true)) {

                out.println("You are connected to the server.");
                String inputLine;

                while ((inputLine = in.readLine()) != null) {
                    System.out.println("Received: " + inputLine);
                    if (currentUser == null) {
                        if (inputLine.startsWith("LOGIN") && inputLine.split(" ").length == 2) {
                            currentUser = inputLine.split(" ")[1];
                            messages.putIfAbsent(currentUser, new LinkedList<>());
                            out.println(messages.get(currentUser).size());
                        } else {
                            break;
                        }
                    } else {
                        if (receiving) {
                            Map<String, String> message = new HashMap<>();
                            message.put("from", currentUser);
                            message.put("message", inputLine);
                            messages.putIfAbsent(toUser, new LinkedList<>());                            
                            messages.get(toUser).add(message);
                            receiving = false;
                            out.println("MESSAGE SENT");
                        } else {
                            if (inputLine.startsWith("READ")) {
                                Queue<Map<String, String>> userMessages = messages.get(currentUser);
                                if (userMessages != null && !userMessages.isEmpty()) {
                                    Map<String, String> message = userMessages.poll();
                                    out.println(message.get("from"));
                                    out.println(message.get("message"));
                                } else {
                                    out.println("READ ERROR");
                                }
                            } else if (inputLine.startsWith("COMPOSE") && inputLine.split(" ").length == 2) {
                                toUser = inputLine.split(" ")[1];
                                receiving = true;
                            } else if (inputLine.equals("EXIT")) {
                                break;
                            } else {
                                break;
                            }
                        }
                    }
                }
            } catch (IOException e) {
                System.out.println("Exception caught when trying to listen on port or listening for a connection");
                System.out.println(e.getMessage());
            } finally {
                try {
                    clientSocket.close();
                } catch (IOException e) {
                    System.out.println("Could not close the client socket");
                    System.out.println(e.getMessage());
                }
            }
        }
    }
}
