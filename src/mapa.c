#include <stdlib.h>
#include "mapa.h"

// Mapeamento de IDs (0..13):
//  0: cercadoPonta1 (topo-esquerda)
//  1: cercadoPonta2 (topo-direita)
//  2: cercadoPonta3 (baixo-esquerda)
//  3: cercadoPonta4 (baixo-direita)
//  4: cercadoEsquerda1  |  5: cercadoEsquerda2
//  6: cercadoDireita1   |  7: cercadoDireita2
//  8: cercadoCima1      |  9: cercadoCima2
// 10: cercadoBaixo1     | 11: cercadoBaixo2
// 12: grama1            | 13: grama2

static inline int escolher_id_tile(int i, int j, int L, int C) {
    const int lastI = L - 1;
    const int lastJ = C - 1;

    // Cantos
    if (i == 0     && j == 0)     return 0; // topo-esquerda
    if (i == 0     && j == lastJ) return 1; // topo-direita
    if (i == lastI && j == 0)     return 2; // baixo-esquerda
    if (i == lastI && j == lastJ) return 3; // baixo-direita

    // Bordas (excluindo cantos já tratados)
    if (j == 0) { // primeira coluna: revezar Esquerda1/Esquerda2 entre as pontas
        // entre i=1 .. lastI-1
        return (i & 1) ? 5 : 4; // i ímpar -> Esquerda2, par -> Esquerda1
    }
    if (j == lastJ) { // última coluna: revezar Direita1/Direita2
        return (i & 1) ? 7 : 6;
    }
    if (i == 0) { // primeira linha: revezar Cima1/Cima2
        return (j & 1) ? 9 : 8;
    }
    if (i == lastI) { // última linha: revezar Baixo1/Baixo2
        return (j & 1) ? 11 : 10;
    }

    // Miolo: grama alternando 12/13
    return ((i + j) & 1) ? 13 : 12;
}

static inline bool eh_cercado(int id_tile) {
    // 0..11 são cercados (cantos e bordas); 12..13 são gramas
    return id_tile <= 11;
}

Mapa **criar_mapa_encadeado(int L, int C) {
    Mapa **linhas = (Mapa **)malloc(L * sizeof(Mapa *));
    if (!linhas) return NULL;

    for (int i = 0; i < L; ++i) {
        linhas[i] = (Mapa *)calloc(C, sizeof(Mapa));
        if (!linhas[i]) {
            for (int k = 0; k < i; ++k) free(linhas[k]);
            free(linhas);
            return NULL;
        }
    }

    for (int i = 0; i < L; ++i) {
        for (int j = 0; j < C; ++j) {
            Mapa *no = &linhas[i][j];
            no->linha  = i;
            no->coluna = j;
            no->id_tile = escolher_id_tile(i, j, L, C);
            no->colisao = eh_cercado(no->id_tile);

            // Vizinhanca (setaremos completa logo abaixo)
            no->cima = no->baixo = no->esquerda = no->direita = NULL;
        }
    }

    // Liga vizinhos
    for (int i = 0; i < L; ++i) {
        for (int j = 0; j < C; ++j) {
            Mapa *no = &linhas[i][j];
            if (i > 0)     no->cima     = &linhas[i-1][j];
            if (i < L-1)   no->baixo    = &linhas[i+1][j];
            if (j > 0)     no->esquerda = &linhas[i][j-1];
            if (j < C-1)   no->direita  = &linhas[i][j+1];
        }
    }

    return linhas;
}

void destruir_mapa_encadeado(Mapa **linhas, int L) {
    if (!linhas) return;
    for (int i = 0; i < L; ++i) free(linhas[i]);
    free(linhas);
}
