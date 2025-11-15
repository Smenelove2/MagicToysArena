#ifndef OBJETO_H
#define OBJETO_H

#include "raylib.h"
#include <stdbool.h>

struct Jogador;

typedef struct ObjetoLancavel {
    Vector2 posicao;
    Vector2 direcao;
    float velocidade;
    float dano;
    bool ativo; 

    float tempoVida;
    
    Texture2D sprite; 
} ObjetoLancavel;

bool IniciarObjeto(ObjetoLancavel* o, const char* caminhoSprite);
void DescarregarObjeto(ObjetoLancavel* o);

void AtualizarObjeto(ObjetoLancavel* o, float dt);
void DesenharObjeto(const ObjetoLancavel* o);
bool VerificarColisaoObjetoJogador(ObjetoLancavel* o, struct Jogador* jogador);

#define MAX_TEMPO 5.0f

#endif 