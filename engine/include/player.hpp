#pragma once
#include <string>
#include <memory>
#include <vector>
#include "card.hpp"
#include "player_state.hpp"

class Player {
private:
    int id;
    std::string name;
    int chips;
    PlayerState state;
    int last_pot;
    std::vector<Card> hand;

public:
    Player(int id, const std::string& name, int chips)
        : id(id), name(name), chips(chips), state(PlayerState::IN), last_pot(0) {}

    // Getters
    int getId() const { return id; }
    const std::string& getName() const { return name; }
    int getChips() const { return chips; }
    PlayerState getState() const { return state; }
    int getLastPot() const { return last_pot; }
    const std::vector<Card>& getHand() const { return hand; }

    // Setters
    void setChips(int amount) { chips = amount; }
    void setState(PlayerState new_state) { state = new_state; }
    void setLastPot(int pot) { last_pot = pot; }
    
    // Hand management
    void addCard(const Card& card) { hand.push_back(card); }
    void clearHand() { hand.clear(); }

    // State checks
    bool isActive() const { return state == PlayerState::IN || state == PlayerState::TO_CALL; }
    bool isAllIn() const { return state == PlayerState::ALL_IN; }
    bool hasFolded() const { return state == PlayerState::OUT; }
}; 