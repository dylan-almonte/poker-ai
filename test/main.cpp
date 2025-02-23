#include <iostream>


int main() {
    poker::Deck deck;
    deck.shuffle();
    
    std::cout << "Drawing 5 cards:" << std::endl;
    for (int i = 0; i < 5; i++) {
        poker::Card card = deck.draw();
        std::cout << card.toString() << std::endl;
    }
    
    return 0;
} 