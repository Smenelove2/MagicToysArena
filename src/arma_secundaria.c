#include "arma_secundaria.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define DIRETORIO_PADRAO_HABILIDADES "assets/Equipamentos/arma2/"

ArmaSecundaria gArmasSecundarias[] = {
    {
        .nome = "Tocha de Hades",
        .descricao = "Labaredas cercam o jogador causando dano continuo durante alguns segundos.",
        .status = "Dano: 30/s | Duracao: 3s | Raio: 220 | Recarga: 12s",
        .tipo = TIPO_ARMA_SECUNDARIA_AREA_PULSANTE,
        .dano = 30.0f,
        .raioOuAlcance = 220.0f,
        .duracao = 3.0f,
        .tempoRecarga = 12.0f,
        .spriteBase = "TochadeHades",
        .offsetFrame1 = (Vector2){0.0f, 0.0f},
        .offsetFrame2 = (Vector2){0.0f, 0.0f},
    },
    {
        .nome = "Secador Tunel 3000",
        .descricao = "Empurra inimigos em um cone a frente, causando dano leve.",
        .status = "Dano: 40 | Alcance: 260 | Recarga: 6s",
        .tipo = TIPO_ARMA_SECUNDARIA_CONE_EMPURRAO,
        .dano = 40.0f,
        .raioOuAlcance = 260.0f,
        .duracao = 0.2f,
        .tempoRecarga = 6.0f,
        .spriteBase = "SecadorTunel3000",
        .offsetFrame1 = (Vector2){-3.0f, 0.0f},
        .offsetFrame2 = (Vector2){-1.0f, 0.0f},
    },
    {
        .nome = "Escudo do Cap",
        .descricao = "Anula todo dano recebido por alguns segundos.",
        .status = "Duracao: 4s | Raio: 70 | Recarga: 7s",
        .tipo = TIPO_ARMA_SECUNDARIA_DEFESA_TEMPORARIA,
        .dano = 0.0f,
        .raioOuAlcance = 70.0f,
        .duracao = 4.0f,
        .tempoRecarga = 7.0f,
        .spriteBase = "EscudodoCap",
        .offsetFrame1 = (Vector2){0.0f, 0.0f},
        .offsetFrame2 = (Vector2){0.0f, 0.0f},
    },
    {
        .nome = "Remo da Moana",
        .descricao = "Cria uma area aquatica que reduz drasticamente a velocidade dos inimigos.",
        .status = "Duracao: 5s | Raio: 220 | Recarga: 10s",
        .tipo = TIPO_ARMA_SECUNDARIA_ZONA_LENTIDAO,
        .dano = 0.0f,
        .raioOuAlcance = 220.0f,
        .duracao = 5.0f,
        .tempoRecarga = 10.0f,
        .spriteBase = "RemodaMoana",
        .offsetFrame1 = (Vector2){0.0f, 0.0f},
        .offsetFrame2 = (Vector2){0.0f, 0.0f},
    },
};

const size_t gQuantidadeArmasSecundarias =
    sizeof(gArmasSecundarias) / sizeof(gArmasSecundarias[0]);

const ArmaSecundaria *ObterArmaSecundariaPorIndice(size_t indice) {
    if (indice >= gQuantidadeArmasSecundarias) return NULL;
    return &gArmasSecundarias[indice];
}

const ArmaSecundaria *ObterArmaSecundariaPorNome(const char *nome) {
    if (!nome) return NULL;
    for (size_t i = 0; i < gQuantidadeArmasSecundarias; ++i) {
        if (strcmp(gArmasSecundarias[i].nome, nome) == 0) {
            return &gArmasSecundarias[i];
        }
    }
    return NULL;
}

static void DescarregarSeCarregado(Texture2D *tex) {
    if (tex && tex->id != 0) {
        UnloadTexture(*tex);
        *tex = (Texture2D){0};
    }
}

const char *GerarNomeSpriteArmaSecundaria(const char *nome, char *buffer, int tamanhoBuffer) {
    if (!nome || !buffer || tamanhoBuffer <= 0) return NULL;

    int indice = 0;
    for (const char *p = nome; *p != '\0' && indice < tamanhoBuffer - 1; ++p) {
        if (*p == ' ') continue;
        char c = *p;
        buffer[indice++] = (char)tolower((unsigned char)c);
    }
    buffer[indice] = '\0';
    return buffer;
}

static const char *NomeArquivoArmaSecundaria(const ArmaSecundaria *hab, char *buffer, size_t tamanhoBuffer) {
    if (!hab) return NULL;
    if (hab->spriteBase && hab->spriteBase[0] != '\0') return hab->spriteBase;
    return GerarNomeSpriteArmaSecundaria(hab->nome, buffer, (int)tamanhoBuffer);
}

static bool TentarCarregarFrame(Texture2D *destino, const char *dir, const char *base, const char *sufixo) {
    char caminho[256];
    if (snprintf(caminho, sizeof(caminho), "%s%s%s.png", dir, base, sufixo) < 0) {
        return false;
    }

    Texture2D sprite = LoadTexture(caminho);
    if (sprite.id == 0) return false;

    *destino = sprite;
    return true;
}

bool CarregarSpritesArmaSecundaria(ArmaSecundaria *hab, const char *diretorio) {
    if (!hab) return false;

    char base[64];
    const char *nomeBase = NomeArquivoArmaSecundaria(hab, base, sizeof(base));
    if (!nomeBase) return false;

    const char *dir = diretorio ? diretorio : DIRETORIO_PADRAO_HABILIDADES;

    DescarregarSeCarregado(&hab->spriteIdleOuFrame1);
    DescarregarSeCarregado(&hab->spriteFrame2);

    bool ok1 = TentarCarregarFrame(&hab->spriteIdleOuFrame1, dir, nomeBase, "1");
    if (!ok1) ok1 = TentarCarregarFrame(&hab->spriteIdleOuFrame1, dir, nomeBase, "");
    if (!ok1) return false;

    if (!TentarCarregarFrame(&hab->spriteFrame2, dir, nomeBase, "2")) {
        DescarregarSeCarregado(&hab->spriteFrame2);
    }

    return true;
}

void DescarregarSpritesArmaSecundaria(ArmaSecundaria *hab) {
    if (!hab) return;
    DescarregarSeCarregado(&hab->spriteIdleOuFrame1);
    DescarregarSeCarregado(&hab->spriteFrame2);
}

void DesenharArmaSecundaria(const ArmaSecundaria *hab, Vector2 posicaoCentro,
                                  bool emMovimento, bool frameAlternado, float escala) {
    if (!hab || hab->spriteIdleOuFrame1.id == 0) return;
    if (escala <= 0.0f) escala = 1.0f;

    const Texture2D *sprite = &hab->spriteIdleOuFrame1;
    Vector2 offset = hab->offsetFrame1;

    if (emMovimento) {
        if (hab->spriteFrame2.id != 0) {
            sprite = frameAlternado ? &hab->spriteFrame2 : &hab->spriteIdleOuFrame1;
        }
        offset = frameAlternado ? hab->offsetFrame2 : hab->offsetFrame1;
    }

    Vector2 posicao = {
        posicaoCentro.x - (sprite->width * escala) / 2.0f + offset.x * escala,
        posicaoCentro.y - (sprite->height * escala) / 2.0f + offset.y * escala
    };

    DrawTextureEx(*sprite, posicao, 0.0f, escala, WHITE);
}
