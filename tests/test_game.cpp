#include <gtest/gtest.h>
#include "../src/core/game_logic.hpp" 

// Los tests se dividen en 3 fases: Arrange (Preparar), Act (Actuar), Assert (Comprobar)

TEST(JugadorTest, RecibirCartasAumentaElConteo) {
    // Arrange: Creamos un jugador
    themind::Jugador jugador1("Ada Lovelace");
    
    // Act: Le damos dos cartas simuladas
    jugador1.receiveCard(42);
    jugador1.receiveCard(15);
    
    // Assert: Comprobamos que la máquina cuente exactamente 2 cartas
    EXPECT_EQ(jugador1.getCardCount(), 2);
}

TEST(DeckTest, InicializaConCienCartas) {
    // Arrange & Act
    themind::Deck mazo_central;

    // Assert
    EXPECT_EQ(mazo_central.getRemainingCards(), 100);
}


TEST(GameSessionTest, JugadaCorrectaMantieneVidas) {
    themind::GameSession sesion(2);
    
    // Tiramos un 15 en una mesa vacía
    bool resultado = sesion.playCard(15);
    
    EXPECT_TRUE(resultado); // Esperamos que la jugada sea válida
    EXPECT_EQ(sesion.getLastCard(), 15); // La mesa debe tener el 15
    EXPECT_EQ(sesion.getLives(), 3); // No deberíamos haber perdido vidas
}

TEST(GameSessionTest, JugadaIncorrectaRestaUnaVida) {
    themind::GameSession sesion(2);
    
    sesion.playCard(50); // Alguien tira un 50 (válido)
    
    // Alguien se equivoca y tira un 20 cuando ya había un 50
    bool resultado = sesion.playCard(20); 
    
    EXPECT_FALSE(resultado); // Esperamos que la jugada sea reportada como inválida
    EXPECT_EQ(sesion.getLives(), 2); // ¡El equipo debió perder una vida!
}

TEST(GameSessionTest, UsarShurikenReduceLaCantidad) {
    themind::GameSession sesion(2); // 2 jugadores
    
    EXPECT_EQ(sesion.getShurikens(), 1); // Empezamos con 1
    
    bool usado = sesion.useShuriken();
    EXPECT_TRUE(usado);
    EXPECT_EQ(sesion.getShurikens(), 0); // Nos quedamos sin shurikens
    
    bool usado_de_nuevo = sesion.useShuriken();
    EXPECT_FALSE(usado_de_nuevo); // Ya no podemos usar más
}

TEST(GameSessionTest, SubirDeNivelAlJugarTodasLasCartas) {
    // 2 jugadores en el nivel 1 = deben jugar 2 cartas para pasar de nivel
    themind::GameSession sesion(2); 
    
    EXPECT_EQ(sesion.getLevel(), 1);
    
    sesion.playCard(10); // Carta 1
    sesion.playCard(20); // Carta 2 -> ¡Debería disparar el Level Up!
    
    EXPECT_EQ(sesion.getLevel(), 2);
    EXPECT_EQ(sesion.getLastCard(), 0); // La mesa debe limpiarse
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

