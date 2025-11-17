#include "pontuacao.h"
#include "ui_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARQUIVO_PONTUACOES "pontuacoes.txt"

static int CompararPontuacao(const void *a, const void *b)
{
    const EntradaLeaderboard *ea = (const EntradaLeaderboard *)a;
    const EntradaLeaderboard *eb = (const EntradaLeaderboard *)b;
    if (ea->pontuacao == eb->pontuacao) return 0;
    return (eb->pontuacao > ea->pontuacao) ? 1 : -1;
}

void PontuacaoInicializar(EstadoPontuacao *estado)
{
    if (!estado) return;
    memset(estado, 0, sizeof(*estado));
}

void PontuacaoPrepararCadastro(EstadoPontuacao *estado, int pontuacao)
{
    if (!estado) return;
    estado->cadastro.pontuacaoFinal = pontuacao;
    estado->cadastro.nome[0] = '\0';
    estado->cadastro.tamanho = 0;
}

void PontuacaoRecarregarArquivo(EstadoPontuacao *estado)
{
    if (!estado) return;
    estado->leaderboard.quantidade = 0;
    FILE *arquivo = fopen(ARQUIVO_PONTUACOES, "r");
    if (!arquivo) return;

    char linha[256];
    while (fgets(linha, sizeof(linha), arquivo)) {
        char *p = strchr(linha, '\n');
        if (p) *p = '\0';
        p = strchr(linha, '\r');
        if (p) *p = '\0';
        if (linha[0] == '\0') continue;
        char *sep = strchr(linha, ';');
        if (!sep) continue;
        *sep = '\0';
        char *nome = linha;
        char *pontStr = sep + 1;
        while (*nome == ' ') nome++;
        while (*pontStr == ' ') pontStr++;
        if (*nome == '\0') continue;
        EntradaLeaderboard entrada = {0};
        strncpy(entrada.nome, nome, sizeof(entrada.nome) - 1);
        entrada.pontuacao = atoi(pontStr);
        estado->leaderboard.entradas[estado->leaderboard.quantidade++] = entrada;
        if (estado->leaderboard.quantidade >= MAX_ENTRADAS_LEADERBOARD) break;
    }
    fclose(arquivo);

    if (estado->leaderboard.quantidade > 1) {
        qsort(estado->leaderboard.entradas,
              estado->leaderboard.quantidade,
              sizeof(EntradaLeaderboard),
              CompararPontuacao);
    }
}

static bool SalvarPontuacaoEmArquivo(const CadastroPontuacao *cadastro)
{
    if (!cadastro || cadastro->tamanho <= 0) return false;
    FILE *arquivo = fopen(ARQUIVO_PONTUACOES, "a");
    if (!arquivo) return false;
    fprintf(arquivo, "%s;%d\n", cadastro->nome, cadastro->pontuacaoFinal);
    fclose(arquivo);
    return true;
}

ResultadoLeaderboard PontuacaoDesenharLeaderboard(const EstadoPontuacao *estado,
                                                  Font fonteBold,
                                                  Vector2 mousePos, bool mouseClick,
                                                  int largura, int altura)
{
    ResultadoLeaderboard resultado = { .voltarMenu = false };
    if (!estado) return resultado;

    float margem = 20.0f;
    Rectangle btnVoltar = { margem, margem, 150.0f, 50.0f };
    if (UI_BotaoTexto(btnVoltar, "Voltar", mousePos, mouseClick,
                      (Color){80, 80, 120, 255}, (Color){180, 180, 230, 255},
                      fonteBold, true)) {
        resultado.voltarMenu = true;
    }

    const char *titulo = "Top 10 Pontuacoes";
    float tituloTam = UI_AjustarTamanhoFonte(48.0f);
    Vector2 medidaTitulo = MeasureTextEx(fonteBold, titulo, tituloTam, 1.0f);
    DrawTextEx(fonteBold, titulo,
               (Vector2){ largura / 2.0f - medidaTitulo.x / 2.0f, altura * 0.1f },
               tituloTam, 1.0f, WHITE);

    Rectangle painel = { largura * 0.18f, altura * 0.2f, largura * 0.64f, altura * 0.7f };
    DrawRectangleRounded(painel, 0.08f, 8, (Color){22, 22, 40, 240});

    if (estado->leaderboard.quantidade == 0) {
        const char *msg = "Nenhuma pontuacao registrada.";
        float msgTam = UI_AjustarTamanhoFonte(26.0f);
        Vector2 medida = MeasureTextEx(fonteBold, msg, msgTam, 1.0f);
        DrawTextEx(fonteBold, msg,
                   (Vector2){ painel.x + painel.width / 2 - medida.x / 2,
                              painel.y + painel.height / 2 - medida.y / 2 },
                   msgTam, 1.0f, LIGHTGRAY);
        return resultado;
    }

    float linhaAltura = 48.0f * UI_GetEscala();
    float inicioY = painel.y + 40.0f;
    size_t maxEntradas = estado->leaderboard.quantidade;
    if (maxEntradas > 10) maxEntradas = 10;
    for (size_t i = 0; i < maxEntradas; ++i) {
        const EntradaLeaderboard *entrada = &estado->leaderboard.entradas[i];
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "%2zu. %-20s", i + 1, entrada->nome);
        float textoTam = UI_AjustarTamanhoFonte(28.0f);
        DrawTextEx(fonteBold, buffer,
                   (Vector2){ painel.x + 40.0f, inicioY },
                   textoTam, 1.0f, WHITE);
        char pontos[64];
        snprintf(pontos, sizeof(pontos), "%d", entrada->pontuacao);
        Vector2 medida = MeasureTextEx(fonteBold, pontos, textoTam, 1.0f);
        DrawTextEx(fonteBold, pontos,
                   (Vector2){ painel.x + painel.width - medida.x - 40.0f, inicioY },
                   textoTam, 1.0f, GOLD);
        inicioY += linhaAltura;
    }
    return resultado;
}

