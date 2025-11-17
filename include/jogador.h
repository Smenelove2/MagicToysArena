#ifndef JOGADOR_H
#define JOGADOR_H

#include "raylib.h"
#include <stdbool.h>

struct Mapa;

typedef struct Jogador {
    Vector2 posicao;
    float velocidade;
    float velocidadeBase;
    float vida;
    float vidaMaxima;
    float regeneracaoBase;

    Texture2D parado;
    Texture2D andando1;
    Texture2D andando2;

    float fpsAndar;
    float acumulador;
    bool alternarFrame;
    bool emMovimento;

    struct Mapa *noAtual;
} Jogador;

bool IniciarJogador(Jogador* j,
                    Vector2 posInicial,
                    float velocidade,
                    float fpsAndar,
                    float vida,
                    const char* caminhoParado,
                    const char* caminhoAndando1,
                    const char* caminhoAndando2);

void AtualizarJogador(Jogador* j, float dt);

void DesenharJogador(const Jogador* j);

void DescarregarJogador(Jogador* j);

Vector2 TamanhoJogador(const Jogador* j);

void AplicarColisaoPosicaoJogador(Jogador* j, Vector2 posAnterior,
                                  struct Mapa **mapa, int linhas, int colunas,
                                  int tileLargura, int tileAltura);

void AtualizarNoAtualJogador(Jogador* j, struct Mapa **mapa, int linhas, int colunas,
                             int tileLargura, int tileAltura);

#endif // JOGADOR_H
