#include <iostream>
#include <string>
#include <fmt/core.h>
#include <asio.hpp>

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

        // 2. Preparamos un buffer de memoria para recibir el mensaje de bienvenida
        asio::streambuf buffer_recepcion;
        asio::error_code error;

        // 3. Leemos de la red hasta encontrar un salto de línea ('\n')
        asio::read_until(socket, buffer_recepcion, '\n', error);

        if (!error) {
            // Extraemos el texto del buffer de Asio y lo convertimos a un string de C++
            std::istream stream_entrada(&buffer_recepcion);
            std::string mensaje_servidor;
            std::getline(stream_entrada, mensaje_servidor);

            // Imprimimos lo que nos dijo el servidor
            fmt::print("Servidor dice: {}\n", mensaje_servidor);
        }

        // Pausa antes de cerrar
        fmt::print("\nPresiona ENTER para salir...");
        std::cin.get();

    } catch (std::exception& e) {
        fmt::print("Error de conexión: ¿Está el servidor encendido?\nDetalle: {}\n", e.what());
    }

    return 0;
}