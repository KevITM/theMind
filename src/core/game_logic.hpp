#include <vector>
#include <numeric>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fmt/core.h>
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

                int getCardCount() const {
                    return this->cartas.size();
                };
                
                // Verifica si el jugador realmente tiene esta carta
                bool hasCard(int card) const {
                    // std::find busca en el vector. Si no llega al final (.end()), es porque la encontró.
                    return std::find(cartas.begin(), cartas.end(), card) != cartas.end(); // ac+a obtiene el último posición en memoria despúes del último elemento ??
                }

                // Elimina la carta de la mano después de jugarla
                void removeCard(int card) {
                    auto it = std::find(cartas.begin(), cartas.end(), card);
                    if (it != cartas.end()) {
                        cartas.erase(it);
                    }
                }

                void sortHand() {
                    std::sort(cartas.begin(), cartas.end());
                }

                // Para que el servidor sepa cuál es la carta más baja (para el Shuriken)
                int getLowestCard() const {
                    if (cartas.empty()) return 0;
                    return cartas.front(); // Como están ordenadas, la primera es la más baja
                }

                // Para eliminar la más baja
                void removeLowestCard() {
                    if (!cartas.empty()) {
                        cartas.erase(cartas.begin());
                    }
                }
                
                // Para limpiar la mano al empezar un nuevo nivel
                void clearHand() {
                    cartas.clear();
                }
    };

    enum class PlayResult {
        ValidPlay,
        LevelUp,
        LostLife,
        GameOver,
        InvalidCard,
        LostLifeAndLevelUp
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
                /*for(player player : players){ a   sí me haría una copia del jugador, no me utilizaría el mismo*/ 
                for(auto& player : players){ /*acá traigo mi jugador usando &, auto es para que averigue el tipo */
                    player.clearHand();
                    for( int i = 0; i < level; i++){
                        player.receiveCard(deck.drawCard());
                    }
                    player.sortHand();
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

            void levelUp() {
                level++;
                lastCard = 0; // Limpiamos la mesa
                
                // 1. Creamos entropía real desde el hardware
                std::random_device rd;
                
                // 2. Inicializamos el motor avanzado
                std::mt19937 gen(rd());
                
                // 3. Definimos las reglas de la moneda: 50% de probabilidad (0 o 1)
                std::uniform_int_distribution<> moneda(0, 1);

                // Tiramos la moneda para cada recompensa de forma independiente
                int recompensa_vida = moneda(gen);
                int recompensa_shuriken = moneda(gen);

                lives += recompensa_vida;
                shurikens += recompensa_shuriken;
                
                // Opcional: Imprimimos en la consola del servidor qué premios tocaron
                fmt::print("\n[RECOMPENSAS] Nivel completado. Vidas ganadas: {} | Shurikens ganados: {}\n", 
                           recompensa_vida, recompensa_shuriken);
            }

            PlayResult playCard(int playerIndex, int card){
                // 1. Verificamos que no sea trampa (que sí tenga la carta)
                if(!players[playerIndex].hasCard(card)){
                    return PlayResult::InvalidCard;
                }

                // Sacamos la carta de la mano del jugador
                players[playerIndex].removeCard(card);
                
                bool jugada_incorrecta = false;

                // 2. INSPECCIÓN GLOBAL: ¿Alguien más tiene una carta menor?
                for (auto& p : players) {
                    // Si el jugador tiene cartas y su carta más baja es menor a la que tiraste...
                    if (p.getCardCount() > 0 && p.getLowestCard() < card) {
                        jugada_incorrecta = true;
                        
                        // Regla oficial: Descartamos todas las cartas menores que se quedaron "atrapadas"
                        while (p.getCardCount() > 0 && p.getLowestCard() < card) {
                            p.removeLowestCard();
                            playedCards++; // Las contamos como jugadas para que el nivel pueda terminar
                        }
                    }
                }

                // 3. ¿Alguien tiró una carta menor a la que ya estaba en la mesa por accidente?
                if (lastCard != 0 && card < lastCard) {
                    jugada_incorrecta = true;
                }

                // Actualizamos la mesa
                lastCard = card;
                // Ya no importa si olvidamos un playedCards++; la verdad está en las manos.

                // 4. Procesamos el castigo y el estado final
                if (jugada_incorrecta) {
                    lives--;
                    if (lives <= 0) {
                        return PlayResult::GameOver;
                    }
                    
                    // USAMOS LA ÚNICA FUENTE DE VERDAD
                    if (isLevelComplete()) {
                        levelUp();
                        return PlayResult::LostLifeAndLevelUp;
                    }
                    
                    return PlayResult::LostLife;
                }

                // 5. Si no hubo castigo, verificamos victoria normal
                if (isLevelComplete()) {
                    levelUp(); 
                    return PlayResult::LevelUp;
                }

                return PlayResult::ValidPlay;
            };

            bool useShuriken(std::string& reporte) {
                if (shurikens > 0) {
                    shurikens--;
                    reporte = "\n>> [SHURIKEN USADO] Vuelan las estrellas ninja. Cartas descartadas: ";
                    
                    for (auto& p : players) {
                        if (p.getCardCount() > 0) {
                            reporte += std::to_string(p.getLowestCard()) + " ";
                            p.removeLowestCard();
                        }
                    }
                    reporte += "\n";

                    // REEMPLAZAMOS LA MATEMÁTICA AQUÍ TAMBIÉN
                    if (isLevelComplete()) {
                        levelUp();
                        return true; 
                    }
                    return false;
                }
                reporte = ">> [ERROR] No les quedan Shurikens.\n";
                return false;
            }

            bool isLevelComplete() const {
                int jugadores_con_cartas = 0;
                
                for (const auto& p : players) {
                    if (p.getCardCount() > 0) {
                        jugadores_con_cartas++;
                    }
                }
                
                // Si solo queda 1 jugador con cartas (o ninguno), el nivel está matemáticamente ganado
                return jugadores_con_cartas <= 1; 
            }

        int getShurikens() const { return shurikens; }
        int getLevel() const { return level; }
        int getLives() const { return lives; }
        int getLastCard() const { return lastCard; }
        player& getPlayer(int index) {
            return players[index];
        }

    };

}