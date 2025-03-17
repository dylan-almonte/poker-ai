#pragma once
#include <string>

class HandPhase {
public:
    enum Phase {
        PREHAND,  // Players sit out/rejoin, blinds move and post, cards dealt
        PREFLOP,  // First betting round, two hole cards, no community cards
        FLOP,     // Second betting round, three community cards
        TURN,     // Third betting round, one more community card
        RIVER,    // Fourth/final betting round, last community card
        SETTLE    // Winners decided, chips awarded
    };

    static const int NEW_CARDS[];  // Cards to deal in each phase
    static const Phase NEXT_PHASE[];  // Mapping to next phase

    static int getNewCards(Phase phase) { return NEW_CARDS[phase]; }
    static Phase getNextPhase(Phase phase) { return NEXT_PHASE[phase]; }
};

// Define static arrays
inline const int HandPhase::NEW_CARDS[] = {0, 0, 3, 1, 1, 0};
inline const HandPhase::Phase HandPhase::NEXT_PHASE[] = {
    PREFLOP, FLOP, TURN, RIVER, SETTLE, PREHAND
};

std::string phaseToString(HandPhase::Phase phase); 