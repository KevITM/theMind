#include <iostream>
#include <vector>
#include <fmt/core.h>
#include <asio.hpp>

// Incluimos la lógica de nuestro juego
#include "core/game_logic.hpp"

using asio::ip::tcp;

int main() {
    try {
        asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));
        
        int num_jugadores_esperados = 2;
        
        // Aquí guardaremos los sockets conectados a cada jugador
        std::vector<tcp::socket> sockets_jugadores; // Los Sockets no se pueden copiar.!!
        /*
        Piensa en un socket como si fuera un teléfono físico conectado a la pared. Si intentas hacer vector.push_back(socket), C++ intentará crear un clon exacto del teléfono. 
        El sistema operativo no permite esto porque si dos clones intentan leer los mismos datos de la red al mismo tiempo, el sistema colapsaría.
         */

        fmt::print("=== SERVIDOR DE THE MIND ===\n");
        fmt::print("Esperando a {} jugadores en el puerto 8080...\n", num_jugadores_esperados);

        // Bucle para dejar entrar a los jugadores uno por uno
        for (int i = 0; i < num_jugadores_esperados; ++i) {
            
            tcp::socket socket_temporal(io_context);
            
            // El programa se pausa aquí hasta que entra alguien
            acceptor.accept(socket_temporal);
            
            fmt::print("[+] Jugador {} conectado desde: {}\n", 
                       i + 1, 
                       socket_temporal.remote_endpoint().address().to_string());

            // Le enviamos un mensaje personalizado
            std::string msj = fmt::format("Bienvenido a The Mind. Eres el Jugador {}.\n", i + 1);
            asio::write(socket_temporal, asio::buffer(msj));

            // Usamos std::move para transferir la propiedad del socket al vector sin copiarlo.
            sockets_jugadores.push_back(std::move(socket_temporal));
            /*
             std::move() es una de las razones por las que C++ es tan rápido. 
             En lugar de copiar megabytes de datos en memoria, simplemente cambia los "dueños" de los punteros a nivel de hardware.
            */
        }

        fmt::print("\n¡Todos los jugadores se han conectado!\n");
        fmt::print("Iniciando el motor del juego...\n");

        // --- AQUÍ CONECTAMOS LA RED CON NUESTRO CEREBRO LÓGICO ---
        themind::GameSession sesion(num_jugadores_esperados);
        sesion.start(); // Baraja y reparte las cartas del Nivel 1

        fmt::print("Nivel 1 iniciado. Cartas repartidas en la memoria del servidor.\n");

        // --- ENVIAMOS LAS CARTAS A CADA CLIENTE ---
        for (int i = 0; i < num_jugadores_esperados; ++i) {
            // 1. Obtenemos las cartas de este jugador específico en formato texto
            std::string mano = sesion.getPlayer(i).getHandAsString();
            
            // 2. Formateamos el mensaje con un salto de línea ('\n') al final.
            // El '\n' es VITAL porque le dice al cliente cuándo termina el mensaje.
            std::string msj = fmt::format("\n--- NIVEL 1 ---\nTus cartas son: {}\n", mano);
            
            // 3. Enviamos el texto por su cable correspondiente
            asio::write(sockets_jugadores[i], asio::buffer(msj));
        }

        // Para evitar que el servidor se cierre de golpe, lo pausamos temporalmente
        // pidiéndole al administrador que presione Enter en la consola.
        fmt::print("Presiona ENTER para cerrar el servidor...");
        std::cin.get();

    } catch (std::exception& e) {
        fmt::print("Error crítico de red: {}\n", e.what());
    }

    return 0;
}