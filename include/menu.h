#ifndef MENU_H
#define MENU_H

#include "raylib.h"
#include <stdbool.h>
#include "jogador.h"
#include "armadura.h"
#include "capacete.h"
#include "arma_principal.h"
#include "arma_secundaria.h"
#include "equipamentos.h"

typedef struct {
    CategoriaEquipamento categoriaSelecionada;
    size_t indicesSelecionados[CAT_TOTAL];
} EstadoMenu;

typedef enum {
    MENU_ACAO_NENHUMA = 0,
    MENU_ACAO_JOGAR,
    MENU_ACAO_VOLTAR,
    MENU_ACAO_SAIR,
    MENU_ACAO_LEADERBOARD
} AcaoMenu;

typedef struct {
    AcaoMenu acao;
    Armadura *armadura;
    Capacete *capacete;
    ArmaPrincipal *armaPrincipal;
    ArmaSecundaria *armaSecundaria;
} ResultadoMenu;

void MenuInicializarEstado(EstadoMenu *estado);
AcaoMenu MenuDesenharTelaInicial(const EstadoMenu *estado, Vector2 mousePos, bool mouseClick,
                                 Font fonteBold, int largura, int altura);
ResultadoMenu MenuDesenharTelaConfig(EstadoMenu *estado,
                                     Vector2 mousePos, bool mouseClick,
                                     Font fonteNormal, Font fonteBold,
                                     const Jogador *jogadorBase,
                                     int largura, int altura);

#endif
