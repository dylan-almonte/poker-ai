#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include "player.hpp"
#include "table.hpp"
#include "evaluator.hpp"

class PokerEngine {
public:
    PokerEngine(std::shared_ptr<PokerTable> table, int small_blind, int big_blind);
    
    void playOneRound();
    void roundSetup();
    void computeWinners();

private:
    void allDealingAndBettingRounds();
    void roundCleanup();
    void assignOrderToPlayers();
    void assignBlinds();
    void bettingRound(bool first_round = false);
    void betUntilEveryoneHasEvenBets();
    void allActivePlayersTakeAction(bool first_round);
    void postBettingAnalysis();
    
    std::vector<std::vector<std::shared_ptr<Player>>> rankPlayersByBestHand();
    std::unordered_map<std::shared_ptr<Player>, int> computePayouts(
        const std::vector<std::vector<std::shared_ptr<Player>>>& ranked_player_groups);
    void payoutPlayers(const std::unordered_map<std::shared_ptr<Player>, int>& payouts);

    std::shared_ptr<PokerTable> table_;
    int small_blind_;
    int big_blind_;
    std::unique_ptr<Evaluator> evaluator_;
    std::vector<std::pair<int, int>> wins_and_losses_;

    // Helper properties
    int getNumPlayersWithMoves() const;
    int getNumActivePlayers() const;
    int getNumAllInPlayers() const;
    std::vector<int> getAllBets() const;
    bool moreBettingNeeded() const;
}; 