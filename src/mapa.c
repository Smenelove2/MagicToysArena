#include <stdlib.h>
#include "mapa.h"

static inline int escolher_id_tile(int i, int j, int L, int C)
{
    const int lastI = L - 1;
    const int lastJ = C - 1;

    if (i == 0     && j == 0)     return 0;
    if (i == 0     && j == lastJ) return 1;
    if (i == lastI && j == 0)     return 2;
    if (i == lastI && j == lastJ) return 3;

    if (j == 0) return (i & 1) ? 5 : 4;
    if (j == lastJ) return (i & 1) ? 7 : 6;
    if (i == 0) return (j & 1) ? 9 : 8;
    if (i == lastI) return (j & 1) ? 11 : 10;

    return ((i + j) & 1) ? 13 : 12;
}

static inline bool eh_cercado(int id_tile)
{
    return id_tile <= 11;
}

Mapa **criar_mapa_encadeado(int L, int C, int tile_w, int tile_h)
{
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
            no->posX = (float)j * (float)tile_w;
            no->posY = (float)i * (float)tile_h;
            no->cima = no->baixo = no->esquerda = no->direita = NULL;
        }
    }

    for (int i = 0; i < L; ++i) {
        for (int j = 0; j < C; ++j) {
            Mapa *no = &linhas[i][j];
            if (i > 0)     no->cima     = &linhas[i-1][j];
            if (i < L - 1) no->baixo    = &linhas[i+1][j];
            if (j > 0)     no->esquerda = &linhas[i][j-1];
            if (j < C - 1) no->direita  = &linhas[i][j+1];
        }
    }

    return linhas;
}

void destruir_mapa_encadeado(Mapa **linhas, int L)
{
    if (!linhas) return;
    for (int i = 0; i < L; ++i) free(linhas[i]);
    free(linhas);
}

bool ConverterPosicaoParaIndice(float posX, float posY, int tileLargura, int tileAltura,
                                int maxLinhas, int maxColunas, int *outI, int *outJ)
{
    if (tileLargura <= 0 || tileAltura <= 0) return false;
    if (posX < 0.0f || posY < 0.0f) return false;

    int j = (int)(posX / (float)tileLargura);
    int i = (int)(posY / (float)tileAltura);

    if (i < 0 || j < 0 || i >= maxLinhas || j >= maxColunas) return false;

    if (outI) *outI = i;
    if (outJ) *outJ = j;
    return true;
}

bool TilePossuiColisao(Mapa **mapa, int linhas, int colunas, int i, int j)
{
    if (!mapa) return true;
    if (i < 0 || j < 0 || i >= linhas || j >= colunas) return true;
    return mapa[i][j].colisao;
}

void DesenharMapaVisivel(Camera2D *camera, int telaLargura, int telaAltura,
                         Mapa **mapa, int linhas, int colunas,
                         Texture2D *tiles, int quantidadeTiles,
                         int tileLargura, int tileAltura,
                         int idTileForaMapa)
{
    if (!camera || !mapa || !tiles || quantidadeTiles <= 0) return;
    if (tileLargura <= 0 || tileAltura <= 0) return;

    Vector2 topoEsquerdo = GetScreenToWorld2D((Vector2){0, 0}, *camera);
    Vector2 fundoDireito = GetScreenToWorld2D((Vector2){(float)telaLargura, (float)telaAltura}, *camera);

    int jIni = (int)(topoEsquerdo.x / tileLargura) - 1;
    int iIni = (int)(topoEsquerdo.y / tileAltura) - 1;
    int jFim = (int)(fundoDireito.x / tileLargura) + 1;
    int iFim = (int)(fundoDireito.y / tileAltura) + 1;

    int ruaIndex = idTileForaMapa;
    if (ruaIndex < 0 || ruaIndex >= quantidadeTiles) ruaIndex = quantidadeTiles - 1;

    for (int i = iIni; i <= iFim; ++i) {
        for (int j = jIni; j <= jFim; ++j) {
            bool dentro = (i >= 0 && j >= 0 && i < linhas && j < colunas);
            int id = dentro ? mapa[i][j].id_tile : ruaIndex;
            if (id < 0 || id >= quantidadeTiles) id = ruaIndex;

            float posX = (float)j * tileLargura;
            float posY = (float)i * tileAltura;
            Vector2 pos = { posX, posY };
            if (!dentro && id == ruaIndex) {
                DrawTextureEx(tiles[id], pos, 0.0f, 2.0f, WHITE);
            } else {
                DrawTextureV(tiles[id], pos, WHITE);
            }
        }
    }
}
