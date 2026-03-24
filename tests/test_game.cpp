#include <gtest/gtest.h>
#include "../src/core/game_logic.hpp" 

// Los tests se dividen en 3 fases: Arrange (Preparar), Act (Actuar), Assert (Comprobar)
TEST(JugadorTest, RecibirCartasAumentaElConteo) {
    themind::player jugador1("Ada Lovelace");
    jugador1.receiveCard(42);
    jugador1.receiveCard(15);
    EXPECT_EQ(jugador1.getCardCount(), 2);
}

TEST(DeckTest, InicializaConCienCartas) {
    themind::Deck mazo_central;
    EXPECT_EQ(mazo_central.getRemainingCards(), 100);
}

TEST(GameSessionTest, JugadaCorrectaMantieneVidas) {
    themind::GameSession sesion(2);
    
    // ARRANGE: Le damos legalmente la carta 15 al Jugador 0
    sesion.getPlayer(0).receiveCard(15);
    
    // ACT: El Jugador 0 tira el 15
    themind::PlayResult resultado = sesion.playCard(0, 15);
    
    EXPECT_EQ(resultado, themind::PlayResult::ValidPlay); 
    EXPECT_EQ(sesion.getLastCard(), 15); 
    EXPECT_EQ(sesion.getLives(), 3); 
}

TEST(GameSessionTest, JugadaIncorrectaRestaUnaVida) {
    themind::GameSession sesion(2);
    
    // ARRANGE: Preparamos las manos
    sesion.getPlayer(0).receiveCard(50);
    sesion.getPlayer(1).receiveCard(20);
    
    sesion.playCard(0, 50); // Jugador 0 tira 50 (válido)
    
    // ACT: Jugador 1 se equivoca y tira 20
    themind::PlayResult resultado = sesion.playCard(1, 20); 
    
    EXPECT_EQ(resultado, themind::PlayResult::LostLife); 
    EXPECT_EQ(sesion.getLives(), 2); 
}

TEST(GameSessionTest, UsarShurikenReduceLaCantidad) {
    themind::GameSession sesion(2); 
    EXPECT_EQ(sesion.getShurikens(), 1); 
    
    bool usado = sesion.useShuriken();
    EXPECT_TRUE(usado);
    EXPECT_EQ(sesion.getShurikens(), 0); 
    
    bool usado_de_nuevo = sesion.useShuriken();
    EXPECT_FALSE(usado_de_nuevo); 
}

TEST(GameSessionTest, SubirDeNivelAlJugarTodasLasCartas) {
    themind::GameSession sesion(2); 
    EXPECT_EQ(sesion.getLevel(), 1);
    
    sesion.getPlayer(0).receiveCard(10);
    sesion.getPlayer(1).receiveCard(20);
    
    sesion.playCard(0, 10); // Carta 1
    
    // ACT: Jugador 1 tira su 20 y completa el nivel
    auto resultado = sesion.playCard(1, 20); 
    
    EXPECT_EQ(resultado, themind::PlayResult::LevelUp);
    EXPECT_EQ(sesion.getLevel(), 2);
    EXPECT_EQ(sesion.getLastCard(), 0); 
}

TEST(GameSessionTest, PerderTodasLasVidasEsGameOver) {
    themind::GameSession sesion(2); 
    
    // ARRANGE: Le damos al Jugador 1 las cartas para que pueda equivocarse
    sesion.getPlayer(0).receiveCard(50);
    sesion.getPlayer(1).receiveCard(40);
    sesion.getPlayer(1).receiveCard(30);
    sesion.getPlayer(1).receiveCard(20);
    
    sesion.playCard(0, 50); // Preparamos la mesa en 50
    
    // Nos equivocamos 3 veces seguidas tirando cartas menores (que sí tenemos en la mano)
    EXPECT_EQ(sesion.playCard(1, 40), themind::PlayResult::LostLife); // Quedan 2 vidas
    EXPECT_EQ(sesion.playCard(1, 30), themind::PlayResult::LostLife); // Queda 1 vida
    
    // El golpe de gracia
    auto resultado_final = sesion.playCard(1, 20);
    EXPECT_EQ(resultado_final, themind::PlayResult::GameOver); 
    EXPECT_EQ(sesion.getLives(), 0);
}