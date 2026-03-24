#include <vector>
#include <numeric>
#include <string>
#include <cstdlib>
#include <ctime>
#include <random> // Required for random number generators
#include <algorithm> // Required for std::shuffle
#include <chrono>    // Required for time-based seeding

// Usamos un 'namespace' para encapsular nuestra lógica y que no choque con otras librerías
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
    };

    class player{
        private:
            std::string mombre;
            std::vector<int> cartas;

            public:
                player(std::string nombre){
                    this->mombre=nombre;
                };

                void receiveCard(int num){
                    this->cartas.push_back(num);
                };

                // Devuelve las cartas formateadas como un texto separado por espacios
                std::string getHandAsString() const {
                    std::string hand = "";
                    for (int c : cartas) {
                        hand += std::to_string(c) + " ";
                    }
                    return hand;
                }

                int getCardCount(){
                    return this->cartas.size();
                };


    };

    enum class PlayResult {
        ValidPlay,
        LevelUp,
        LostLife,
        GameOver
    };

    class GameSession{
        private:
            int lives;
            int level;
            int lastCard;
            int shurikens;
            std::vector<player> players;
            int playedCards;
            Deck deck;

        public:
            GameSession(int nPlayers){
                this->level=1;
                this->lives=3;
                this->lastCard=0;
                this->shurikens=1;
                this->playedCards = 0;
                std::srand(std::time(NULL));

                for(int i = 0; i < nPlayers; i++) {
                    players.push_back(player("player " + std::to_string(i + 1)));
                }


            };     

            
            void start()
            {
                deck.reset();
                deck.shuffle();
                /*for(player player : players){ así me haría una copia del jugador, no me utilizaría el mismo*/ 
                for(auto& player : players){ /*acá traigo mi jugador usando &, auto es para que averigue el tipo */
                    for( int i = 0; i < level; i++){
                        player.receiveCard(deck.drawCard());
                    }
                }
            };

            bool useShuriken(){
                if (shurikens > 0)
                {
                    shurikens--;
                    return true;
                }
                return false;
                
            }

            void levelUp()
            {
                level++;
                playedCards =0;
                lastCard=0;
                lives += std::rand() % 2; 
                shurikens += std::rand() % 2;
            };

            PlayResult playCard(int card){
                if(lastCard == 0){
                    lastCard = card;
                    playedCards++;
                    if (playedCards == level * players.size())
                    {
                        levelUp(); 
                        return PlayResult::LevelUp;
                    }
                    return PlayResult::ValidPlay;
                }

                if(card > lastCard)
                {
                    lastCard=card;
                    playedCards++;

                    if(playedCards == level*players.size()){
                        levelUp(); 
                        return PlayResult::LevelUp;                       
                    }

                    return PlayResult::ValidPlay;
                }
                else
                {
                    lives--;
                    if (lives == 0) 
                    {
                        return PlayResult::GameOver;
                    }

                    return PlayResult::LostLife;
                }
            }

        int getShurikens() const { return shurikens; }
        int getLevel() const { return level; }
        int getLives() const { return lives; }
        int getLastCard() const { return lastCard; }
        player getPlayer(int index) const {
            return players[index];
        }

    };

}