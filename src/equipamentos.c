#include "equipamentos.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

static const char *NomeStatusArmadura(TipoStatusArmadura status)
{
    switch (status) {
        case STATUS_VIDA_MAXIMA: return "Vida";
        case STATUS_VELOCIDADE: return "Velocidade";
        case STATUS_DANO: return "Dano";
        default: return "Status";
    }
}

static float CalcularRegeneracaoComCapacete(const Jogador *j, const Capacete *cap)
{
    float base = (j && j->regeneracaoBase > 0.0f) ? j->regeneracaoBase : 1.0f;
    float bonus = cap ? cap->modificador.bonusRegeneracaoVida : 0.0f;
    float valor = base * (1.0f + bonus);
    if (valor < 0.0f) valor = 0.0f;
    return valor;
}

void CarregarTexturasEquipamentos(void)
{
    for (size_t i = 0; i < gQuantidadeCapacetes; ++i) {
        if (!CarregarSpriteCapacete(&gCapacetes[i], NULL)) {
            printf("Aviso: nao foi possivel carregar sprite do capacete %s\n", gCapacetes[i].nome);
        }
    }
    for (size_t i = 0; i < gQuantidadeArmaduras; ++i) {
        if (!CarregarSpritesArmadura(&gArmaduras[i], NULL)) {
            printf("Aviso: nao foi possivel carregar sprites da armadura %s\n", gArmaduras[i].nome);
        }
    }
    for (size_t i = 0; i < gQuantidadeArmasPrincipais; ++i) {
        if (!CarregarSpritesArmaPrincipal(&gArmasPrincipais[i], NULL)) {
            printf("Aviso: nao foi possivel carregar sprites da arma %s\n", gArmasPrincipais[i].nome);
        }
    }
    for (size_t i = 0; i < gQuantidadeArmasSecundarias; ++i) {
        if (!CarregarSpritesArmaSecundaria(&gArmasSecundarias[i], NULL)) {
            printf("Aviso: nao foi possivel carregar sprites da habilidade %s\n", gArmasSecundarias[i].nome);
        }
    }
}

void DescarregarTexturasEquipamentos(void)
{
    for (size_t i = 0; i < gQuantidadeCapacetes; ++i) {
        DescarregarSpriteCapacete(&gCapacetes[i]);
    }
    for (size_t i = 0; i < gQuantidadeArmaduras; ++i) {
        DescarregarSpritesArmadura(&gArmaduras[i]);
    }
    for (size_t i = 0; i < gQuantidadeArmasPrincipais; ++i) {
        DescarregarSpritesArmaPrincipal(&gArmasPrincipais[i]);
    }
    for (size_t i = 0; i < gQuantidadeArmasSecundarias; ++i) {
        DescarregarSpritesArmaSecundaria(&gArmasSecundarias[i]);
    }
}

size_t Equipamento_QuantidadePorCategoria(CategoriaEquipamento cat)
{
    switch (cat) {
        case CAT_ARMA_PRINCIPAL: return gQuantidadeArmasPrincipais;
        case CAT_ARMA_SECUNDARIA: return gQuantidadeArmasSecundarias;
        case CAT_ARMADURA: return gQuantidadeArmaduras;
        case CAT_CAPACETE: return gQuantidadeCapacetes;
        default: return 0;
    }
}

const char *Equipamento_NomeCategoria(CategoriaEquipamento cat, size_t indice)
{
    switch (cat) {
        case CAT_ARMA_PRINCIPAL:
            return (indice < gQuantidadeArmasPrincipais) ? gArmasPrincipais[indice].nome : "N/A";
        case CAT_ARMA_SECUNDARIA:
            return (indice < gQuantidadeArmasSecundarias) ? gArmasSecundarias[indice].nome : "N/A";
        case CAT_ARMADURA:
            return (indice < gQuantidadeArmaduras) ? gArmaduras[indice].nome : "N/A";
        case CAT_CAPACETE:
            return (indice < gQuantidadeCapacetes) ? gCapacetes[indice].nome : "N/A";
        default:
            return "N/A";
    }
}

