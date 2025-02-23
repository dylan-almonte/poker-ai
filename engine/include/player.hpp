#pragma once

#include <memory>
#include <string>
#include <vector>
#include "actions.hpp"
#include "card.hpp"
#include "pot.hpp"

class Player {
public:
    
    virtual ~Player() = default;
    Player(const std::string& name, int initial_chips, std::shared_ptr<Pot> pot);

    // Action methods
    std::unique_ptr<Action> fold();
    std::unique_ptr<Action> call(const std::vector<std::shared_ptr<Player>>& players);
    std::unique_ptr<Action> raiseTo(int chips);
    virtual std::unique_ptr<Action> takeAction(void* game_state) = 0;

    // Chip management
    void addChips(int chips);
    void addToPot(int chips);
    int tryToMakeFullBet(int chips);

    // Card management
    void addPrivateCard(std::shared_ptr<Card> card);
    const std::vector<std::shared_ptr<Card>>& getCards() const { return cards_; }

    // Getters
    const std::string& getName() const { return name_; }
    int getChips() const { return chips_; }
    int getBetChips() const { return pot_->getPlayerContribution(this); }
    std::shared_ptr<Pot> getPot() const { return pot_; }
    bool isActive() const { return is_active_; }
    bool isAllIn() const { return is_active_ && chips_ == 0; }
    int getOrder() const { return order_; }

    // Setters
    void setActive(bool active) { is_active_ = active; }
    void setOrder(int order) { order_ = order; }

private:
    std::string name_;
    int chips_;
    std::vector<std::shared_ptr<Card>> cards_;
    bool is_active_{true};
    std::shared_ptr<Pot> pot_;
    int order_{0};
}; 