#pragma once

#include "server.h"
#include <vector>
#include <string>
#include <map>
#include <nlohmann/json.hpp>

// Forward declarations for poker engine classes
namespace poker {
    class Game;
    class Player;
}

namespace poker {

    // Player state in the poker game
    struct PlayerState {
        int id;
        std::string name;
        int chips;
        bool isConnected;
        bool isActive;
        bool isReady;
    };

    // Game stage
    enum class GameStage {
        WAITING_FOR_PLAYERS,
        READY_TO_START,
        PRE_FLOP,
        FLOP,
        TURN,
        RIVER,
        SHOWDOWN,
        GAME_OVER
    };

    class PokerServer {
    public:
        PokerServer(int port = 8080, int minPlayers = 2, int maxPlayers = 9, int startingChips = 1000);
        ~PokerServer();

        // Start the server
        bool start();

        // Stop the server
        void stop();

        // Run the game loop
        void run();

    private:
        // Process messages from clients
        void handleMessage(int clientId, const std::string& message);

        // Parse JSON message
        nlohmann::json parseMessage(const std::string& message);

        // Handle different message types
        void handleRegisterPlayer(int clientId, const nlohmann::json& json);
        void handlePlayerAction(int clientId, const nlohmann::json& json);
        void handleReadyState(int clientId, const nlohmann::json& json);

        // Game logic
        void startGame();
        void endGame();
        void nextRound();
        void nextPlayer();

        // Send game state updates
        void broadcastGameState();
        void sendPlayerState(int clientId);
        void sendPrivateCards(int clientId);
        void broadcastPublicCards();
        void broadcastWinners(const std::vector<int>& winnerIds);

        // Utility methods
        std::string serializeGameState();

        Server server_;
        int minPlayers_;
        int maxPlayers_;
        int startingChips_;
        GameStage gameStage_;

        std::map<int, PlayerState> players_;
        int currentPlayerTurn_;
        int dealerPosition_;

        // Poker game engine instance
        std::unique_ptr<Game> game_;
    };

} // namespace poker 