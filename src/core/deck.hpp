#ifndef THEMIND_DECK_HPP
#define THEMIND_DECK_HPP

#include <vector>
#include <numeric>
#include <random>
#include <algorithm>
#include <chrono>

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace themind {

    class Deck {
    private:
        std::vector<int> cards;

    public:
        Deck() {
            cards.resize(100);
            std::iota(cards.begin(), cards.end(), 1); 
        }

        void reset(){
            cards.resize(100);
            std::iota(cards.begin(),cards.end(),1); /*rellenar un rango con incrementos secuenciales.*/
        }

        void shuffle()
        {
            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
            std::mt19937 rng(seed);
            std::shuffle(cards.begin(), cards.end(), rng);
        }

        int drawCard() {
            int temCard = cards.back();
            cards.pop_back();
            return temCard; 
        }

        int getRemainingCards() const {
            return cards.size();
        }
        
        json toJson() const {
            return {
                {"cards", cards}
            };
        }
    };

}

#endif // THEMIND_DECK_HPP
