#include "../include/client.h"

namespace poker {

    Client::Client() :
        socket_(-1),
        connected_(false) {
    }

    Client::~Client() {
        disconnect();
    }

    bool Client::connect(const std::string& serverIp, int port) {
        if (connected_) {
            return true;
        }

        // Create socket
        socket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_ == -1) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        // Set up server address
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);

        // Convert IP address from text to binary
        if (inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr) <= 0) {
            std::cerr << "Invalid address or address not supported" << std::endl;
            close(socket_);
            socket_ = -1;
            return false;
        }

        // Connect to server
        if (::connect(socket_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Connection failed to " << serverIp << ":" << port << std::endl;
            close(socket_);
            socket_ = -1;
            return false;
        }

        connected_ = true;
        std::cout << "Connected to server at " << serverIp << ":" << port << std::endl;

        // Start thread to receive messages
        receiveThread_ = std::thread(&Client::receiveMessages, this);

        return true;
    }

    void Client::disconnect() {
        if (!connected_) {
            return;
        }

        connected_ = false;

        // Close socket
        if (socket_ != -1) {
            close(socket_);
            socket_ = -1;
        }

        // Join receive thread
        if (receiveThread_.joinable()) {
            receiveThread_.join();
        }

        std::cout << "Disconnected from server" << std::endl;
    }

    bool Client::isConnected() const {
        return connected_;
    }

    bool Client::sendMessage(const std::string& message) {
        if (!connected_) {
            return false;
        }

        std::lock_guard<std::mutex> lock(sendMutex_);
        if (send(socket_, message.c_str(), message.size(), 0) < 0) {
            std::cerr << "Failed to send message to server" << std::endl;
            return false;
        }

        return true;
    }

    void Client::registerMessageHandler(const std::function<void(const std::string&)>& handler) {
        messageHandler_ = handler;
    }

    void Client::receiveMessages() {
        const int bufferSize = 1024;
        char buffer[bufferSize];

        while (connected_) {
            memset(buffer, 0, bufferSize);
            int bytesRead = recv(socket_, buffer, bufferSize - 1, 0);

            if (bytesRead <= 0) {
                if (bytesRead == 0) {
                    std::cout << "Server closed connection" << std::endl;
                } else {
                    std::cerr << "Error reading from server" << std::endl;
                }
                connected_ = false;
                break;
            }

            // Process received message
            std::string message(buffer, bytesRead);
            if (messageHandler_) {
                messageHandler_(message);
            }
        }
    }

} // namespace poker 