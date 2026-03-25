#ifndef THEMIND_PLAYER_HPP
#define THEMIND_PLAYER_HPP

#include <vector>
#include <string>
#include <algorithm>

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace themind {

    class player {
    private:
        std::string nombre;
        std::vector<int> cartas;

    public:
        player(std::string nombre){
            this->nombre=nombre;
        }

        void receiveCard(int num){
            this->cartas.push_back(num);
        }

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
        }
        
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

        json toJson() const {
            return {
                {"nombre", nombre}, 
                {"cartas", cartas}
            };
        }
    };

}

#endif // THEMIND_PLAYER_HPP
