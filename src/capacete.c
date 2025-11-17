#include "capacete.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define DIRETORIO_PADRAO_CAPACETES "assets/Equipamentos/capacetes/"

Capacete gCapacetes[] = {
    {
        .nome = "Cabelo da Rapunzel",
        .descricao = "Amplia a regeneracao de vida por segundo.",
        .spriteBase = "CabeloRapunzel",
        .modificador = {
            .bonusRegeneracaoVida = 0.35f,
        },
    },
    {
        .nome = "Chapeu do Gandalf",
        .descricao = "Potencializa o dano causado pelas armas.",
        .spriteBase = "ChapeuGandalf",
        .modificador = {
            .bonusDanoArmas = 0.30f,
        },
    },
    {
        .nome = "Panela do Menino Maluquinho",
        .descricao = "Aumenta a vida maxima em porcentagem.",
        .spriteBase = "PaneladoMeninoMaluquinho",
        .modificador = {
            .bonusVidaMaxima = 0.20f,
        },
    },
};

const size_t gQuantidadeCapacetes = sizeof(gCapacetes) / sizeof(gCapacetes[0]);

Capacete *ObterCapacetePorIndice(size_t indice) {
    if (indice >= gQuantidadeCapacetes) return NULL;
    return &gCapacetes[indice];
}

Capacete *ObterCapacetePorNome(const char *nome) {
    if (!nome) return NULL;
    for (size_t i = 0; i < gQuantidadeCapacetes; ++i) {
        if (strcmp(gCapacetes[i].nome, nome) == 0) {
            return &gCapacetes[i];
        }
    }
    return NULL;
}

const char *GerarNomeSpriteCapacete(const char *nome, char *buffer, int tamanhoBuffer) {
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

static const char *NomeArquivoCapacete(const Capacete *cap, char *buffer, size_t tamanhoBuffer) {
    if (!cap) return NULL;
    if (cap->spriteBase && cap->spriteBase[0] != '\0') return cap->spriteBase;
    return GerarNomeSpriteCapacete(cap->nome, buffer, (int)tamanhoBuffer);
}

bool CarregarSpriteCapacete(Capacete *cap, const char *diretorioSprites) {
    if (!cap) return false;

    char nomeSprite[64];
    const char *basename = NomeArquivoCapacete(cap, nomeSprite, sizeof(nomeSprite));
    if (!basename) return false;

    char caminho[256];
    const char *dir = diretorioSprites ? diretorioSprites : DIRETORIO_PADRAO_CAPACETES;
    if (snprintf(caminho, sizeof(caminho), "%s%s.png", dir, basename) < 0) return false;

    if (cap->sprite.id != 0) {
        UnloadTexture(cap->sprite);
        cap->sprite = (Texture2D){0};
    }

    Texture2D sprite = LoadTexture(caminho);
    if (sprite.id == 0) {
        return false;
    }
    cap->sprite = sprite;
    return true;
}

void DescarregarSpriteCapacete(Capacete *cap) {
    if (!cap) return;
    if (cap->sprite.id != 0) {
        UnloadTexture(cap->sprite);
        cap->sprite = (Texture2D){0};
    }
}

void DesenharCapacete(const Capacete *cap, Vector2 posicaoCentroJogador, float escala) {
    if (!cap || cap->sprite.id == 0) return;
    if (escala <= 0.0f) escala = 1.0f;

    const Texture2D sprite = cap->sprite;
    Vector2 posicao = {
        posicaoCentroJogador.x - (sprite.width * escala) / 2.0f,
        posicaoCentroJogador.y - (sprite.height * escala) / 2.0f
    };

    DrawTextureEx(sprite, posicao, 0.0f, escala, WHITE);
}
