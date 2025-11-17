#include "armadura.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define DIRETORIO_PADRAO_ARMADURAS "assets/Equipamentos/armadura/"

Armadura gArmaduras[] = {
    {
        .nome = "Roupa do Miranha",
        .descricao = "Impulsiona a velocidade, mas sacrifica parte da vida maxima.",
        .spriteBase = "RoupadoMiranha",
        .modificador = {
            .statusBeneficio = STATUS_VELOCIDADE,
            .valorBeneficio = 60,
            .statusPenalidade = STATUS_VIDA_MAXIMA,
            .valorPenalidade = 10,
        },
    },
    {
        .nome = "Casaco do Sully",
        .descricao = "Reforca a vida maxima, mas deixa o jogador mais lento.",
        .spriteBase = "CasacodoSully",
        .modificador = {
            .statusBeneficio = STATUS_VIDA_MAXIMA,
            .valorBeneficio = 20,
            .statusPenalidade = STATUS_VELOCIDADE,
            .valorPenalidade = 40,
        },
    },
    {
        .nome = "Vestido da Princesa Sofia",
        .descricao = "Aumenta a vida, porem reduz o dano direto das armas.",
        .spriteBase = "VestidodaPrincesaSofia",
        .modificador = {
            .statusBeneficio = STATUS_VIDA_MAXIMA,
            .valorBeneficio = 25,
            .statusPenalidade = STATUS_DANO,
            .valorPenalidade = 8,
        },
    },
    {
        .nome = "Armadura do Master Chief",
        .descricao = "Destina mais dano as armas, mas compromete a velocidade.",
        .spriteBase = "ArmaduradoMasterChief",
        .modificador = {
            .statusBeneficio = STATUS_DANO,
            .valorBeneficio = 12,
            .statusPenalidade = STATUS_VELOCIDADE,
            .valorPenalidade = 48,
        },
    },
};

const size_t gQuantidadeArmaduras = sizeof(gArmaduras) / sizeof(gArmaduras[0]);

Armadura *ObterArmaduraPorIndice(size_t indice) {
    if (indice >= gQuantidadeArmaduras) return NULL;
    return &gArmaduras[indice];
}

Armadura *ObterArmaduraPorNome(const char *nome) {
    if (!nome) return NULL;
    for (size_t i = 0; i < gQuantidadeArmaduras; ++i) {
        if (strcmp(gArmaduras[i].nome, nome) == 0) {
            return &gArmaduras[i];
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

const char *GerarNomeSpriteArmadura(const char *nome, char *buffer, int tamanhoBuffer) {
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

static const char *BaseArmadura(const Armadura *arm, char *buffer, size_t tam) {
    if (!arm) return NULL;
    if (arm->spriteBase && arm->spriteBase[0] != '\0') return arm->spriteBase;
    return GerarNomeSpriteArmadura(arm->nome, buffer, (int)tam);
}

static bool CarregarSpriteUnico(Texture2D *destino, const char *dir, const char *base, const char *sufixo) {
    char caminho[256];
    if (snprintf(caminho, sizeof(caminho), "%s%s%s.png", dir, base, sufixo) < 0) {
        return false;
    }

    Texture2D sprite = LoadTexture(caminho);
    if (sprite.id == 0) {
        return false;
    }

    *destino = sprite;
    return true;
}

bool CarregarSpritesArmadura(Armadura *arm, const char *diretorioSprites) {
    if (!arm) return false;

    char base[64];
    const char *nomeBase = BaseArmadura(arm, base, sizeof(base));
    if (!nomeBase) return false;

    const char *dir = diretorioSprites ? diretorioSprites : DIRETORIO_PADRAO_ARMADURAS;

    DescarregarSeCarregado(&arm->spriteIdle);
    DescarregarSeCarregado(&arm->spriteAndando1);
    DescarregarSeCarregado(&arm->spriteAndando2);

    bool okIdle = CarregarSpriteUnico(&arm->spriteIdle, dir, nomeBase, "Idle");
    bool okWalk1 = CarregarSpriteUnico(&arm->spriteAndando1, dir, nomeBase, "1");
    bool okWalk2 = CarregarSpriteUnico(&arm->spriteAndando2, dir, nomeBase, "2");

    if (!okIdle || !okWalk1 || !okWalk2) {
        DescarregarSeCarregado(&arm->spriteIdle);
        DescarregarSeCarregado(&arm->spriteAndando1);
        DescarregarSeCarregado(&arm->spriteAndando2);
        return false;
    }

    return true;
}

void DescarregarSpritesArmadura(Armadura *arm) {
    if (!arm) return;
    DescarregarSeCarregado(&arm->spriteIdle);
    DescarregarSeCarregado(&arm->spriteAndando1);
    DescarregarSeCarregado(&arm->spriteAndando2);
}

void DesenharArmadura(const Armadura *arm, Vector2 posicaoCentroJogador,
                      bool emMovimento, bool frameAlternado, float escala) {
    if (!arm) return;
    if (escala <= 0.0f) escala = 1.0f;

    const Texture2D *sprite = &arm->spriteIdle;
    if (emMovimento) {
        sprite = frameAlternado ? &arm->spriteAndando1 : &arm->spriteAndando2;
    }

    if (!sprite || sprite->id == 0) return;

    Vector2 posicao = {
        posicaoCentroJogador.x - (sprite->width * escala) / 2.0f,
        posicaoCentroJogador.y - (sprite->height * escala) / 2.0f
    };

    DrawTextureEx(*sprite, posicao, 0.0f, escala, WHITE);
}
