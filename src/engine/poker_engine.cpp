#include "poker_engine.hpp"
#include <algorithm>
#include <stdexcept>

// GameState implementation
GameState GameState::with_player_state(size_t player, PlayerState new_state) const {
    GameState new_state = *this;
    new_state.player_states[player] = new_state;
    return new_state;
}

GameState GameState::with_player_chips(size_t player, int new_chips) const {
    GameState new_state = *this;
    new_state.player_chips[player] = new_chips;
    return new_state;
}

GameState GameState::with_player_bets(size_t player, int new_bets) const {
    GameState new_state = *this;
    new_state.player_bets[player] = new_bets;
    return new_state;
}

GameState GameState::with_player_rewards(size_t player, int new_rewards) const {
    GameState new_state = *this;
    new_state.player_rewards[player] = new_rewards;
    return new_state;
}

GameState GameState::with_pot(Pot new_pot) const {
    GameState new_state = *this;
    new_state.pots.push_back(new_pot);
    new_state.num_pots = new_state.pots.size();
    return new_state;
}

GameState GameState::with_board_card(size_t index, Card card) const {
    GameState new_state = *this;
    new_state.board[index] = card;
    return new_state;
}

GameState GameState::with_hole_cards(std::pair<Card, Card> new_cards) const {
    GameState new_state = *this;
    new_state.hole_cards = new_cards;
    return new_state;
}

GameState GameState::with_street(HandPhase::Phase new_street) const {
    GameState new_state = *this;
    new_state.street = new_street;
    return new_state;
}

GameState GameState::with_current_player(size_t new_player) const {
    GameState new_state = *this;
    new_state.current_player = new_player;
    return new_state;
}

GameState GameState::with_terminal(bool terminal) const {
    GameState new_state = *this;
    new_state.is_terminal = terminal;
    return new_state;
}

// PokerEngine implementation
PokerEngine::PokerEngine(int small_blind, int big_blind, size_t num_players)
    : small_blind_(small_blind)
    , big_blind_(big_blind)
    , num_players_(num_players)
    , deck_() {}

GameState PokerEngine::initial_state() const {
    GameState state;
    state.num_players = num_players_;
    state.player_states.resize(num_players_, PlayerState::IN);
    state.player_chips.resize(num_players_, 1000); // Starting stack
    state.player_bets.resize(num_players_, 0);
    state.player_rewards.resize(num_players_, 0);
    state.pots.push_back(Pot());
    state.num_pots = 1;
    return start_new_hand(state, small_blind_, big_blind_);
}

std::vector<Action> PokerEngine::legal_actions(const GameState& state) const {
    if (state.is_terminal) {
        return {};
    }

    std::vector<Action> actions;
    const int to_call = chips_to_call(state, state.current_player);
    const int player_chips = state.player_chips[state.current_player];

    // Fold is always legal
    actions.push_back(Action(ActionType::FOLD));

    // Check if checking is legal
    if (to_call == 0) {
        actions.push_back(Action(ActionType::CHECK));
    }

    // Call is legal if player has enough chips
    if (to_call > 0 && to_call <= player_chips) {
        actions.push_back(Action(ActionType::CALL));
    }

    // Raise is legal if player has enough chips
    if (player_chips > to_call) {
        const int min_raise = to_call * 2;
        if (min_raise <= player_chips) {
            actions.push_back(Action(ActionType::RAISE, min_raise));
        }
    }

    // All-in is legal if player has chips
    if (player_chips > 0) {
        actions.push_back(Action(ActionType::ALL_IN));
    }

    return actions;
}

