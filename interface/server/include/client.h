#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <functional>
#include <mutex>
#include <atomic>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

namespace poker {

    class Client {
    public:
        Client();
        ~Client();

        // Connect to server
        bool connect(const std::string& serverIp, int port);

        // Disconnect from server
        void disconnect();

        // Check if connected to server
        bool isConnected() const;

        // Send message to server
        bool sendMessage(const std::string& message);

        // Register a callback for handling server messages
        void registerMessageHandler(const std::function<void(const std::string&)>& handler);

    private:
        // Handle messages from server
        void receiveMessages();

        int socket_;
        std::atomic<bool> connected_;
        std::thread receiveThread_;
        std::function<void(const std::string&)> messageHandler_;
        std::mutex sendMutex_;
    };

} // namespace poker 