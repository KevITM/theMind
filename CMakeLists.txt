# Versión mínima de CMake (recomendado usar versiones recientes)
cmake_minimum_required(VERSION 3.20)

# Nombre del proyecto y lenguaje
project(TheMindGame VERSION 0.1.0 LANGUAGES CXX)

# Exigir C++20 (ideal para aprovechar características modernas)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Encontrar los paquetes instalados por vcpkg
find_package(fmt CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)

# 2. Definir el ejecutable principal (ejemplo: el servidor o el juego local)
# Aquí agregas los archivos fuente que componen este ejecutable
add_executable(the_mind_app 
    src/main.cpp 
)

# 3. Vincular las librerías al ejecutable
target_link_libraries(the_mind_app PRIVATE fmt::fmt)
target_link_libraries(the_mind_app PRIVATE fmt::fmt asio::asio)

# 4. Configuración de Tests Unitarios
enable_testing()
add_executable(core_tests 
    tests/test_game.cpp 
)
target_link_libraries(core_tests PRIVATE GTest::gtest_main GTest::gtest fmt::fmt)

# Integrar los tests con CTest (el ejecutor de tests de CMake)
include(GoogleTest)
gtest_discover_tests(core_tests)