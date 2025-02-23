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

void PokerEngine::computeWinners() {
    auto rankedPlayerGroups = rankPlayersByBestHand();
    auto payouts = computePayouts(rankedPlayerGroups);
    payoutPlayers(payouts);
}

void PokerEngine::postBettingAnalysis() {
    // Verify total chips in play remain constant
    int totalChipsInPot = table_->getPot()->getTotal();
    int totalPlayerChips = std::accumulate(table_->getPlayers().begin(), 
                                         table_->getPlayers().end(), 
                                         0,
                                         [](int sum, const auto& player) {
                                             return sum + player->getChips();
                                         });
    
    int totalBets = std::accumulate(table_->getPlayers().begin(),
                                   table_->getPlayers().end(),
                                   0,
                                   [](int sum, const auto& player) {
                                       return sum + player->getBetChips();
                                   });

    // Verify pot matches total bets
    if (totalBets != table_->getPot()->getTotal()) {
        throw std::runtime_error("Invalid state: Pot total does not match sum of bets");
    }

    // Verify total chips haven't changed
    if (totalChipsInPot + totalPlayerChips != table_->getTotalChips()) {
        throw std::runtime_error("Invalid state: Total chips in play has changed");
    }
}

std::vector<std::vector<std::shared_ptr<Player>>> PokerEngine::rankPlayersByBestHand() {
    std::map<int, std::vector<std::shared_ptr<Player>>> groupedPlayers;
    
    // Get community cards in evaluator format
    std::vector<int32_t> tableCards;
    for (const auto& card : table_->getCommunityCards()) {
        tableCards.push_back(card->getEvalCard());
    }

    // Evaluate each active player's hand
    for (const auto& player : table_->getPlayers()) {
        if (player->isActive()) {
            std::vector<int32_t> handCards;
            for (const auto& card : player->getCards()) {
                handCards.push_back(card->getEvalCard());
            }
            
            int rank = evaluator_->evaluate(tableCards, handCards);
            int handClass = evaluator_->getRankClass(rank);
            groupedPlayers[rank].push_back(player);
        }
    }

    // Convert map to vector of player groups, sorted by rank
    std::vector<std::vector<std::shared_ptr<Player>>> rankedPlayerGroups;
    for (const auto& [rank, players] : groupedPlayers) {
        rankedPlayerGroups.push_back(players);
    }
    
    return rankedPlayerGroups;
}

std::map<std::shared_ptr<Player>, int> PokerEngine::computePayouts(
    const std::vector<std::vector<std::shared_ptr<Player>>>& rankedPlayerGroups) {
    
    std::map<std::shared_ptr<Player>, int> payouts;
    
    for (const auto& sidePot : table_->getPot()->getSidePots()) {
        for (const auto& playerGroup : rankedPlayerGroups) {
            auto potPayouts = processSidePot(playerGroup, sidePot);
            if (!potPayouts.empty()) {
                // Merge pot payouts into total payouts
                for (const auto& [player, amount] : potPayouts) {
                    payouts[player] += amount;
                }
                break;  // Move to next side pot once we've found winners
            }
        }
    }
    
    return payouts;
}

void PokerEngine::payoutPlayers(const std::map<std::shared_ptr<Player>, int>& payouts) {
    table_->getPot()->reset();
    for (const auto& [player, winnings] : payouts) {
        player->addChips(winnings);
    }
}

std::map<std::shared_ptr<Player>, int> PokerEngine::processSidePot(
    const std::vector<std::shared_ptr<Player>>& playerGroup,
    const std::map<std::shared_ptr<Player>, int>& pot) {
    
    std::map<std::shared_ptr<Player>, int> payouts;
    
    // Get players in this pot, ordered by their position
    std::vector<std::shared_ptr<Player>> playersInPot;
    for (const auto& player : playerGroup) {
        if (pot.find(player) != pot.end()) {
            playersInPot.push_back(player);
        }
    }
    
    // Sort players by their order
    std::sort(playersInPot.begin(), playersInPot.end(),
              [](const auto& a, const auto& b) {
                  return a->getOrder() < b->getOrder();
              });
    
    if (playersInPot.empty()) {
        return payouts;
    }
    
    // Calculate total pot amount and split it
    int totalPot = std::accumulate(pot.begin(), pot.end(), 0,
                                 [](int sum, const auto& p) {
                                     return sum + p.second;
                                 });
    
    int numPlayers = playersInPot.size();
    int amountPerPlayer = totalPot / numPlayers;
    int remainder = totalPot % numPlayers;
    
    // Distribute the pot
    for (const auto& player : playersInPot) {
        payouts[player] = amountPerPlayer;
    }
    
    // Distribute remainder chips to players in order
    for (int i = 0; i < remainder; i++) {
        payouts[playersInPot[i]]++;
    }
    
    return payouts;
} 