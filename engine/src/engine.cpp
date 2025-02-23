#include "engine.hpp"
#include <algorithm>
#include <numeric>
#include <stdexcept>

PokerEngine::PokerEngine(std::shared_ptr<PokerTable> table, int small_blind, int big_blind)
    : table_(table)
    , small_blind_(small_blind)
    , big_blind_(big_blind)
    , evaluator_(std::make_unique<Evaluator>()) {
}

void PokerEngine::playOneRound() {
    roundSetup();
    allDealingAndBettingRounds();
    computeWinners();
    roundCleanup();
}

void PokerEngine::roundSetup() {
    table_->getPot()->reset();
    assignOrderToPlayers();
    assignBlinds();
}

void PokerEngine::allDealingAndBettingRounds() {
    // Deal private cards and run first betting round
    table_->getDealer()->dealPrivateCards(table_->getPlayers());
    bettingRound(true);

    // Deal and bet on flop
    table_->getDealer()->dealFlop(table_);
    bettingRound();

    // Deal and bet on turn
    table_->getDealer()->dealTurn(table_);
    bettingRound();

    // Deal and bet on river
    table_->getDealer()->dealRiver(table_);
    bettingRound();
}

void PokerEngine::bettingRound(bool first_round) {
    if (getNumPlayersWithMoves() > 1) {
        betUntilEveryoneHasEvenBets();
    }
    postBettingAnalysis();
}

void PokerEngine::betUntilEveryoneHasEvenBets() {
    bool first_round = true;
    while (first_round || moreBettingNeeded()) {
        allActivePlayersTakeAction(first_round);
        first_round = false;
    }
}

void PokerEngine::allActivePlayersTakeAction(bool first_round) {
    auto players = first_round ? 
        std::vector<std::shared_ptr<Player>>(table_->getPlayers().begin() + 2, table_->getPlayers().end()) :
        table_->getPlayers();

    if (first_round) {
        // Add small and big blind players to end
        players.push_back(table_->getPlayers()[0]);
        players.push_back(table_->getPlayers()[1]);
    }

    for (auto& player : players) {
        if (player->isActive()) {
            player->takeAction(nullptr); // TODO: Pass game state
        }
    }
}

void PokerEngine::assignBlinds() {
    table_->getPlayers()[0]->addToPot(small_blind_);
    table_->getPlayers()[1]->addToPot(big_blind_);
}

void PokerEngine::assignOrderToPlayers() {
    for (size_t i = 0; i < table_->getPlayers().size(); i++) {
        table_->getPlayers()[i]->setOrder(i);
    }
}

void PokerEngine::roundCleanup() {
    // Rotate players for next round
    auto players = table_->getPlayers();
    std::rotate(players.begin(), players.begin() + 1, players.end());
    table_->setPlayers(players);
}

int PokerEngine::getNumPlayersWithMoves() const {
    return std::count_if(table_->getPlayers().begin(), table_->getPlayers().end(),
        [](const auto& player) { 
            return player->isActive() && !player->isAllIn(); 
        });
}

int PokerEngine::getNumActivePlayers() const {
    return std::count_if(table_->getPlayers().begin(), table_->getPlayers().end(),
        [](const auto& player) { return player->isActive(); });
}

int PokerEngine::getNumAllInPlayers() const {
    return std::count_if(table_->getPlayers().begin(), table_->getPlayers().end(),
        [](const auto& player) { 
            return player->isActive() && player->isAllIn(); 
        });
}

std::vector<int> PokerEngine::getAllBets() const {
    std::vector<int> bets;
    for (const auto& player : table_->getPlayers()) {
        bets.push_back(player->getBetChips());
    }
    return bets;
}

bool PokerEngine::moreBettingNeeded() const {
    std::vector<int> active_bets;
    for (const auto& player : table_->getPlayers()) {
        if (player->isActive() && !player->isAllIn()) {
            active_bets.push_back(player->getBetChips());
        }
    }
    
    if (active_bets.empty()) return false;
    
    return !std::all_of(active_bets.begin(), active_bets.end(),
        [first = active_bets[0]](int bet) { return bet == first; });
} 