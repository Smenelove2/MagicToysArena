#ifndef MAPA_H
#define MAPA_H
#include <stdbool.h>

typedef struct Mapa {
    int id_tile;
    bool colisao;
    int linha;
    int coluna;

    float posX;
    float posY;

    struct Mapa *cima;
    struct Mapa *baixo;
    struct Mapa *esquerda;
    struct Mapa *direita;
} Mapa;

Mapa **criar_mapa_encadeado(int L, int C, int tile_w, int tile_h);
void destruir_mapa_encadeado(Mapa **linhas, int L);

#endif
