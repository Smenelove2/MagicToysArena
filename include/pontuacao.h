#ifndef PONTUACAO_H
#define PONTUACAO_H

#include "raylib.h"
#include <stddef.h>
#include <stdbool.h>

#define MAX_ENTRADAS_LEADERBOARD 512

typedef struct {
    char nome[64];
    int pontuacao;
} EntradaLeaderboard;

typedef struct {
    EntradaLeaderboard entradas[MAX_ENTRADAS_LEADERBOARD];
    size_t quantidade;
} LeaderboardDados;

typedef struct {
    int pontuacaoFinal;
    char nome[32];
    int tamanho;
} CadastroPontuacao;

typedef struct {
    LeaderboardDados leaderboard;
    CadastroPontuacao cadastro;
} EstadoPontuacao;

typedef struct {
    bool voltarMenu;
} ResultadoLeaderboard;

typedef struct {
    bool pontuacaoSalva;
} ResultadoTelaPontuacao;

void PontuacaoInicializar(EstadoPontuacao *estado);
void PontuacaoPrepararCadastro(EstadoPontuacao *estado, int pontuacao);
void PontuacaoRecarregarArquivo(EstadoPontuacao *estado);
ResultadoLeaderboard PontuacaoDesenharLeaderboard(const EstadoPontuacao *estado,
                                                  Font fonteBold,
                                                  Vector2 mousePos, bool mouseClick,
                                                  int largura, int altura);
ResultadoTelaPontuacao PontuacaoDesenharTelaCadastro(EstadoPontuacao *estado,
                                                     Font fonteBold,
                                                     Vector2 mousePos, bool mouseClick,
                                                     int largura, int altura);

#endif
