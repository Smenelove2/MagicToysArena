#ifndef MONSTRO_H
#define MONSTRO_H

#include "raylib.h"
#include <stdbool.h>

struct ObjetoLancavel;
struct MonstroInfo;
struct Jogador;

// Enum para tipos de monstros
typedef enum
{
    MONSTRO_ESQUELETO = 0,
    MONSTRO_ZOMBIE = 1,
    MONSTRO_TIBBERS = 2,
    MONSTRO_IT = 3,
    MONSTRO_CHUCKY = 4,
    MONSTRO_HEROBRINE = 5,
    MONSTRO_RANDALL = 6,
    MONSTRO_SUPREMO = 7,
    MONSTRO_TIPOS_COUNT = 8
} TipoMonstro;

typedef struct Monstro
{
    Vector2 posicao;
    float vida;
    float vidaMaxima;
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
    float cooldownAtaque;
    float acumuladorAtaque;
    float danoContato;
    int pontuacao;

    float cooldownArremesso;
    float acumuladorArremesso;

    struct ObjetoLancavel *objeto;
} Monstro;

// Define número máximo de monstros simultâneos
#define MAX_MONSTROS 100

bool IniciarMonstro(Monstro *m,
                    Vector2 posInicial,
                    const struct MonstroInfo *info);

void DescarregarMonstro(Monstro *m);

void CarregarAssetsMonstro(Monstro *m);

bool TentarLancarObjeto(Monstro *m, float dt, Vector2 alvo);

void AtualizarMonstro(Monstro *m, float dt);

void DesenharMonstro(const Monstro *m);

// IA: Move o monstro constantemente em direção ao jogador
void IAAtualizarMonstro(Monstro *m, struct Jogador *jogador, float dt);

// Colisão: Retorna true se o monstro colidiu com o jogador
bool VerificarColisaoMonstroJogador(Monstro *m, struct Jogador *jogador);

Vector2 GerarMonstros(struct Jogador *jogador, int mapL, int mapC, int tileW, int tileH);

#endif
