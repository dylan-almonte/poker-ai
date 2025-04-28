#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <functional>
#include <unordered_map>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

namespace poker {

class Server {
    public:
        Server(int port = 8080);
        ~Server();

        // Start the server
        bool start();

        // Stop the server
        void stop();

        // Check if server is running
        bool isRunning() const;

        // Register a callback for handling client messages
        void registerMessageHandler(const std::function<void(int, const std::string&)>& handler);

        // Send message to a specific client
        bool sendMessage(int clientId, const std::string& message);

        // Broadcast message to all clients
        void broadcastMessage(const std::string& message);

    private:
        // Accept client connections
        void acceptClients();

        // Handle client communication
        void handleClient(int clientSocket, int clientId);

        int port_;
        int serverSocket_;
        bool running_;
        std::mutex clientsMutex_;
        std::unordered_map<int, int> clients_; // clientId -> socket
        std::vector<std::thread> clientThreads_;
        std::thread acceptThread_;
        std::function<void(int, const std::string&)> messageHandler_;
        int nextClientId_;
    };

} // namespace poker 