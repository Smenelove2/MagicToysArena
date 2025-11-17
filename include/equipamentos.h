#ifndef EQUIPAMENTOS_H
#define EQUIPAMENTOS_H

#include <stddef.h>
#include "jogador.h"
#include "armadura.h"
#include "capacete.h"
#include "arma_principal.h"
#include "arma_secundaria.h"

typedef enum {
    CAT_ARMA_PRINCIPAL = 0,
    CAT_ARMA_SECUNDARIA,
    CAT_ARMADURA,
    CAT_CAPACETE,
    CAT_TOTAL
} CategoriaEquipamento;

void CarregarTexturasEquipamentos(void);
void DescarregarTexturasEquipamentos(void);
size_t Equipamento_QuantidadePorCategoria(CategoriaEquipamento cat);
const char *Equipamento_NomeCategoria(CategoriaEquipamento cat, size_t indice);
void Equipamento_DescricaoEStatus(CategoriaEquipamento cat, size_t indice,
                                  char *descricao, size_t tamDescricao,
                                  char *status, size_t tamStatus);
void Equipamento_AjustarIndice(CategoriaEquipamento cat, int delta, size_t indices[CAT_TOTAL]);
float AtualizarVidaJogadorComEquipamentos(Jogador *j, const Armadura *arm,
                                          const Capacete *cap, float vidaBase,
                                          ArmaPrincipal *armaPrincipal);

#endif
