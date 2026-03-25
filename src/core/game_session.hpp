#ifndef THEMIND_GAME_SESSION_HPP
#define THEMIND_GAME_SESSION_HPP

#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fmt/core.h>
#include <random>

#include <fstream>
#include <nlohmann/json.hpp>


#include "deck.hpp"
#include "player.hpp"
#include "play_result.hpp"

using json = nlohmann::json;


namespace themind {

    class GameSession {
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
        }

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
        }

        bool useShuriken(){
            if (shurikens > 0)
            {
                shurikens--;
                return true;
            }
            return false;
        }

        // Guarda la partida en un archivo
        void saveCheckpoint(const std::string& filename = "checkpoint.json") const {
            json j;
            j["level"] = level;
            j["lives"] = lives;
            j["lastCard"] = lastCard;
            j["shurikens"] = shurikens;
            j["playedCards"] = playedCards;
            
            // Guardamos el mazo
            j["deck"] = deck.toJson();

            // Guardamos a los jugadores (iteramos y convertimos a JSON)
            json players_json = json::array();
            for (const auto& p : players) {
                players_json.push_back(p.toJson());
            }
            j["players"] = players_json;

            // Escribimos al disco duro
            std::ofstream file(filename);
            if (file.is_open()) {
                // El '4' es para que el JSON se guarde con indentación legible
                file << j.dump(4);
                file.close();
            }
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
                                  
            saveCheckpoint();
            fmt::print("[SISTEMA] Partida guardada correctamente en checkpoint.json\n");

        
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
        }

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

#endif // THEMIND_GAME_SESSION_HPP
