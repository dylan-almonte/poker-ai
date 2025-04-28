#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>
#include "server/poker_server.h"



using json = nlohmann::json;

namespace poker {

    PokerServer::PokerServer(int port, int minPlayers, int maxPlayers, int startingChips)
        : server_(port),
        minPlayers_(minPlayers),
        maxPlayers_(maxPlayers),
        startingChips_(startingChips),
        gameStage_(GameStage::WAITING_FOR_PLAYERS),
        currentPlayerTurn_(-1),
        dealerPosition_(-1) {

        // Register message handler
        server_.registerMessageHandler([this](int clientId, const std::string& message) {
            this->handleMessage(clientId, message);
        });
    }

    PokerServer::~PokerServer() {
        stop();
    }

    bool PokerServer::start() {
        return server_.start();
    }

    void PokerServer::stop() {
        server_.stop();

        // Clean up game resources
        game_.reset();
    }

    void PokerServer::run() {
        while (server_.isRunning()) {
            // Main game loop
            if (gameStage_ == GameStage::WAITING_FOR_PLAYERS) {
                // Check if we have enough players ready to start
                int readyPlayers = 0;
                for (const auto& [id, player] : players_) {
                    if (player.isReady) {
                        readyPlayers++;
                    }
                }

                if (readyPlayers >= minPlayers_) {
                    gameStage_ = GameStage::READY_TO_START;
                    startGame();
                }
            }

            // Sleep to avoid high CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void PokerServer::handleMessage(int clientId, const std::string& message) {
        try {
            json jsonMsg = parseMessage(message);

            std::string type = jsonMsg["type"];

            if (type == "register") {
                handleRegisterPlayer(clientId, jsonMsg);
            } else if (type == "action") {
                handlePlayerAction(clientId, jsonMsg);
            } else if (type == "ready") {
                handleReadyState(clientId, jsonMsg);
            } else {
                // Unknown message type
                std::cerr << "Unknown message type: " << type << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error handling message from client " << clientId << ": " << e.what() << std::endl;
        }
    }

    json PokerServer::parseMessage(const std::string& message) {
        return json::parse(message);
    }

    void PokerServer::handleRegisterPlayer(int clientId, const json& jsonMsg) {
        // Add player to the game
        if (players_.size() >= maxPlayers_) {
            // Game is full
            json response = {
                {"type", "error"},
                {"message", "Game is full"}
            };
            server_.sendMessage(clientId, response.dump());
            return;
        }

        std::string name = jsonMsg["name"];

        PlayerState player;
        player.id = clientId;
        player.name = name;
        player.chips = startingChips_;
        player.isConnected = true;
        player.isActive = true;
        player.isReady = false;

        players_[clientId] = player;

        // Send confirmation
        json response = {
            {"type", "registered"},
            {"player_id", clientId},
            {"name", name},
            {"chips", startingChips_}
        };
        server_.sendMessage(clientId, response.dump());

        // Broadcast updated player list
        broadcastGameState();
    }

    void PokerServer::handlePlayerAction(int clientId, const json& jsonMsg) {
        // Ensure game is in progress
        if (gameStage_ == GameStage::WAITING_FOR_PLAYERS || gameStage_ == GameStage::READY_TO_START ||
            gameStage_ == GameStage::GAME_OVER) {
            json response = {
                {"type", "error"},
                {"message", "Game is not in progress"}
            };
            server_.sendMessage(clientId, response.dump());
            return;
        }

        // Ensure it's the player's turn
        if (currentPlayerTurn_ != clientId) {
            json response = {
                {"type", "error"},
                {"message", "Not your turn"}
            };
            server_.sendMessage(clientId, response.dump());
            return;
        }

        // Process the action
        std::string action = jsonMsg["action"];

        // Handle different actions (call, raise, fold, etc.)
        if (action == "fold") {
            // Handle fold action
            // game_->fold(clientId);
        } else if (action == "call") {
            // Handle call action
            // game_->call(clientId);
        } else if (action == "raise") {
            int amount = jsonMsg["amount"];
            // Handle raise action
            // game_->raise(clientId, amount);
        } else if (action == "check") {
            // Handle check action
            // game_->check(clientId);
        } else {
            // Unknown action
            json response = {
                {"type", "error"},
                {"message", "Unknown action: " + action}
            };
            server_.sendMessage(clientId, response.dump());
            return;
        }

        // Move to next player or next round
        nextPlayer();

        // Update game state
        broadcastGameState();
    }

    void PokerServer::handleReadyState(int clientId, const json& jsonMsg) {
        auto it = players_.find(clientId);
        if (it == players_.end()) {
            json response = {
                {"type", "error"},
                {"message", "Player not registered"}
            };
            server_.sendMessage(clientId, response.dump());
            return;
        }

        bool ready = jsonMsg["ready"];
        it->second.isReady = ready;

        // Broadcast updated player list
        broadcastGameState();
    }

    void PokerServer::startGame() {
        std::cout << "Starting a new game with " << players_.size() << " players" << std::endl;

        // Create a new game instance
        // game_ = std::make_unique<Game>();

        // Set up the game
        std::vector<int> activePlayers;
        for (const auto& [id, player] : players_) {
            if (player.isReady) {
                activePlayers.push_back(id);
                // game_->addPlayer(id, player.name, player.chips);
            }
        }

        // Set dealer position (rotate each game)
        if (dealerPosition_ == -1 || dealerPosition_ >= activePlayers.size()) {
            dealerPosition_ = 0;
        } else {
            dealerPosition_ = (dealerPosition_ + 1) % activePlayers.size();
        }

        // Start with the player after the dealer
        currentPlayerTurn_ = (dealerPosition_ + 1) % activePlayers.size();

        // Deal cards
        // game_->dealCards();

        // Update game stage
        gameStage_ = GameStage::PRE_FLOP;

        // Send private cards to each player
        for (const auto& [id, player] : players_) {
            if (player.isReady) {
                sendPrivateCards(id);
            }
        }

        // Broadcast game state
        broadcastGameState();
    }

    void PokerServer::endGame() {
        std::cout << "Game over" << std::endl;

        // Determine winners
        std::vector<int> winnerIds;
        // winnerIds = game_->determineWinners();

        // Broadcast winners
        broadcastWinners(winnerIds);

        // Update game stage
        gameStage_ = GameStage::WAITING_FOR_PLAYERS;

        // Reset game resources
        game_.reset();

        // Reset ready state for next game
        for (auto& [id, player] : players_) {
            player.isReady = false;
        }

        // Broadcast updated game state
        broadcastGameState();
    }

    void PokerServer::nextRound() {
        // Move to the next stage of the game
        switch (gameStage_) {
        case GameStage::PRE_FLOP:
            gameStage_ = GameStage::FLOP;
            // game_->dealFlop();
            break;
        case GameStage::FLOP:
            gameStage_ = GameStage::TURN;
            // game_->dealTurn();
            break;
        case GameStage::TURN:
            gameStage_ = GameStage::RIVER;
            // game_->dealRiver();
            break;
        case GameStage::RIVER:
            gameStage_ = GameStage::SHOWDOWN;
            // Handle showdown
            endGame();
            return;
        default:
            break;
        }

        // Broadcast public cards
        broadcastPublicCards();

        // Reset to first player
        // currentPlayerTurn_ = game_->getNextPlayerToAct();

        // Broadcast game state
        broadcastGameState();
    }

    void PokerServer::nextPlayer() {
        // Check if round is complete
        // if (game_->isRoundComplete()) {
        //     nextRound();
        //     return;
        // }

        // Move to next player
        // currentPlayerTurn_ = game_->getNextPlayerToAct();

        // For now, just cycle through active players
        std::vector<int> activePlayers;
        for (const auto& [id, player] : players_) {
            if (player.isReady && player.isActive) {
                activePlayers.push_back(id);
            }
        }

        if (activePlayers.empty()) {
            endGame();
            return;
        }

        auto it = std::find(activePlayers.begin(), activePlayers.end(), currentPlayerTurn_);
        if (it != activePlayers.end() && it + 1 != activePlayers.end()) {
            currentPlayerTurn_ = *(it + 1);
        } else {
            // Cycle back to first player or move to next round
            // For now, just cycle back to first player
            currentPlayerTurn_ = activePlayers[0];
            // nextRound();
        }
    }

    void PokerServer::broadcastGameState() {
        std::string gameStateJson = serializeGameState();
        server_.broadcastMessage(gameStateJson);
    }

    void PokerServer::sendPlayerState(int clientId) {
        auto it = players_.find(clientId);
        if (it == players_.end()) {
            return;
        }

        // Create player state update
        const PlayerState& player = it->second;
        json playerState = {
            {"type", "player_state"},
            {"player_id", player.id},
            {"name", player.name},
            {"chips", player.chips},
            {"is_active", player.isActive},
            {"is_ready", player.isReady}
        };

        // Send to the specific player
        server_.sendMessage(clientId, playerState.dump());
    }

    void PokerServer::sendPrivateCards(int clientId) {
        // Get player's cards from the game
        std::vector<std::string> cards;
        // cards = game_->getPlayerCards(clientId);

        // Mock cards for now
        cards = { "AH", "KS" };

        // Create cards update
        json cardsUpdate = {
            {"type", "private_cards"},
            {"cards", cards}
        };

        // Send to the specific player
        server_.sendMessage(clientId, cardsUpdate.dump());
    }

    void PokerServer::broadcastPublicCards() {
        // Get public cards from the game
        std::vector<std::string> publicCards;
        // publicCards = game_->getPublicCards();

        // Mock cards for now
        switch (gameStage_) {
        case GameStage::FLOP:
            publicCards = { "2H", "7C", "TD" };
            break;
        case GameStage::TURN:
            publicCards = { "2H", "7C", "TD", "AS" };
            break;
        case GameStage::RIVER:
            publicCards = { "2H", "7C", "TD", "AS", "QH" };
            break;
        default:
            break;
        }

        // Create public cards update
        json publicCardsUpdate = {
            {"type", "public_cards"},
            {"cards", publicCards}
        };

        // Broadcast to all players
        server_.broadcastMessage(publicCardsUpdate.dump());
    }

    void PokerServer::broadcastWinners(const std::vector<int>& winnerIds) {
        // Create winners update
        json winnersUpdate = {
            {"type", "game_result"},
            {"winners", winnerIds}
        };

        // Broadcast to all players
        server_.broadcastMessage(winnersUpdate.dump());
    }

    std::string PokerServer::serializeGameState() {
        // Create a JSON object with the game state
        json gameState = {
            {"type", "game_state"},
            {"stage", static_cast<int>(gameStage_)},
            {"current_player", currentPlayerTurn_},
            {"dealer_position", dealerPosition_},
            {"players", json::array()}
        };

        // Add player information
        for (const auto& [id, player] : players_) {
            json playerJson = {
                {"id", player.id},
                {"name", player.name},
                {"chips", player.chips},
                {"is_connected", player.isConnected},
                {"is_active", player.isActive},
                {"is_ready", player.isReady}
            };
            gameState["players"].push_back(playerJson);
        }

        // Add pot information if game is in progress
        if (gameStage_ != GameStage::WAITING_FOR_PLAYERS && gameStage_ != GameStage::READY_TO_START) {
            gameState["pot"] = 0; // Replace with actual pot value from the game
            // gameState["pot"] = game_->getPot();
        }

        return gameState.dump();
    }

} // namespace poker 