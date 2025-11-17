#ifndef MAPA_H
#define MAPA_H

#include "raylib.h"
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
bool ConverterPosicaoParaIndice(float posX, float posY, int tileLargura, int tileAltura,
                                int maxLinhas, int maxColunas, int *outI, int *outJ);
bool TilePossuiColisao(Mapa **mapa, int linhas, int colunas, int i, int j);
void DesenharMapaVisivel(Camera2D *camera, int telaLargura, int telaAltura,
                         Mapa **mapa, int linhas, int colunas,
                         Texture2D *tiles, int quantidadeTiles,
                         int tileLargura, int tileAltura,
                         int idTileForaMapa);

#endif