GameState PokerEngine::step(const GameState& state, const Action& action) const {
    if (state.is_terminal) {
        throw std::runtime_error("Cannot step from terminal state");
    }

    if (!is_valid_action(state, action)) {
        throw std::invalid_argument("Invalid action for current state");
    }

    // Process the action
    GameState new_state = process_action(state, action);

    // Check if betting round is complete
    if (is_betting_round_complete(new_state)) {
        new_state = collect_bets(new_state);
        
        // Check if hand is complete
        if (is_hand_complete(new_state)) {
            new_state = award_pots(new_state);
            new_state = new_state.with_terminal(true);
        } else {
            // Move to next street
            new_state = deal_community_cards(new_state, deck_);
            new_state = new_state.with_street(static_cast<HandPhase::Phase>(
                static_cast<int>(new_state.street) + 1
            ));
            new_state = new_state.with_current_player(next_active_player(new_state, 0));
        }
    } else {
        // Move to next player
        new_state = new_state.with_current_player(next_active_player(new_state, new_state.current_player + 1));
    }

    return new_state;
}

size_t PokerEngine::current_player(const GameState& state) const {
    return state.current_player;
}

bool PokerEngine::is_terminal(const GameState& state) const {
    return state.is_terminal;
}

std::vector<int> PokerEngine::rewards(const GameState& state) const {
    return state.player_rewards;
}

// Helper methods
bool PokerEngine::is_betting_round_complete(const GameState& state) {
    // Check if all players have acted
    size_t last_player = last_to_act(state);
    if (state.current_player != last_player) {
        return false;
    }

    // Check if all players have called or folded
    for (size_t i = 0; i < state.num_players; i++) {
        if (state.player_states[i] == PlayerState::IN && 
            state.player_bets[i] < state.pots.back().get_raised()) {
            return false;
        }
    }

    return true;
}

bool PokerEngine::is_hand_complete(const GameState& state) {
    // Count active players
    size_t active_players = 0;
    for (const auto& player_state : state.player_states) {
        if (player_state == PlayerState::IN || player_state == PlayerState::ALL_IN) {
            active_players++;
        }
    }

    // Hand is complete if only one player is active or we're on the river
    return active_players <= 1 || state.street == HandPhase::Phase::RIVER;
}

size_t PokerEngine::next_active_player(const GameState& state, size_t start) {
    size_t player = start % state.num_players;
    while (state.player_states[player] != PlayerState::IN && 
           state.player_states[player] != PlayerState::TO_CALL) {
        player = (player + 1) % state.num_players;
    }
    return player;
}

size_t PokerEngine::last_to_act(const GameState& state) {
    // Find the last player who can act
    size_t last_player = state.current_player;
    for (size_t i = 0; i < state.num_players; i++) {
        size_t player = (state.current_player + i) % state.num_players;
        if (state.player_states[player] == PlayerState::IN || 
            state.player_states[player] == PlayerState::TO_CALL) {
            last_player = player;
        }
    }
    return last_player;
}

int PokerEngine::chips_to_call(const GameState& state, size_t player) {
    if (state.pots.empty()) {
        return 0;
    }
    return state.pots.back().chips_to_call(player);
}

bool PokerEngine::is_valid_action(const GameState& state, const Action& action) {
    const auto legal_actions = PokerEngine::legal_actions(state);
    return std::find(legal_actions.begin(), legal_actions.end(), action) != legal_actions.end();
}

// State transition methods
GameState PokerEngine::start_new_hand(const GameState& state, int small_blind, int big_blind) {
    GameState new_state = state;
    new_state.street = HandPhase::Phase::PREFLOP;
    new_state.pots.clear();
    new_state.pots.push_back(Pot());
    new_state.num_pots = 1;
    new_state.board = std::vector<Card>(5, Card(-1));
    
    // Reset player states and bets
    for (size_t i = 0; i < new_state.num_players; i++) {
        if (new_state.player_chips[i] > 0) {
            new_state.player_states[i] = PlayerState::IN;
        }
        new_state.player_bets[i] = 0;
    }
    
    // Deal cards and post blinds
    new_state = deal_hole_cards(new_state, deck_);
    new_state = post_blinds(new_state, small_blind, big_blind);
    new_state = new_state.with_current_player(next_active_player(new_state, 0));
    
    return new_state;
}

