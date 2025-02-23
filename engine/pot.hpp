#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Player;

class Pot {
public:
    Pot();

    // Chip management
    void addChips(Player* player, int chips);
    void reset();
    int getPlayerContribution(Player* player) const;
    int getTotal() const;

    // Side pot calculation
    std::vector<std::unordered_map<Player*, int>> getSidePots() const;

    // Unique identifier
    const std::string& getUid() const { return uid_; }

private:
    std::unordered_map<Player*, int> pot_;
    std::string uid_;
}; 