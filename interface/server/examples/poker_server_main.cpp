#include "../include/poker_server.h"
#include <iostream>
#include <signal.h>

using namespace poker;

// Signal handler to stop the server gracefully
volatile sig_atomic_t running = 1;
void signalHandler(int signal) {
    running = 0;
}

int main(int argc, char* argv[]) {
    // Set up signal handling
    signal(SIGINT, signalHandler);
    
    // Default settings
    int port = 8080;
    int minPlayers = 2;
    int maxPlayers = 9;
    int startingChips = 1000;
    
    // Parse command line arguments if provided
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }
    if (argc > 2) {
        minPlayers = std::stoi(argv[2]);
    }
    if (argc > 3) {
        maxPlayers = std::stoi(argv[3]);
    }
    if (argc > 4) {
        startingChips = std::stoi(argv[4]);
    }
    
    // Create and start the poker server
    PokerServer server(port, minPlayers, maxPlayers, startingChips);
    
    if (!server.start()) {
        std::cerr << "Failed to start poker server" << std::endl;
        return 1;
    }
    
    std::cout << "Poker server started on port " << port << std::endl;
    std::cout << "Min players: " << minPlayers << ", Max players: " << maxPlayers << std::endl;
    std::cout << "Starting chips: " << startingChips << std::endl;
    std::cout << "Press Ctrl+C to stop the server" << std::endl;
    
    // Run the game loop in a separate thread to keep main thread responsive
    std::thread gameThread([&server]() {
        server.run();
    });
    
    // Wait for signal to stop
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "Stopping poker server..." << std::endl;
    server.stop();
    
    // Join game thread
    if (gameThread.joinable()) {
        gameThread.join();
    }
    
    return 0;
} 