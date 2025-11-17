#include "arma_principal.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define DIRETORIO_PADRAO_ARMAS_PRINCIPAIS "assets/Equipamentos/arma1/"

ArmaPrincipal gArmasPrincipais[] = {
    {
        .nome = "Espada de Diamante",
        .descricao = "Golpeia em um cone curto na direcao do clique.",
        .alcance = TIPO_ALCANCE_CORPO,
        .ataqueEmArea = true,
        .formatoArea = TIPO_AREA_CONE,
        .tipoDisparo = TIPO_DISPARO_AUTOMATICO,
        .alcanceMaximo = 150.0f,
        .danoBase = 20.0f,
        .tempoRecarga = 0.2f,
        .tempoRecargaRestante = 0.0f,
        .aberturaConeGraus = 80.0f,
        .spriteBase = "EspadadeDiamante",
        .offsetFrame1 = (Vector2){0.0f, 0.0f},
        .offsetFrame2 = (Vector2){0.0f, 0.0f},
        .corEfeito = (Color){70, 130, 255, 255},
        .danoBaseOriginal = 45.0f,
        .tempoRecargaOriginal = 0.2f,
    },
    {
        .nome = "RayGun",
        .descricao = "Dispara projeteis rapidos com dano massivo ao atingir o inimigo.",
        .alcance = TIPO_ALCANCE_DISTANCIA,
        .ataqueEmArea = false,
        .formatoArea = TIPO_AREA_NENHUMA,
        .tipoDisparo = TIPO_DISPARO_AUTOMATICO,
        .alcanceMaximo = 400.0f,
        .danoBase = 200.0f,
        .tempoRecarga = 0.4f,
        .tempoRecargaRestante = 0.0f,
        .aberturaConeGraus = 0.0f,
        .spriteBase = "Raygun",
        .offsetFrame1 = (Vector2){6.0f, 0.0f},
        .offsetFrame2 = (Vector2){4.0f, 0.0f},
        .larguraLinha = 24.0f,
        .corEfeito = (Color){60, 0, 90, 255},
        .corProjetil = (Color){70, 0, 110, 255},
        .raioProjetilVisual = 8.0f,
        .raioProjetilColisao = 22.0f,
        .danoBaseOriginal = 200.0f,
        .tempoRecargaOriginal = 0.4f,
    },
    {
        .nome = "Varinha da Fada Madrinha",
        .descricao = "Aplica dano direto no ponto clicado se estiver ao alcance.",
        .alcance = TIPO_ALCANCE_DISTANCIA,
        .ataqueEmArea = false,
        .formatoArea = TIPO_AREA_PONTO,
        .tipoDisparo = TIPO_DISPARO_POINT_CLICK,
        .alcanceMaximo = 450.0f,
        .danoBase = 100.0f,
        .tempoRecarga = 1.0f,
        .tempoRecargaRestante = 0.0f,
        .aberturaConeGraus = 0.0f,
        .spriteBase = "VarinhadaFadaMadrinha",
        .offsetFrame1 = (Vector2){0.0f, 0.0f},
        .offsetFrame2 = (Vector2){0.0f, 0.0f},
        .raioArea = 37.0f,
        .corEfeito = (Color){255, 160, 0, 255},
        .danoBaseOriginal = 100.0f,
        .tempoRecargaOriginal = 1.0f,
    },
    {
        .nome = "Laco da Verdade",
        .descricao = "Puxa uma linha de energia que causa dano em area em linha reta.",
        .alcance = TIPO_ALCANCE_CORPO,
        .ataqueEmArea = true,
        .formatoArea = TIPO_AREA_LINHA,
        .tipoDisparo = TIPO_DISPARO_AUTOMATICO,
        .alcanceMaximo = 200.0f,
        .danoBase = 40.0f,
        .tempoRecarga = 0.8f,
        .tempoRecargaRestante = 0.0f,
        .aberturaConeGraus = 0.0f,
        .spriteBase = "LacodaVerdade",
        .offsetFrame1 = (Vector2){0.0f, 0.0f},
        .offsetFrame2 = (Vector2){0.0f, 0.0f},
        .larguraLinha = 30.0f,
        .corEfeito = (Color){255, 140, 0, 255},
        .danoBaseOriginal = 50.0f,
        .tempoRecargaOriginal = 0.8f,
    },
};

const size_t gQuantidadeArmasPrincipais = sizeof(gArmasPrincipais) / sizeof(gArmasPrincipais[0]);

ArmaPrincipal *ObterArmaPrincipalPorIndice(size_t indice) {
    if (indice >= gQuantidadeArmasPrincipais) return NULL;
    return &gArmasPrincipais[indice];
}

