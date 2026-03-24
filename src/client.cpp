#include <iostream>
#include <string>
#include <fmt/core.h>
#include <asio.hpp>
#include <thread>

using asio::ip::tcp;

int main() {
    try {
        asio::io_context io_context;
        tcp::socket socket(io_context);

        fmt::print("=== CLIENTE DE THE MIND ===\n");
        fmt::print("Conectando al servidor en 127.0.0.1:8080...\n");

        // 1. Tocamos la puerta del servidor
        socket.connect(tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 8080));
        
        fmt::print("[+] ¡Conectado a la mesa con éxito!\n\n");

        asio::streambuf buffer_recepcion; /* si se deja andetro de while se crear un nuevo buffer y se elimina todo*/
        /*
        Este concepto de "consumir flujos de datos" es la base de cómo herramientas masivas (como Apache Kafka o los sistemas de entrenamiento distribuido) evitan perder información cuando la red es más rápida que el procesador.
         */


         // Creamos un hilo paralelo que recibe el socket por referencia (&)
        std::thread hilo_teclado([&socket]() {
            std::string entrada_teclado;
            
            // Bucle infinito que espera a que escriba algo y presione ENTER
            while (std::getline(std::cin, entrada_teclado)) {
                
                // Le agregamos un salto de línea vital para que el servidor sepa dónde termina
                std::string mensaje_a_enviar = entrada_teclado + "\n";
                
                // Enviamos lo que escribiste a través de la red
                asio::error_code error_escritura;
                asio::write(socket, asio::buffer(mensaje_a_enviar), error_escritura);
                
                if (error_escritura) {
                    break; // Si se corta la red, el hilo termina
                }
            }
        });

        hilo_teclado.detach();

        // BUCLE INFINITO DEL JUEGO
        while (true) {
            asio::error_code error;

            // El programa se pausa aquí esperando a que el servidor diga algo
            asio::read_until(socket, buffer_recepcion, '\n', error);

            // Si hay un error (ej. el servidor se cerró o se cayó el internet)
            if (error) {
                fmt::print("\n[!] Desconectado del servidor.\n");
                break; // Rompemos el bucle y el programa termina
            }

            // Extraemos e imprimimos el mensaje
            std::istream stream_entrada(&buffer_recepcion);
            std::string mensaje_servidor;
            std::getline(stream_entrada, mensaje_servidor);

            fmt::print("{}\n", mensaje_servidor);
        }


    } catch (std::exception& e) {
        fmt::print("Error de conexión: ¿Está el servidor encendido?\nDetalle: {}\n", e.what());
    }

    return 0;
}