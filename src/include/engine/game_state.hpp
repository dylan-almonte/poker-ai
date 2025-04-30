#pragma once
#include <memory>
#include <vector>
#include <string>
#include "player.hpp"
#include "pot.hpp"
#include "deck.hpp"
#include "hand_phase.hpp"
#include "action.hpp"
#include "player_state.hpp"
#include "betting/betting_state.hpp"
#include <list>
#include <iostream>

// Game phases
enum class GamePhase {
    PREHAND,
    BETTING_ROUND,
    SHOWDOWN,
    HAND_COMPLETE
};

// Betting phases
enum class BettingPhase {
    PREFLOP,
    FLOP,
    TURN,
    RIVER
};

struct GameState {
    bool is_terminal = false;
    size_t num_players{};
    size_t num_pots{};
    size_t current_player{};
    HandPhase::Phase street{};
    std::vector<PlayerState> player_states{};
    std::vector<int> player_chips{};
    std::vector<int> player_bets{};
    std::vector<int> player_rewards{};
    std::vector<Pot> pots{};
    std::vector<Card> board{ 5, Card(-1) };
    std::pair<Card, Card> hole_cards{ Card(-1), Card(-1) };
    HandPhase::Phase hand_phase{ HandPhase::Phase::PREFLOP };


    void print() const {
        std::cout << "GameState:" << std::endl;
        std::cout << "  is_terminal: " << is_terminal << std::endl;
        std::cout << "  num_players: " << num_players << std::endl;
        std::cout << "  num_pots: " << num_pots << std::endl;
        std::cout << "  street: " << phaseToString(street) << std::endl;
        std::cout << std::endl;
        std::cout << "  current_player: " << current_player << std::endl;
        std::cout << "  hole_cards: " << hole_cards.first.toString() << " " << hole_cards.second.toString() << std::endl;
        std::cout << "  board: ";
        for (const auto& card : board) {
            std::cout << card.toString() << " ";
        }
        std::cout << std::endl;
        std::cout << std::endl;

        std::cout << "  player_chips: ";
        for (const auto& chip : player_chips) {
            std::cout << chip << " ";
        }
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "  players: " << std::endl;
        for (size_t i = 0; i < player_bets.size(); i++) {
            std::cout << "    player " << i << ": " << std::endl;
            std::cout << "\tchips: " << player_chips[i] << std::endl;
            std::cout << "\tbets: " << player_bets[i] << std::endl;
            std::cout << "\tstate: " << playerStateToString(player_states[i]) << std::endl;
            std::cout << "\treward: " << player_rewards[i] << std::endl;
        }

        std::cout << std::endl;
        std::cout << "  pots: ";
        for (const auto& pot : pots) {
            std::cout << pot.get_total_amount() << " ";
        }
        std::cout << std::endl;

    }
};

// Base class for all game states
class GameStateMachine {
private:
    std::unique_ptr<GameState> current_state_;
    std::vector<std::shared_ptr<Player>> players_;
    std::vector<std::shared_ptr<Pot>> pots_;
    std::vector<Card> board_;
    GamePhase current_phase_;
    BettingPhase current_betting_phase_;
    int button_position_;
    int small_blind_;
    int big_blind_;

public:
    GameStateMachine(std::vector<std::shared_ptr<Player>> players,
                    int small_blind,
                    int big_blind);

    // Transition to a new state
    void transitionTo(std::unique_ptr<GameState> new_state);

    // Handle an action
    void handleAction(const Action& action);

    // Getters
    const std::vector<std::shared_ptr<Player>>& getPlayers() const { return players_; }
    const std::vector<std::shared_ptr<Pot>>& getPots() const { return pots_; }
    const std::vector<Card>& getBoard() const { return board_; }
    GamePhase getCurrentPhase() const { return current_phase_; }
    BettingPhase getCurrentBettingPhase() const { return current_betting_phase_; }
    int getButtonPosition() const { return button_position_; }
    int getSmallBlind() const { return small_blind_; }
    int getBigBlind() const { return big_blind_; }

    // Setters
    void setCurrentPhase(GamePhase phase) { current_phase_ = phase; }
    void setCurrentBettingPhase(BettingPhase phase) { current_betting_phase_ = phase; }
    void setButtonPosition(int position) { button_position_ = position; }

    // Helper methods
    void moveButton();
    void postBlinds();
    void dealHoleCards();
    void dealCommunityCards();
    void startNewHand();
    void endHand();
};

// Concrete game states
class PrehandState : public GameState {
private:
    GameStateMachine& machine_;

public:
    explicit PrehandState(GameStateMachine& machine) : machine_(machine) {}

    void enter() override;
    void exit() override;
    void handleAction(const Action& action) override;
    std::string getName() const override { return "Prehand"; }
};

class BettingRoundState : public GameState {
private:
    GameStateMachine& machine_;
    std::unique_ptr<BettingStateMachine> betting_machine_;

public:
    explicit BettingRoundState(GameStateMachine& machine) : machine_(machine) {}

    void enter() override;
    void exit() override;
    void handleAction(const Action& action) override;
    std::string getName() const override { return "BettingRound"; }
};

class ShowdownState : public GameState {
private:
    GameStateMachine& machine_;

public:
    explicit ShowdownState(GameStateMachine& machine) : machine_(machine) {}

    void enter() override;
    void exit() override;
    void handleAction(const Action& action) override;
    std::string getName() const override { return "Showdown"; }
};

class HandCompleteState : public GameState {
private:
    GameStateMachine& machine_;

public:
    explicit HandCompleteState(GameStateMachine& machine) : machine_(machine) {}

    void enter() override;
    void exit() override;
    void handleAction(const Action& action) override;
    std::string getName() const override { return "HandComplete"; }
};