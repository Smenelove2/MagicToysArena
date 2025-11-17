#ifndef JOGO_H
#define JOGO_H

#include "raylib.h"
#include <stdbool.h>
#include "jogador.h"
#include "mapa.h"
#include "armadura.h"
#include "capacete.h"
#include "arma_principal.h"
#include "arma_secundaria.h"
#include "equipamentos.h"
#include "estado_habilidade.h"
#include "monstro.h"
#include "objeto.h"

#define MAX_OBJETOS_VOO 50

typedef struct {
    bool ativo;
    const ArmaPrincipal *arma;
    Vector2 origem;
    Vector2 posicao;
    Vector2 destino;
    Vector2 direcao;
    Vector2 velocidade;
    float tempoRestante;
} ProjetilRaygun;

typedef struct {
    ProjetilRaygun projetilRaygun;
    EstadoArmaSecundaria armaSecundaria;
    float cooldownArmaSecundaria;
    float regeneracaoAtual;
    bool pausado;
    bool solicitouRetornoMenu;
    EfeitoVisualArmaPrincipal efeitoArmaPrincipal;
    Monstro monstros[MAX_MONSTROS];
    int monstrosAtivos;
    float tempoSpawnMonstro;
    float intervaloSpawnMonstro;
    float tempoTotalJogo;
    int pontuacaoTotal;
    bool jogadorMorto;
    ObjetoLancavel objetosEmVoo[MAX_OBJETOS_VOO];
} EstadoJogo;

void JogoInicializar(EstadoJogo *estado, float regeneracaoBase);
void JogoReiniciar(EstadoJogo *estado,
                   Jogador *jogador,
                   Camera2D *camera,
                   Vector2 posInicial);

void JogoAtualizar(EstadoJogo *estado,
                   Jogador *jogador,
                   Camera2D *camera,
                   Mapa **mapa,
                   int linhasMapa,
                   int colunasMapa,
                   int tileLargura,
                   int tileAltura,
                   float dt,
                   Vector2 mouseNoMundo,
                   bool mouseClickEsq,
                   bool mouseClickDir,
                   bool escapePress,
                   const Armadura *armaduraAtual,
                   const Capacete *capaceteAtual,
                   ArmaPrincipal *armaPrincipalAtual,
                   const ArmaSecundaria *armaSecundariaAtual);

void JogoDesenhar(EstadoJogo *estado,
                  const Jogador *jogador,
                  const Camera2D *camera,
                  Mapa **mapa,
                  Texture2D *tiles,
                  int quantidadeTiles,
                  int idTileForaMapa,
                  int linhasMapa,
                  int colunasMapa,
                  int tileLargura,
                  int tileAltura,
                  int largura, int altura,
                  Font fonteNormal,
                  Font fonteBold,
                  const Armadura *armaduraAtual,
                  const Capacete *capaceteAtual,
                  const ArmaPrincipal *armaPrincipalAtual,
                  const ArmaSecundaria *armaSecundariaAtual,
                  Vector2 mousePos,
                  bool mouseClick);

void JogoLiberarRecursos(EstadoJogo *estado);

#endif
