#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <map>

class Player;

class Pot {
public:
    Pot();

    // Chip management
    void addChips(Player* player, int chips);
    void reset();
    int getPlayerContribution(const Player* player) const;
    int getTotal() const;

    // Side pot calculation
    std::vector<std::unordered_map<Player*, int>> getSidePots() const;

    // Unique identifier
    const std::string& getUid() const { return uid_; }

private:
    std::string uid_;
    std::map<Player*, int> pot_;
}; 