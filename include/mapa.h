#ifndef MAPA_H
#define MAPA_H
#include <stdbool.h>

typedef struct Mapa {
    int id_tile;        // 0..13 conforme mapeamento definido em mapa.c
    bool colisao;       // true para todos cercados (bordas/cantos), false p/ grama
    int linha;
    int coluna;

    struct Mapa *cima;
    struct Mapa *baixo;
    struct Mapa *esquerda;
    struct Mapa *direita;
} Mapa;

Mapa **criar_mapa_encadeado(int L, int C);
void destruir_mapa_encadeado(Mapa **linhas, int L);

#endif
