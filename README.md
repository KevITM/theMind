
# 🧠 The Mind - Distributed C++ Engine

Un motor multijugador asíncrono y distribuido del famoso juego de cartas "The Mind", construido en C++ moderno. 

Este proyecto implementa una arquitectura Cliente-Servidor robusta utilizando *Sockets TCP*, paralelismo con *Multithreading* y protección de memoria compartida (*Mutex*), sentando las bases arquitectónicas similares a las utilizadas en sistemas de Machine Learning distribuido (Parameter Servers).

## ✨ Características Principales
* **Redes de Alto Rendimiento:** Comunicación bidireccional asíncrona usando `Asio`.
* **Concurrencia Segura:** Hilos independientes por jugador protegidos por `std::mutex` para evitar *Race Conditions* en la lógica central.
* **Máquina de Estados Estricta:** Validación de jugadas del lado del servidor (Server-Side Validation) para prevenir inyección de estados corruptos.
* **Serialización (Auto-Save):** Sistema de *Checkpoints* guardados en formato JSON usando `nlohmann-json`.
* **Aleatoriedad de Grado Científico:** Uso de `std::mt19937` (Mersenne Twister) inyectado con entropía de hardware para barajar y generar recompensas.

## 🛠️ Stack Tecnológico
* **Lenguaje:** C++17 / C++20
* **Build System:** CMake
* **Gestor de Paquetes:** vcpkg
* **Librerías Externas:** `asio` (Red), `fmt` (Formateo), `nlohmann-json` (Serialización), `gtest` (Testing).

## 🚀 Guía de Instalación y Compilación

1. **Clonar el repositorio y preparar la caché:**
   ```bash
   git clone <tu-repo>
   cd the-mind-game
   ```
2. **Configurar con CMake y vcpkg:**
   ```bash
   cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="<ruta-a-tu-vcpkg>/scripts/buildsystems/vcpkg.cmake"
   ```
3. **Compilar el proyecto (Servidor y Cliente):**
   ```bash
   cmake --build build
   ```

## 🎮 Cómo Jugar
1. Inicia el servidor: `.\build\Debug\the_mind_app.exe`
2. El servidor preguntará si deseas cargar el último `checkpoint.json`.
3. Inicia dos instancias del cliente: `.\build\Debug\the_mind_client.exe`
4. ¡Juega tus cartas escribiendo el número en la terminal, o usa `S` para lanzar un Shuriken!


---

# 📚 Wiki Técnica de Arquitectura

Puedes guardar este contenido en una carpeta `/docs` o en la sección Wiki de GitHub.

## 1. Visión General de la Arquitectura de Red

El sistema utiliza un modelo topológico de estrella centralizada. El Servidor actúa como la única fuente de verdad (Single Source of Truth), manteniendo el estado del juego, las vidas y las manos de los jugadores en su memoria RAM.



* **Acceptor (Puerto 8080):** El hilo principal del servidor espera las conexiones TCP.
* **Worker Threads:** Una vez que un cliente se conecta, se genera un hilo dedicado (`std::thread`) para escuchar exclusivamente a ese "Worker Node".
* **Event Loop del Cliente:** El cliente posee un hilo que imprime la interfaz de usuario en la terminal (HUD) y un hilo separado (desacoplado con `.detach()`) que lee las interacciones del teclado para no bloquear la lectura de la red.

## 2. Gestión de Concurrencia y Memoria

Dado que múltiples clientes envían datos simultáneamente, el servidor debe proteger la actualización del estado global (vidas, nivel, última carta).



* **Zona Crítica (`std::lock_guard<std::mutex>`):** Cada vez que un paquete de red es descifrado e interpretado como una jugada, el hilo correspondiente solicita el *Mutex de la Mesa*. Esto asegura que si dos jugadores juegan una carta en el mismo milisegundo exacto, el servidor procesará una jugada completa antes de procesar la otra, evitando la corrupción de punteros y memoria.

## 3. Máquina de Estados (State Machine)

La clase `GameSession` evalúa de forma global las transiciones de estado del juego usando un enumerador `PlayResult`.



* **ValidPlay:** La carta es correcta y es la más baja globalmente.
* **LostLife / LostLifeAndLevelUp:** Si un jugador tira una carta, el servidor itera sobre todos los jugadores (`players`). Si descubre cartas omitidas menores a la jugada, emite el castigo y ejecuta el protocolo de *Descarte Obligatorio* para limpiar el estado y destrabar la simulación.
* **Saneamiento de Época (Level Up):** Al subir de nivel, el servidor fuerza un reseteo estricto (`clearHand()`) para evitar fugas de memoria (*State Leak*) del nivel anterior, re-baraja usando entropía de hardware y repuebla el entorno.

## 4. Persistencia de Datos (Serialización JSON)

El motor implementa un sistema de tolerancia a fallos mediante un Checkpoint al final de cada fase de preparación.

* Al momento de finalizar `GameSession::start()`, las estructuras de datos nativas de C++ (Vectores, Enteros) son transformadas recursivamente mediante métodos `toJson()`.
* Para restaurar el estado, la función `loadCheckpoint()` lee el archivo de disco, parsea el árbol JSON, e inyecta los valores directamente en la clase usando los métodos `fromJson()`, recreando el escenario exacto antes de que los clientes reciban su primer paquete de datos.

