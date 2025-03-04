#include "info_state.hpp"
#include <sstream>

InfoState::InfoState(int player_id, 
                     const std::vector<Card>& hole_cards,
                     const std::vector<Card>& board_cards,
                     HandPhase::Phase phase,
                     const std::vector<int>& pot_sizes,
                     const std::vector<int>& player_stacks,
                     const std::vector<PlayerState>& player_states,
                     const std::vector<std::pair<int, ActionType>>& action_history)
    : player_id(player_id),
      hole_cards(hole_cards),
      board_cards(board_cards),
      phase(phase),
      pot_sizes(pot_sizes),
      player_stacks(player_stacks),
      player_states(player_states),
      action_history(action_history) {}

InfoState InfoState::fromGame(const Game& game, int player_id) {
    // Extract player's hole cards
    const auto& players = game.getPlayers();
    std::vector<Card> hole_cards = players[player_id]->getHand();
    
    // Extract board cards
    std::vector<Card> board_cards = game.getBoard();
    
    // Extract phase
    HandPhase::Phase phase = game.getPhase();
    
    // Extract pot sizes
    std::vector<int> pot_sizes;
    for (const auto& pot : game.getPots()) {
        pot_sizes.push_back(pot->get_total_amount());
    }
    
    // Extract player stacks
    std::vector<int> player_stacks;
    for (const auto& player : players) {
        player_stacks.push_back(player->getChips());
    }
    
    // Extract player states
    std::vector<PlayerState> player_states;
    for (const auto& player : players) {
        player_states.push_back(player->getState());
    }
    
    // TODO: Extract action history from the game
    // This would require the game to maintain an action history
    std::vector<std::pair<int, ActionType>> action_history;
    
    return InfoState(player_id, hole_cards, board_cards, phase, pot_sizes, 
                     player_stacks, player_states, action_history);
}

std::vector<float> InfoState::toFeatureVector() const {
    // This is a simplified feature vector for poker
    // In a real implementation, you would want a more comprehensive representation
    std::vector<float> features;
    
    // One-hot encode player ID
    for (int i = 0; i < 6; i++) {  // Assuming max 6 players
        features.push_back(i == player_id ? 1.0f : 0.0f);
    }
    
    // Encode hole cards (one-hot for rank and suit)
    for (const auto& card : hole_cards) {
        // One-hot for rank (13 ranks)
        for (int i = 0; i < 13; i++) {
            features.push_back(i == card.getRank() ? 1.0f : 0.0f);
        }
        
        // One-hot for suit (4 suits)
        for (int i = 1; i <= 8; i *= 2) {
            features.push_back(i == card.getSuit() ? 1.0f : 0.0f);
        }
    }
    
    // Encode board cards
    for (int i = 0; i < 5; i++) {  // Max 5 board cards
        if (i < board_cards.size()) {
            const auto& card = board_cards[i];
            
            // One-hot for rank
            for (int j = 0; j < 13; j++) {
                features.push_back(j == card.getRank() ? 1.0f : 0.0f);
            }
            
            // One-hot for suit
            for (int j = 1; j <= 8; j *= 2) {
                features.push_back(j == card.getSuit() ? 1.0f : 0.0f);
            }
        } else {
            // Padding for missing cards
            features.insert(features.end(), 13 + 4, 0.0f);
        }
    }
    
    // Encode phase (one-hot)
    for (int i = 0; i < 6; i++) {  // 6 phases
        features.push_back(i == static_cast<int>(phase) ? 1.0f : 0.0f);
    }
    
    // Encode pot sizes (normalized)
    for (int pot : pot_sizes) {
        features.push_back(static_cast<float>(pot) / 10000.0f);  // Normalize by max pot
    }
    
    // Encode player stacks (normalized)
    for (int stack : player_stacks) {
        features.push_back(static_cast<float>(stack) / 10000.0f);  // Normalize by max stack
    }
    
    // Encode player states (one-hot)
    for (const auto& state : player_states) {
        for (int i = 0; i < 5; i++) {  // 5 player states
            features.push_back(i == static_cast<int>(state) ? 1.0f : 0.0f);
        }
    }
    
    // Encode action history (simplified)
    // In a real implementation, you would want to encode the full action history
    int num_actions = std::min(10, static_cast<int>(action_history.size()));
    for (int i = 0; i < num_actions; i++) {
        int player = action_history[action_history.size() - 1 - i].first;
        ActionType action = action_history[action_history.size() - 1 - i].second;
        
        // One-hot for player
        for (int j = 0; j < 6; j++) {
            features.push_back(j == player ? 1.0f : 0.0f);
        }
        
        // One-hot for action
        for (int j = 0; j < 5; j++) {
            features.push_back(j == static_cast<int>(action) ? 1.0f : 0.0f);
        }
    }
    
    // Pad to fixed size if needed
    while (features.size() < 500) {
        features.push_back(0.0f);
    }
    
    return features;
}

int InfoState::getNumActions() const {
    return getLegalActions().size();
}

std::vector<ActionType> InfoState::getLegalActions() const {
    // Simplified legal actions based on the game phase
    std::vector<ActionType> actions;
    
    // Always include fold (except preflop with no raises)
    if (phase != HandPhase::Phase::PREFLOP || !action_history.empty()) {
        actions.push_back(ActionType::FOLD);
    }
    
    // Check/call
    actions.push_back(ActionType::CALL);
    
    // Raise/bet
    actions.push_back(ActionType::RAISE);
    
    // All-in
    actions.push_back(ActionType::ALL_IN);
    
    return actions;
}

std::string InfoState::toString() const {
    std::stringstream ss;
    
    ss << "Player: " << player_id << std::endl;
    ss << "Phase: " << phaseToString(phase) << std::endl;
    
    ss << "Hole cards: ";
    for (const auto& card : hole_cards) {
        ss << card.toString() << " ";
    }
    ss << std::endl;
    
    ss << "Board cards: ";
    for (const auto& card : board_cards) {
        ss << card.toString() << " ";
    }
    ss << std::endl;
    
    ss << "Pot sizes: ";
    for (int pot : pot_sizes) {
        ss << pot << " ";
    }
    ss << std::endl;
    
    ss << "Player stacks: ";
    for (int stack : player_stacks) {
        ss << stack << " ";
    }
    ss << std::endl;
    
    ss << "Player states: ";
    for (const auto& state : player_states) {
        ss << playerStateToString(state) << " ";
    }
    ss << std::endl;
    
    ss << "Action history: ";
    for (const auto& [player, action] : action_history) {
        ss << "P" << player << ":" << actionTypeToString(action) << " ";
    }
    ss << std::endl;
    
    return ss.str();
} 