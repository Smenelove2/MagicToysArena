#ifndef ESTADO_HABILIDADE_H
#define ESTADO_HABILIDADE_H

#include "raylib.h"
#include <stdbool.h>
#include "arma_secundaria.h"

typedef struct {
    bool ativo;
    const ArmaSecundaria *dados;
    Vector2 centro;
    Vector2 direcao;
    bool segueJogador;
    float tempoRestante;
    float tempoDecorrido;
    bool impactoAplicado;
} EstadoArmaSecundaria;

#endif