ArmaPrincipal *ObterArmaPrincipalPorNome(const char *nome) {
    if (!nome) return NULL;
    for (size_t i = 0; i < gQuantidadeArmasPrincipais; ++i) {
        if (strcmp(gArmasPrincipais[i].nome, nome) == 0) {
            return &gArmasPrincipais[i];
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

const char *GerarNomeSpriteArmaPrincipal(const char *nome, char *buffer, int tamanhoBuffer) {
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

static const char *BaseArmaPrincipal(const ArmaPrincipal *arma, char *buffer, size_t tam) {
    if (!arma) return NULL;
    if (arma->spriteBase && arma->spriteBase[0] != '\0') return arma->spriteBase;
    return GerarNomeSpriteArmaPrincipal(arma->nome, buffer, (int)tam);
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

bool CarregarSpritesArmaPrincipal(ArmaPrincipal *arma, const char *diretorioSprites) {
    if (!arma) return false;

    char base[64];
    const char *nomeBase = BaseArmaPrincipal(arma, base, sizeof(base));
    if (!nomeBase) return false;

    const char *dir = diretorioSprites ? diretorioSprites : DIRETORIO_PADRAO_ARMAS_PRINCIPAIS;

    DescarregarSeCarregado(&arma->spriteIdleOuFrame1);
    DescarregarSeCarregado(&arma->spriteFrame2);

    bool ok1 = TentarCarregarFrame(&arma->spriteIdleOuFrame1, dir, nomeBase, "1");
    if (!ok1) {
        ok1 = TentarCarregarFrame(&arma->spriteIdleOuFrame1, dir, nomeBase, "");
    }

    bool ok2 = TentarCarregarFrame(&arma->spriteFrame2, dir, nomeBase, "2");

    if (!ok1) {
        DescarregarSeCarregado(&arma->spriteIdleOuFrame1);
        DescarregarSeCarregado(&arma->spriteFrame2);
        return false;
    }

    if (!ok2) {
        // deixa spriteFrame2 zerado para reutilizar o frame1 no desenho
        DescarregarSeCarregado(&arma->spriteFrame2);
    }

    return true;
}

void DescarregarSpritesArmaPrincipal(ArmaPrincipal *arma) {
    if (!arma) return;
    DescarregarSeCarregado(&arma->spriteIdleOuFrame1);
    DescarregarSeCarregado(&arma->spriteFrame2);
}

void DesenharArmaPrincipal(const ArmaPrincipal *arma, Vector2 posicaoCentroJogador,
                           bool emMovimento, bool frameAlternado, float escala) {
    if (!arma || arma->spriteIdleOuFrame1.id == 0) return;
    if (escala <= 0.0f) escala = 1.0f;

    const Texture2D *sprite = &arma->spriteIdleOuFrame1;
    Vector2 offset = arma->offsetFrame1;
    if (emMovimento) {
        if (arma->spriteFrame2.id != 0) {
            sprite = frameAlternado ? &arma->spriteIdleOuFrame1 : &arma->spriteFrame2;
        }
        offset = frameAlternado ? arma->offsetFrame1 : arma->offsetFrame2;
    }

    if (!sprite || sprite->id == 0) return;

    Vector2 posicao = {
        posicaoCentroJogador.x - (sprite->width * escala) / 2.0f + offset.x * escala,
        posicaoCentroJogador.y - (sprite->height * escala) / 2.0f + offset.y * escala
    };

    DrawTextureEx(*sprite, posicao, 0.0f, escala, WHITE);
}

static float ComprimentoVetor2(Vector2 v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

static Vector2 NormalizarVetor2(Vector2 v) {
    float len = ComprimentoVetor2(v);
    if (len <= 0.0001f) {
        return (Vector2){1.0f, 0.0f};
    }
    return (Vector2){ v.x / len, v.y / len };
}

void AtualizarArmaPrincipal(ArmaPrincipal *arma, float dt) {
    if (!arma || dt <= 0.0f) return;
    if (arma->tempoRecargaRestante > 0.0f) {
        arma->tempoRecargaRestante -= dt;
        if (arma->tempoRecargaRestante < 0.0f) arma->tempoRecargaRestante = 0.0f;
    }
}

bool PodeAtacarArmaPrincipal(const ArmaPrincipal *arma) {
    return arma && arma->tempoRecargaRestante <= 0.0f;
}

void GerarEfeitoArmaPrincipal(const ArmaPrincipal *arma, Vector2 origem, Vector2 destino,
                              Vector2 direcao, EfeitoVisualArmaPrincipal *efeito) {
    if (!arma || !efeito) return;

    efeito->ativo = true;
    efeito->formato = arma->formatoArea;
    efeito->ataqueEmArea = arma->ataqueEmArea;
    efeito->origem = origem;
    efeito->destino = destino;
    efeito->direcao = direcao;
    efeito->alcance = arma->alcanceMaximo;
    efeito->tempoRestante = 0.05f;
    float larguraPadrao = (arma->larguraLinha > 0.0f) ? arma->larguraLinha : 18.0f;
    efeito->larguraLinha = larguraPadrao;
    efeito->coneAberturaGraus = (arma->aberturaConeGraus > 0.0f) ? arma->aberturaConeGraus : 55.0f;
    efeito->raio = (arma->raioArea > 0.0f) ? arma->raioArea : 32.0f;
    efeito->cor = (arma->corEfeito.a != 0) ? arma->corEfeito : BLUE;

    switch (arma->formatoArea) {
        case TIPO_AREA_PONTO:
            efeito->raio = (arma->raioArea > 0.0f) ? arma->raioArea : 32.0f;
            break;
        case TIPO_AREA_LINHA:
        case TIPO_AREA_NENHUMA:
            efeito->larguraLinha = larguraPadrao;
            break;
        case TIPO_AREA_CONE:
            efeito->alcance = arma->alcanceMaximo;
            break;
        default:
            break;
    }
}

bool AcionarAtaqueArmaPrincipal(ArmaPrincipal *arma, Vector2 origem, Vector2 alvo,
                                EfeitoVisualArmaPrincipal *efeito) {
    if (!arma) return false;
    if (!PodeAtacarArmaPrincipal(arma)) return false;

    Vector2 delta = { alvo.x - origem.x, alvo.y - origem.y };
    float distancia = ComprimentoVetor2(delta);
    Vector2 direcao = NormalizarVetor2(delta);
    if (distancia <= 0.0001f) {
        direcao = (Vector2){1.0f, 0.0f};
        distancia = arma->alcanceMaximo;
    }

    if (arma->tipoDisparo == TIPO_DISPARO_POINT_CLICK && distancia > arma->alcanceMaximo) {
        return false;
    }

    float alcanceUtil = (arma->alcance == TIPO_ALCANCE_CORPO)
                        ? fminf(distancia, arma->alcanceMaximo)
                        : fminf(arma->alcanceMaximo, distancia > 0.0f ? distancia : arma->alcanceMaximo);

    if (arma->tipoDisparo != TIPO_DISPARO_POINT_CLICK && distancia < arma->alcanceMaximo) {
        alcanceUtil = arma->alcance == TIPO_ALCANCE_CORPO ? fminf(distancia, arma->alcanceMaximo)
                                                          : distancia;
    }

    Vector2 destino = origem;
    if (arma->tipoDisparo == TIPO_DISPARO_POINT_CLICK) {
        destino = alvo;
    } else {
        destino.x += direcao.x * alcanceUtil;
        destino.y += direcao.y * alcanceUtil;
    }

    arma->tempoRecargaRestante = arma->tempoRecarga;

    if (efeito) {
        GerarEfeitoArmaPrincipal(arma, origem, destino, direcao, efeito);
    }

    return true;
}

void AtualizarEfeitoArmaPrincipal(EfeitoVisualArmaPrincipal *efeito, float dt) {
    if (!efeito || !efeito->ativo) return;
    efeito->tempoRestante -= dt;
    if (efeito->tempoRestante <= 0.0f) {
        efeito->ativo = false;
    }
}

void DesenharEfeitoArmaPrincipal(const EfeitoVisualArmaPrincipal *efeito) {
    if (!efeito || !efeito->ativo) return;

    Color corFundo = ColorAlpha(efeito->cor, 0.35f);
    Color corLinha = ColorAlpha(efeito->cor, 0.85f);

    switch (efeito->formato) {
        case TIPO_AREA_CONE: {
            float angulo = atan2f(efeito->direcao.y, efeito->direcao.x) * (180.0f / (float)PI);
            float inicio = angulo - efeito->coneAberturaGraus * 0.5f;
            float fim = angulo + efeito->coneAberturaGraus * 0.5f;
            DrawCircleSector(efeito->origem, efeito->alcance, inicio, fim, 0, corFundo);
            DrawCircleSectorLines(efeito->origem, efeito->alcance, inicio, fim, 0, corLinha);
        } break;
        case TIPO_AREA_LINHA:
        case TIPO_AREA_NENHUMA: {
            DrawLineEx(efeito->origem, efeito->destino, efeito->larguraLinha, corLinha);
        } break;
        case TIPO_AREA_PONTO: {
            DrawCircleV(efeito->destino, efeito->raio, corFundo);
            DrawCircleLines((int)efeito->destino.x, (int)efeito->destino.y, efeito->raio, corLinha);
        } break;
        default:
            break;
    }
}
