#include <vector>
#include <numeric>
#include <string>
#include <cstdlib>
#include <ctime>

// Usamos un 'namespace' para encapsular nuestra lógica y que no choque con otras librerías
namespace themind {

    // En The Mind, el mazo central es crucial. 
    // Esta clase representa el mazo de cartas del 1 al 100.
    class Deck {
    private:
        // std::vector es el equivalente a las listas de Python.
        // Crece y se destruye dinámicamente. ¡Cero fugas de memoria!
        std::vector<int> cards;

    public:
        // Constructor: Se ejecuta al crear un objeto Deck
        Deck() {
            // Llenamos el mazo con cartas del 1 al 100
            cards.resize(100);
            std::iota(cards.begin(), cards.end(), 1); 
        }

        // Método constante (const): Promete que no modificará el estado del mazo
        int getRemainingCards() const {
            return cards.size();
        }
    };

    class Jugador{
        private:
            std::string mombre;
            std::vector<int> cartas;

            public:
                Jugador(std::string nombre){
                    this->mombre=nombre;
                };

                void receiveCard(int num){
                    this->cartas.push_back(num);
                };

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
            int players;
            int playedCards;

        public:
            GameSession(int players){
                this->level=1;
                this->lives=3;
                this->lastCard=0;
                this->shurikens=1;
                this->players=players;
                this->playedCards = 0;
                std::srand(std::time(NULL));


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
                    if (playedCards == level * players)
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

                    if(playedCards == level*players){
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

    };

}