void Equipamento_DescricaoEStatus(CategoriaEquipamento cat, size_t indice,
                                  char *descricao, size_t tamDescricao,
                                  char *status, size_t tamStatus)
{
    if (descricao && tamDescricao > 0) descricao[0] = '\0';
    if (status && tamStatus > 0) status[0] = '\0';

    switch (cat) {
        case CAT_ARMA_PRINCIPAL:
            if (indice < gQuantidadeArmasPrincipais) {
                const ArmaPrincipal *arma = &gArmasPrincipais[indice];
                if (descricao) {
                    strncpy(descricao, arma->descricao, tamDescricao - 1);
                    descricao[tamDescricao - 1] = '\0';
                }
                if (status) {
                    snprintf(status, tamStatus, "Dano: %.0f | Alcance: %.0f | Recarga: %.1fs",
                             arma->danoBase, arma->alcanceMaximo, arma->tempoRecarga);
                }
            }
            break;
        case CAT_ARMA_SECUNDARIA:
            if (indice < gQuantidadeArmasSecundarias) {
                const ArmaSecundaria *hab = &gArmasSecundarias[indice];
                if (descricao) {
                    strncpy(descricao, hab->descricao, tamDescricao - 1);
                    descricao[tamDescricao - 1] = '\0';
                }
                if (status) {
                    strncpy(status, hab->status, tamStatus - 1);
                    status[tamStatus - 1] = '\0';
                }
            }
            break;
        case CAT_ARMADURA:
            if (indice < gQuantidadeArmaduras) {
                const Armadura *arm = &gArmaduras[indice];
                if (descricao) {
                    strncpy(descricao, arm->descricao, tamDescricao - 1);
                    descricao[tamDescricao - 1] = '\0';
                }
                if (status) {
                    snprintf(status, tamStatus, "+%d %s | -%d %s",
                             arm->modificador.valorBeneficio, NomeStatusArmadura(arm->modificador.statusBeneficio),
                             arm->modificador.valorPenalidade, NomeStatusArmadura(arm->modificador.statusPenalidade));
                }
            }
            break;
        case CAT_CAPACETE:
            if (indice < gQuantidadeCapacetes) {
                const Capacete *cap = &gCapacetes[indice];
                if (descricao) {
                    strncpy(descricao, cap->descricao, tamDescricao - 1);
                    descricao[tamDescricao - 1] = '\0';
                }
                if (status) {
                    char buffer[3][32];
                    int count = 0;
                    if (cap->modificador.bonusVidaMaxima != 0.0f && count < 3)
                        snprintf(buffer[count++], sizeof(buffer[0]), "+%.0f%% Vida", cap->modificador.bonusVidaMaxima * 100.0f);
                    if (cap->modificador.bonusRegeneracaoVida != 0.0f && count < 3)
                        snprintf(buffer[count++], sizeof(buffer[0]), "+%.0f%% Regen", cap->modificador.bonusRegeneracaoVida * 100.0f);
                    if (cap->modificador.bonusDanoArmas != 0.0f && count < 3)
                        snprintf(buffer[count++], sizeof(buffer[0]), "+%.0f%% Dano", cap->modificador.bonusDanoArmas * 100.0f);

                    status[0] = '\0';
                    for (int i = 0; i < count; ++i) {
                        strncat(status, buffer[i], tamStatus - strlen(status) - 1);
                        if (i < count - 1) strncat(status, " | ", tamStatus - strlen(status) - 1);
                    }
                }
            }
            break;
        default:
            break;
    }
}

void Equipamento_AjustarIndice(CategoriaEquipamento cat, int delta, size_t indices[CAT_TOTAL])
{
    size_t total = Equipamento_QuantidadePorCategoria(cat);
    if (total == 0) return;
    int atual = (int)indices[cat] + delta;
    while (atual < 0) atual += (int)total;
    atual %= (int)total;
    indices[cat] = (size_t)atual;
}

static float ClampPercent(float valor)
{
    if (valor < -0.95f) return -0.95f;
    return valor;
}

float AtualizarVidaJogadorComEquipamentos(Jogador *j, const Armadura *arm,
                                          const Capacete *cap, float vidaBase,
                                          ArmaPrincipal *armaPrincipal)
{
    if (!j) return 0.0f;
    if (vidaBase <= 0.0f) vidaBase = (j->vidaMaxima > 0.0f) ? j->vidaMaxima : 100.0f;
    float vidaFlat = 0.0f;
    float velFlat = 0.0f;
    float danoFlat = 0.0f;
    float reducaoRecarga = 0.0f;

    if (arm) {
        switch (arm->modificador.statusBeneficio) {
            case STATUS_VIDA_MAXIMA:
                vidaFlat += (float)arm->modificador.valorBeneficio;
                break;
            case STATUS_VELOCIDADE:
                velFlat += (float)arm->modificador.valorBeneficio;
                break;
            case STATUS_DANO:
                danoFlat += (float)arm->modificador.valorBeneficio;
                break;
            default: break;
        }
        switch (arm->modificador.statusPenalidade) {
            case STATUS_VIDA_MAXIMA:
                vidaFlat -= (float)arm->modificador.valorPenalidade;
                break;
            case STATUS_VELOCIDADE:
                velFlat -= (float)arm->modificador.valorPenalidade;
                break;
            case STATUS_DANO:
                danoFlat -= (float)arm->modificador.valorPenalidade;
                break;
            default: break;
        }
    }

    float vidaComArmadura = vidaBase + vidaFlat;
    if (vidaComArmadura < 1.0f) vidaComArmadura = 1.0f;

    if (cap) {
        reducaoRecarga += cap->modificador.reducaoTempoRecarga;
    }

    if (armaPrincipal) {
        float danoBase = armaPrincipal->danoBaseOriginal > 0.0f ? armaPrincipal->danoBaseOriginal : armaPrincipal->danoBase;
        danoBase += danoFlat;
        if (cap) {
            danoBase *= (1.0f + cap->modificador.bonusDanoArmas);
        }
        if (danoBase < 1.0f) danoBase = 1.0f;
        armaPrincipal->danoBase = danoBase;

        float tempoBase = armaPrincipal->tempoRecargaOriginal > 0.0f ? armaPrincipal->tempoRecargaOriginal : armaPrincipal->tempoRecarga;
        float fatorRecarga = 1.0f - ClampPercent(reducaoRecarga);
        if (fatorRecarga < 0.05f) fatorRecarga = 0.05f;
        armaPrincipal->tempoRecarga = tempoBase * fatorRecarga;
        if (armaPrincipal->tempoRecargaRestante > armaPrincipal->tempoRecarga) {
            armaPrincipal->tempoRecargaRestante = armaPrincipal->tempoRecarga;
        }
    }

    j->velocidade = j->velocidadeBase + velFlat;
    if (j->velocidade < 50.0f) j->velocidade = 50.0f;

    float novaMax = vidaComArmadura;
    if (cap && cap->modificador.bonusVidaMaxima != 0.0f) {
        novaMax *= (1.0f + cap->modificador.bonusVidaMaxima);
    }
    if (novaMax < 1.0f) novaMax = 1.0f;
    j->vidaMaxima = novaMax;
    if (j->vida > j->vidaMaxima) j->vida = j->vidaMaxima;

    return CalcularRegeneracaoComCapacete(j, cap);
}
