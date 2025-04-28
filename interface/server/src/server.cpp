#include "server.h"

namespace poker {

    Server::Server(int port) :
        port_(port),
        serverSocket_(-1),
        running_(false),
        nextClientId_(1) {
    }

    Server::~Server() {
        stop();
    }

    bool Server::start() {
        // Create socket
        serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket_ == -1) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        // Set socket options
        int opt = 1;
        if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
            std::cerr << "Failed to set socket options" << std::endl;
            close(serverSocket_);
            return false;
        }

        // Bind socket to port
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);

        if (bind(serverSocket_, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "Failed to bind socket to port " << port_ << std::endl;
            close(serverSocket_);
            return false;
        }

        // Listen for connections
        if (listen(serverSocket_, 5) < 0) {
            std::cerr << "Failed to listen for connections" << std::endl;
            close(serverSocket_);
            return false;
        }

        running_ = true;
        std::cout << "Server started on port " << port_ << std::endl;

        // Start thread to accept clients
        acceptThread_ = std::thread(&Server::acceptClients, this);

        return true;
    }

    void Server::stop() {
        if (!running_) {
            return;
        }

        running_ = false;

        // Close server socket
        if (serverSocket_ != -1) {
            close(serverSocket_);
            serverSocket_ = -1;
        }

        // Close all client sockets
        {
            std::lock_guard<std::mutex> lock(clientsMutex_);
            for (const auto& [clientId, clientSocket] : clients_) {
                close(clientSocket);
            }
            clients_.clear();
        }

        // Join client threads
        for (auto& thread : clientThreads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        clientThreads_.clear();

        // Join accept thread
        if (acceptThread_.joinable()) {
            acceptThread_.join();
        }

        std::cout << "Server stopped" << std::endl;
    }

    bool Server::isRunning() const {
        return running_;
    }

    void Server::registerMessageHandler(const std::function<void(int, const std::string&)>& handler) {
        messageHandler_ = handler;
    }

    bool Server::sendMessage(int clientId, const std::string& message) {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        auto it = clients_.find(clientId);
        if (it == clients_.end()) {
            return false;
        }

        int clientSocket = it->second;
        if (send(clientSocket, message.c_str(), message.size(), 0) < 0) {
            std::cerr << "Failed to send message to client " << clientId << std::endl;
            return false;
        }

        return true;
    }

    void Server::broadcastMessage(const std::string& message) {
        std::lock_guard<std::mutex> lock(clientsMutex_);
        for (const auto& [clientId, clientSocket] : clients_) {
            if (send(clientSocket, message.c_str(), message.size(), 0) < 0) {
                std::cerr << "Failed to broadcast message to client " << clientId << std::endl;
            }
        }
    }

    void Server::acceptClients() {
        struct sockaddr_in clientAddr;
        socklen_t addrlen = sizeof(clientAddr);

        while (running_) {
            int clientSocket = accept(serverSocket_, (struct sockaddr*)&clientAddr, &addrlen);
            if (clientSocket < 0) {
                if (running_) {
                    std::cerr << "Failed to accept client connection" << std::endl;
                }
                continue;
            }

            int clientId;
            {
                std::lock_guard<std::mutex> lock(clientsMutex_);
                clientId = nextClientId_++;
                clients_[clientId] = clientSocket;
            }

            std::cout << "Player " << clientId << " connected from "
                << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;

            // Start thread to handle client
            clientThreads_.emplace_back(&Server::handleClient, this, clientSocket, clientId);
        }
    }

    void Server::handleClient(int clientSocket, int clientId) {
        const int bufferSize = 1024;
        char buffer[bufferSize];

        // Send welcome message with player ID
        std::string welcomeMsg = "{\"type\":\"welcome\",\"player_id\":" + std::to_string(clientId) + "}";
        send(clientSocket, welcomeMsg.c_str(), welcomeMsg.size(), 0);

        while (running_) {
            memset(buffer, 0, bufferSize);
            int bytesRead = recv(clientSocket, buffer, bufferSize - 1, 0);

            if (bytesRead <= 0) {
                if (bytesRead == 0) {
                    std::cout << "Player " << clientId << " disconnected" << std::endl;
                } else {
                    std::cerr << "Error reading from player " << clientId << std::endl;
                }

                // Remove client
                {
                    std::lock_guard<std::mutex> lock(clientsMutex_);
                    clients_.erase(clientId);
                }

                close(clientSocket);
                break;
            }

            // Process received message
            std::string message(buffer, bytesRead);
            if (messageHandler_) {
                messageHandler_(clientId, message);
            }
        }
    }

} // namespace poker 