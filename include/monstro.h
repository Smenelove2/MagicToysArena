#ifndef MONSTRO_H
#define MONSTRO_H

#include "raylib.h"
#include <stdbool.h>

struct ObjetoLancavel;
struct Jogador;

// Enum para tipos de monstros
typedef enum
{
    MONSTRO_ESQUELETO = 0,
    MONSTRO_ZOMBIE = 1,
    MONSTRO_URSO = 2,
    MONSTRO_IT = 3,
    MONSTRO_TIPOS_COUNT = 4
} TipoMonstro;

typedef struct Monstro
{
    Vector2 posicao;
    float vida;
    float velocidade;
    TipoMonstro tipo;
    bool ativo; // Indica se o monstro está vivo/ativo

    Texture2D sprite1;
    Texture2D sprite2;
    Texture2D sprite3;

    float fpsAnimacao;
    float acumulador;
    int frameAtual;

    float alcanceAtaque;
    float raioDeteccao; // Raio em pixels para detectar o jogador
    float cooldownAtaque;
    float acumuladorAtaque;

    struct ObjetoLancavel *objeto;
} Monstro;

// Define número máximo de monstros simultâneos
#define MAX_MONSTROS 10

bool IniciarMonstro(Monstro *m,
                    Vector2 posInicial,
                    float vida,
                    float velocidade,
                    float fpsAnimacao,
                    float alcanceAtaque,
                    float cooldownAtaque,
                    TipoMonstro tipo);

void DescarregarMonstro(Monstro *m);

void CarregarAssetsMonstro(Monstro *m);

void AtualizarMonstro(Monstro *m, float dt);

void DesenharMonstro(const Monstro *m);

// IA: Move o monstro em direção ao jogador se estiver no raio de detecção
void IAAtualizarMonstro(Monstro *m, struct Jogador *jogador, float dt);

// Colisão: Retorna true se o monstro colidiu com o jogador
bool VerificarColisaoMonstroJogador(Monstro *m, struct Jogador *jogador);

Vector2 GerarMonstros(struct Jogador *jogador, int mapL, int mapC, int tileW, int tileH);

#endif