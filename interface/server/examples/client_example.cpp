#include "../include/client.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <signal.h>

using namespace poker;

// Signal handler to stop the client gracefully
volatile sig_atomic_t running = 1;
void signalHandler(int signal) {
    running = 0;
}

int main(int argc, char* argv[]) {
    // Set up signal handling
    signal(SIGINT, signalHandler);
    
    // Check command line arguments
    std::string serverIp = "127.0.0.1";  // Default to localhost
    int port = 8080;                     // Default port
    
    if (argc > 1) {
        serverIp = argv[1];
    }
    if (argc > 2) {
        port = std::stoi(argv[2]);
    }
    
    // Create client
    Client client;
    
    // Register message handler
    client.registerMessageHandler([](const std::string& message) {
        std::cout << "Received from server: " << message << std::endl;
    });
    
    // Connect to server
    if (!client.connect(serverIp, port)) {
        std::cerr << "Failed to connect to server at " << serverIp << ":" << port << std::endl;
        return 1;
    }
    
    std::cout << "Connected to server at " << serverIp << ":" << port << std::endl;
    std::cout << "Type messages to send to the server. Press Ctrl+C to exit." << std::endl;
    
    // Send an initial message
    client.sendMessage("Hello from client!");
    
    // Input loop
    std::string input;
    while (running) {
        // Check if there's input from the user
        if (std::cin.rdbuf()->in_avail()) {
            std::getline(std::cin, input);
            if (!input.empty()) {
                if (!client.sendMessage(input)) {
                    std::cerr << "Failed to send message" << std::endl;
                    break;
                }
            }
        }
        
        // Sleep a bit to avoid hogging CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Disconnecting from server..." << std::endl;
    client.disconnect();
    
    return 0;
} 