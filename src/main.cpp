#include <iostream>
#include <fmt/core.h>
#include <asio.hpp>

// Un atajo para no escribir asio::ip::tcp todo el tiempo
using asio::ip::tcp;

int main() {
    try {
        // 1. Inicializamos el motor de red
        asio::io_context io_context;

        // 2. Creamos el recepcionista. 
        // Le decimos que escuche en el puerto 8080 usando IPv4
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));
        
        fmt::print("=== SERVIDOR DE THE MIND ===\n");
        fmt::print("Escuchando en el puerto 8080...\n");

        // 3. Preparamos el cable (socket)
        tcp::socket socket(io_context);
        
        // 4. ¡El programa se pausa aquí!
        // acceptor.accept() bloquea la terminal hasta que un cliente real se conecte
        acceptor.accept(socket);

        // Si el código llega a esta línea, significa que alguien se conectó exitosamente
        fmt::print("\n[+] ¡Un nuevo jugador se ha conectado a la mesa!\n");

        // Vamos a enviarle un pequeño mensaje de bienvenida a través del cable
        std::string mensaje = "Bienvenido a The Mind. Esperando a los demas jugadores...\n";
        
        // Escribimos en la red (usamos asio::write para asegurar que todo el texto viaje)
        asio::error_code error;
        asio::write(socket, asio::buffer(mensaje), error);

        if (!error) {
            fmt::print("Mensaje de bienvenida enviado correctamente.\n");
        }

    } catch (std::exception& e) {
        // Si el puerto 8080 está ocupado o no hay internet, el programa no explota, 
        // cae aquí de forma segura.
        fmt::print("Error crítico de red: {}\n", e.what());
    }

    return 0;
}