GameState PokerEngine::post_blinds(const GameState& state, int small_blind, int big_blind) {
    GameState new_state = state;
    
    // Post small blind
    size_t sb_pos = (new_state.current_player + 1) % new_state.num_players;
    if (new_state.player_states[sb_pos] == PlayerState::IN) {
        new_state = new_state.with_player_chips(sb_pos, new_state.player_chips[sb_pos] - small_blind);
        new_state = new_state.with_player_bets(sb_pos, small_blind);
        new_state.pots[0].player_post(sb_pos, small_blind);
    }
    
    // Post big blind
    size_t bb_pos = (new_state.current_player + 2) % new_state.num_players;
    if (new_state.player_states[bb_pos] == PlayerState::IN) {
        new_state = new_state.with_player_chips(bb_pos, new_state.player_chips[bb_pos] - big_blind);
        new_state = new_state.with_player_bets(bb_pos, big_blind);
        new_state.pots[0].player_post(bb_pos, big_blind);
    }
    
    return new_state;
}

GameState PokerEngine::deal_hole_cards(const GameState& state, Deck& deck) {
    GameState new_state = state;
    deck.shuffle();
    
    for (size_t i = 0; i < new_state.num_players; i++) {
        if (new_state.player_states[i] == PlayerState::IN) {
            new_state = new_state.with_hole_cards({deck.deal(), deck.deal()});
        }
    }
    
    return new_state;
}

GameState PokerEngine::deal_community_cards(const GameState& state, Deck& deck) {
    GameState new_state = state;
    
    switch (new_state.street) {
        case HandPhase::Phase::FLOP:
            new_state = new_state.with_board_card(0, deck.deal());
            new_state = new_state.with_board_card(1, deck.deal());
            new_state = new_state.with_board_card(2, deck.deal());
            break;
        case HandPhase::Phase::TURN:
            new_state = new_state.with_board_card(3, deck.deal());
            break;
        case HandPhase::Phase::RIVER:
            new_state = new_state.with_board_card(4, deck.deal());
            break;
        default:
            break;
    }
    
    return new_state;
}

GameState PokerEngine::process_action(const GameState& state, const Action& action) {
    GameState new_state = state;
    const int to_call = chips_to_call(state, state.current_player);
    
    switch (action.getActionType()) {
        case ActionType::FOLD:
            new_state = new_state.with_player_state(state.current_player, PlayerState::OUT);
            break;
            
        case ActionType::CHECK:
            // No state change needed for check
            break;
            
        case ActionType::CALL:
            new_state = new_state.with_player_chips(state.current_player, 
                state.player_chips[state.current_player] - to_call);
            new_state = new_state.with_player_bets(state.current_player, 
                state.player_bets[state.current_player] + to_call);
            new_state.pots[0].player_post(state.current_player, to_call);
            break;
            
        case ActionType::RAISE:
            new_state = new_state.with_player_chips(state.current_player, 
                state.player_chips[state.current_player] - action.getAmount());
            new_state = new_state.with_player_bets(state.current_player, 
                state.player_bets[state.current_player] + action.getAmount());
            new_state.pots[0].player_post(state.current_player, action.getAmount());
            break;
            
        case ActionType::ALL_IN:
            new_state = new_state.with_player_state(state.current_player, PlayerState::ALL_IN);
            new_state = new_state.with_player_bets(state.current_player, 
                state.player_bets[state.current_player] + state.player_chips[state.current_player]);
            new_state.pots[0].player_post(state.current_player, state.player_chips[state.current_player]);
            new_state = new_state.with_player_chips(state.current_player, 0);
            break;
    }
    
    return new_state;
}

GameState PokerEngine::collect_bets(const GameState& state) {
    GameState new_state = state;
    for (auto& pot : new_state.pots) {
        pot.collect_bets();
    }
    return new_state;
}

GameState PokerEngine::split_pot(const GameState& state) {
    GameState new_state = state;
    // TODO: Implement pot splitting logic
    return new_state;
}

GameState PokerEngine::award_pots(const GameState& state) {
    GameState new_state = state;
    // TODO: Implement pot awarding logic
    return new_state;
} 