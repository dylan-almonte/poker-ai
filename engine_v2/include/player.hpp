#pragma once
#include <string>
#include <memory>
#include <vector>
#include "card.hpp"
#include "player_state.hpp"




class Player {
private:
    int id_;
    std::string name_;
    int chips_;
    PlayerState state_;
    int last_pot_;
    std::vector<Card> hand_;
    int initial_stack_;

public:
    Player(int id, const std::string& name, int chips)
        : id_(id), name_(name), chips_(chips), state_(PlayerState::IN), last_pot_(0) {}

    // Getters
    int getId() const { return id_; }
    const std::string& getName() const { return name_; }
    int getChips() const { return chips_; }
    PlayerState getState() const { return state_; }
    int getLastPot() const { return last_pot_; }
    const std::vector<Card>& getHand() const { return hand_; }

    // Setters
    void setChips(int amount) { chips_ = amount; }
    void setState(PlayerState new_state) { state_ = new_state; }
    void setLastPot(int pot) { last_pot_ = pot; }
    
    // Hand management
    void addCard(const Card& card) { hand_.push_back(card); }
    void clearHand() { hand_.clear(); }

    // State checks
    bool isActive() const { return state_ == PlayerState::IN || state_ == PlayerState::TO_CALL; }
    bool isAllIn() const { return state_ == PlayerState::ALL_IN || (isActive() && chips_ == 0); }
    bool hasFolded() const { return state_ == PlayerState::OUT; }

    void setInitialStack(int stack) { initial_stack_ = stack; }
    int getInitialStack() const { return initial_stack_; }

    // Set the player's ID (used when adding to a game)
    void setId(int new_id) {
        id_ = new_id;
    }
}; 

class PlayerList {
private:
    struct PlayerNode {
        std::shared_ptr<Player> player;
        PlayerNode* next;
        PlayerNode* prev;
    };

    PlayerNode* current;
public:
    

    PlayerList(std::vector<std::shared_ptr<Player>> players, int starting_player) {
        current = new PlayerNode();
        current->player = players[starting_player];
        current->next = nullptr;
        current->prev = nullptr;
    }

    void pop() {
        auto next = current->next;
        current->next = nullptr;
        current->prev = nullptr;
        current = next;
    }

    
};

