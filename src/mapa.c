#include <stdlib.h>
#include "mapa.h"

static inline int padrao_id(int i, int j, int L, int C) {
    const bool borda = (i == 0 || j == 0 || i == L-1 || j == C-1);
    if (borda) {
        return ((i + j) & 1) ? 1 : 0;
    } else {
        return ((i + j) & 1) ? 3 : 2;
    }
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
            no->linha = i;
            no->coluna = j;
            no->id_tile = padrao_id(i, j, L, C);
            no->colisao = (no->id_tile <= 1);
        }
    }
    
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
