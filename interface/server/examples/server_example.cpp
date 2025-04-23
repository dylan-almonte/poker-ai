#include "../include/server.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <signal.h>

using namespace poker;

// Signal handler to stop the server gracefully
volatile sig_atomic_t running = 1;
void signalHandler(int signal) {
    running = 0;
}

int main() {
    // Set up signal handling
    signal(SIGINT, signalHandler);
    
    // Create server on port 8080
    Server server(8080);
    
    // Register message handler
    server.registerMessageHandler([&server](int clientId, const std::string& message) {
        std::cout << "Received message from client " << clientId << ": " << message << std::endl;
        
        // Echo message back to client
        server.sendMessage(clientId, "Server received: " + message);
        
        // Also broadcast to all clients
        server.broadcastMessage("Client " + std::to_string(clientId) + " says: " + message);
    });
    
    // Start server
    if (!server.start()) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    std::cout << "Server started. Press Ctrl+C to stop." << std::endl;
    
    // Main loop
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "Stopping server..." << std::endl;
    server.stop();
    
    return 0;
} 