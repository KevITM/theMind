#include <iostream>
#include <vector>
#include <fmt/core.h>
#include <asio.hpp>
#include <thread>
#include <mutex>

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
        
        // --- AQUÍ CONECTAMOS LA RED CON NUESTRO CEREBRO LÓGICO ---
        themind::GameSession sesion(num_jugadores_esperados);
        
        bool partida_cargada = false;
        
        // 1. Verificamos rápidamente si existe un checkpoint
        std::ifstream file_check("checkpoint.json");
        if (file_check.is_open()) {
            file_check.close();
            fmt::print("\n[!] Se detecto un 'checkpoint.json' en el disco.\n");
            fmt::print("¿Deseas restaurar la partida anterior? (s/n): ");
            
            std::string respuesta;
            std::getline(std::cin, respuesta);
            
            // 2. Intentamos cargar
            if (respuesta == "s" || respuesta == "S") {
                if (sesion.loadCheckpoint()) {
                    partida_cargada = true;
                    fmt::print(">> Partida restaurada con exito en el Nivel {}.\n", sesion.getLevel());
                } else {
                    fmt::print(">> Error al leer el checkpoint. El archivo podria estar corrupto.\n");
                }
            }
        }

        // 3. Si no quisieron cargar, o falló la carga, o no existía el archivo:
        if (!partida_cargada) {
            fmt::print("Iniciando una nueva partida desde cero...\n");
            sesion.start(); // Baraja y reparte el Nivel 1
        }

        // 4. ENVIAMOS EL ESTADO INICIAL A CADA CLIENTE (Sea Nivel 1 o el Nivel Restaurado)
        for (int i = 0; i < num_jugadores_esperados; ++i) {
            std::string mano = sesion.getPlayer(i).getHandAsString();
            
            // Ya no dice "Nivel 1" de forma fija, lee el nivel real de la sesión
            std::string msj = fmt::format("\n=== NIVEL {} ===\nVidas: {} | Shurikens: {}\nTus cartas son: {}\n", 
                                          sesion.getLevel(), sesion.getLives(), sesion.getShurikens(), mano);
            
            asio::write(sockets_jugadores[i], asio::buffer(msj));
        }

        // --- EL CEREBRO DEL JUEGO MULTIHILO (Desde el mutex en adelante se queda igual) ---
        std::mutex mutex_mesa;

        // Creamos un hilo para escuchar a cada jugador individualmente
        std::vector<std::thread> hilos_escucha;

        for (int i = 0; i < num_jugadores_esperados; ++i) {
            // Creamos un hilo y le pasamos referencias a todo lo que necesita
            hilos_escucha.push_back(std::thread([i, num_jugadores_esperados, &sockets_jugadores, &sesion, &mutex_mesa]() {                
                asio::streambuf buffer_recepcion;
                
                // Bucle infinito escuchando a este jugador específico
                while (true) {
                    asio::error_code error;
                    asio::read_until(sockets_jugadores[i], buffer_recepcion, '\n', error);

                    if (error) {
                        fmt::print("[!] Jugador {} se ha desconectado.\n", i + 1);
                        break;
                    }

                    // Extraemos lo que escribió el jugador
                    std::istream stream_entrada(&buffer_recepcion);
                    std::string mensaje_recibido;
                    std::getline(stream_entrada, mensaje_recibido);

                    try {

                        
                        // === ZONA CRÍTICA: PROTEGIDA POR MUTEX ===
                        {
                            std::lock_guard<std::mutex> lock(mutex_mesa);
                            
                            // Función helper (Lambda) para repartir un nuevo nivel y avisar a todos
                            auto iniciar_y_repartir_nivel = [&]() {
                                sesion.start(); // Baraja y reparte internamente
                                for (int j = 0; j < num_jugadores_esperados; ++j) {
                                    std::string mano = sesion.getPlayer(j).getHandAsString();
                                    std::string msj_estado = fmt::format("\n=== NIVEL {} ===\nVidas: {} | Shurikens: {}\nTu mano: {}\n", 
                                                                        sesion.getLevel(), sesion.getLives(), sesion.getShurikens(), mano);
                                    asio::write(sockets_jugadores[j], asio::buffer(msj_estado));
                                }
                            };

                            // 1. ¿EL JUGADOR TIRÓ UN SHURIKEN?
                            if (mensaje_recibido == "S" || mensaje_recibido == "s") {
                                std::string reporte_shuriken;
                                bool paso_nivel = sesion.useShuriken(reporte_shuriken);
                                
                                // Avisamos a todos qué cartas se descartaron
                                for (auto& sock : sockets_jugadores) {
                                    asio::write(sock, asio::buffer(reporte_shuriken));
                                }

                                if (paso_nivel) {
                                    std::string msj = fmt::format("\n>> ¡EXCELENTE! Avanzan al Nivel {}.\n", sesion.getLevel());
                                    for (auto& sock : sockets_jugadores) asio::write(sock, asio::buffer(msj));
                                    iniciar_y_repartir_nivel();
                                }
                                continue; // Terminamos el turno de este hilo
                            }

                            // 2. LÓGICA DE JUGAR UNA CARTA NORMAL
                            try {
                                int carta_jugada = std::stoi(mensaje_recibido);
                                mensaje_recibido.erase(mensaje_recibido.find_last_not_of(" \n\r\t") + 1);

                                fmt::print("Jugador {} intenta jugar la carta: {}\n", i + 1, carta_jugada);
                                
                                // Pasamos 'i' (el índice del jugador) y la carta
                                themind::PlayResult resultado = sesion.playCard(i, carta_jugada);
                                std::string mensaje_global = "";

                                // Evaluamos qué pasó
                                if (resultado == themind::PlayResult::InvalidCard) {
                                    asio::write(sockets_jugadores[i], asio::buffer(">> [SISTEMA] No tienes esa carta en tu mano.\n"));
                                    continue; 
                                }
                                else if (resultado == themind::PlayResult::ValidPlay) {
                                    mensaje_global = fmt::format(">> Jugador {} jugo un {}. (Mesa: {})\n", i + 1, carta_jugada, carta_jugada);
                                } 
                                else if (resultado == themind::PlayResult::LostLife) {
                                    mensaje_global = fmt::format(">> [ERROR] Jugador {} tiro un {}. ¡PERDIERON UNA VIDA!\n", i + 1, carta_jugada);
                                }
                                else if (resultado == themind::PlayResult::LostLifeAndLevelUp) {
                                    mensaje_global = fmt::format(">> [ERROR] Jugador {} tiro un {}. ¡PERDIERON UNA VIDA! Pero lograron vaciar sus manos...\n", i + 1, carta_jugada);
                                }
                                else if (resultado == themind::PlayResult::GameOver) {
                                    mensaje_global = ">> [GAME OVER] Se quedaron sin vidas. El experimento ha terminado.\n";
                                    for (auto& sock : sockets_jugadores) asio::write(sock, asio::buffer(mensaje_global));
                                    break;
                                }

                                // 1. BROADCAST: Enviamos el resultado de la mesa a todos
                                for (auto& sock : sockets_jugadores) asio::write(sock, asio::buffer(mensaje_global));

                                // 2. ACTUALIZACIÓN DE HUD: Le enviamos a cada jugador sus cartas restantes
                                if (resultado == themind::PlayResult::ValidPlay || resultado == themind::PlayResult::LostLife) {
                                    for (int j = 0; j < num_jugadores_esperados; ++j) {
                                        std::string mi_mano = sesion.getPlayer(j).getHandAsString();
                                        if (mi_mano.empty()) mi_mano = "[Ninguna]"; // UX para que no se vea vacío
                                        
                                        std::string hud = fmt::format("---| Tu mano actual: {} | Vidas equipo: {} |---\n", mi_mano, sesion.getLives());
                                        asio::write(sockets_jugadores[j], asio::buffer(hud));
                                    }
                                }

                                // 3. SUBIR DE NIVEL: Si ganaron (limpios o con error), repartimos el siguiente
                                if (resultado == themind::PlayResult::LevelUp || resultado == themind::PlayResult::LostLifeAndLevelUp) {
                                    std::string msj = fmt::format("\n>> Preparando el Nivel {}...\n", sesion.getLevel());
                                    for (auto& sock : sockets_jugadores) asio::write(sock, asio::buffer(msj));
                                    iniciar_y_repartir_nivel();
                                }

                            } catch (const std::invalid_argument&) {
                                asio::write(sockets_jugadores[i], asio::buffer("Comando invalido. Usa un numero o 'S' para Shuriken.\n"));
                            }
                        }
                        // === FIN DE ZONA CRÍTICA === (El lock_guard suelta el bastón automáticamente aquí)

                    } catch (const std::invalid_argument&) {
                        // Si el usuario escribe "hola" en lugar de un número, lo ignoramos
                        std::string msj_error = "Por favor, escribe solo el numero de la carta.\n";
                        asio::write(sockets_jugadores[i], asio::buffer(msj_error));
                    }
                }
            }));
        }

        // Le decimos al hilo principal del servidor que espere a que todos los hilos 
        // secundarios terminen (cosa que solo pasará si cierran el juego)
        for (auto& hilo : hilos_escucha) {
            hilo.join();
        }

    } catch (std::exception& e) {
        fmt::print("Error crítico: {}\n", e.what());
    }

    return 0;
}