ResultadoTelaPontuacao PontuacaoDesenharTelaCadastro(EstadoPontuacao *estado,
                                                     Font fonteBold,
                                                     Vector2 mousePos, bool mouseClick,
                                                     int largura, int altura)
{
    ResultadoTelaPontuacao resultado = { .pontuacaoSalva = false };
    if (!estado) return resultado;
    CadastroPontuacao *cadastro = &estado->cadastro;

    const char *titulo = TextFormat("Sua pontuacao foi: %d", cadastro->pontuacaoFinal);
    float tituloTam = UI_AjustarTamanhoFonte(46.0f);
    Vector2 medidaTitulo = MeasureTextEx(fonteBold, titulo, tituloTam, 1.0f);
    DrawTextEx(fonteBold, titulo,
               (Vector2){ largura / 2.0f - medidaTitulo.x / 2.0f, altura * 0.25f },
               tituloTam, 1.0f, WHITE);

    const char *instrucao = "Digite seu nome:";
    float instrTam = UI_AjustarTamanhoFonte(28.0f);
    Vector2 medidaInstrucao = MeasureTextEx(fonteBold, instrucao, instrTam, 1.0f);
    DrawTextEx(fonteBold, instrucao,
               (Vector2){ largura / 2.0f - medidaInstrucao.x / 2.0f, altura * 0.4f },
               instrTam, 1.0f, LIGHTGRAY);

    Rectangle campo = {
        largura / 2.0f - 260.0f,
        altura * 0.48f,
        520.0f,
        70.0f
    };
    DrawRectangleRounded(campo, 0.2f, 8, (Color){30, 30, 60, 230});
    DrawRectangleRoundedLines(campo, 0.2f, 8, (Color){120, 120, 200, 255});

    int tecla = GetCharPressed();
    while (tecla > 0) {
        if (tecla >= 32 && tecla <= 126 && cadastro->tamanho < (int)sizeof(cadastro->nome) - 1) {
            cadastro->nome[cadastro->tamanho++] = (char)tecla;
            cadastro->nome[cadastro->tamanho] = '\0';
        }
        tecla = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && cadastro->tamanho > 0) {
        cadastro->nome[--cadastro->tamanho] = '\0';
    }

    float nomeTam = UI_AjustarTamanhoFonte(32.0f);
    const char *textoNome = (cadastro->tamanho > 0) ? cadastro->nome : "Digite aqui...";
    Color corTexto = (cadastro->tamanho > 0) ? WHITE : (Color){180, 180, 200, 255};
    Vector2 medidaNome = MeasureTextEx(fonteBold, textoNome, nomeTam, 1.0f);
    DrawTextEx(fonteBold, textoNome,
               (Vector2){ campo.x + 20.0f, campo.y + campo.height / 2.0f - medidaNome.y / 2.0f },
               nomeTam, 1.0f, corTexto);

    Rectangle btnSalvar = {
        largura / 2.0f - 150.0f,
        campo.y + campo.height + 40.0f,
        300.0f,
        60.0f
    };
    bool permitirSalvar = cadastro->tamanho > 0;
    Color corNormal = permitirSalvar ? (Color){80, 150, 80, 255} : (Color){80, 80, 80, 255};
    Color corHover = permitirSalvar ? (Color){140, 200, 140, 255} : (Color){110, 110, 110, 255};
    bool cliqueSalvar = permitirSalvar &&
        UI_BotaoTexto(btnSalvar, "Salvar pontuacao", mousePos, mouseClick, corNormal, corHover, fonteBold, true);
    bool enterSalvar = permitirSalvar && IsKeyPressed(KEY_ENTER);
    if ((cliqueSalvar || enterSalvar) && permitirSalvar) {
        if (SalvarPontuacaoEmArquivo(cadastro)) {
            resultado.pontuacaoSalva = true;
        }
    }
    return resultado;
}
