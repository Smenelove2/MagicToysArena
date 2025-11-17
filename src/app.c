#include "app.h"
#include "raylib.h"
#include "jogador.h"
#include "mapa.h"
#include "menu.h"
#include "jogo.h"
#include "equipamentos.h"
#include "ui_utils.h"
#include "pontuacao.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define MAP_L 65
#define MAP_C 65
#define TOTAL_TILES 15
#define ID_TILE_GRAMA_BASE 12
#define ID_TILE_RUA (TOTAL_TILES - 1)

static const float LARGURA_BASE_UI = 1280.0f;
static const float ALTURA_BASE_UI = 720.0f;

static const char *CAMINHOS_TILES[TOTAL_TILES] = {
    "assets/tiles/cercadoPonta1.png",
    "assets/tiles/cercadoPonta2.png",
    "assets/tiles/cercadoPonta3.png",
    "assets/tiles/cercadoPonta4.png",
    "assets/tiles/cercadoEsquerda1.png",
    "assets/tiles/cercadoEsquerda2.png",
    "assets/tiles/cercadoDireita1.png",
    "assets/tiles/cercadoDireita2.png",
    "assets/tiles/cercadoCima1.png",
    "assets/tiles/cercadoCima2.png",
    "assets/tiles/cercadoBaixo1.png",
    "assets/tiles/cercadoBaixo2.png",
    "assets/tiles/grama1.png",
    "assets/tiles/grama2.png",
    "assets/tiles/rua.png"
};

typedef enum {
    TELA_MENU = 0,
    TELA_CONFIG,
    TELA_LEADERBOARD,
    TELA_JOGO,
    TELA_PONTUACAO
} TelaAtual;

typedef struct {
    Texture2D tiles[TOTAL_TILES];
    int tileW;
    int tileH;
    Mapa **mapa;
    Jogador jogador;
    EstadoMenu estadoMenu;
    EstadoJogo estadoJogo;
    EstadoPontuacao estadoPontuacao;
    Armadura *armaduraAtual;
    Capacete *capaceteAtual;
    ArmaPrincipal *armaPrincipalAtual;
    ArmaSecundaria *armaSecundariaAtual;
    Camera2D camera;
    Font fonteNormal;
    Font fonteBold;
    Vector2 posInicial;
    float vidaBaseJogador;
    TelaAtual telaAtual;
    bool solicitarEncerramento;
} AppContext;

static bool CarregarTilesEGerarMapa(AppContext *ctx)
{
    ctx->tiles[ID_TILE_GRAMA_BASE] = LoadTexture(CAMINHOS_TILES[ID_TILE_GRAMA_BASE]);
    if (ctx->tiles[ID_TILE_GRAMA_BASE].id == 0) {
        printf("Erro: Nao foi possivel carregar tile base\n");
        return false;
    }
    ctx->tileW = ctx->tiles[ID_TILE_GRAMA_BASE].width;
    ctx->tileH = ctx->tiles[ID_TILE_GRAMA_BASE].height;

    ctx->mapa = criar_mapa_encadeado(MAP_L, MAP_C, ctx->tileW, ctx->tileH);
    if (!ctx->mapa) {
        UnloadTexture(ctx->tiles[ID_TILE_GRAMA_BASE]);
        ctx->tiles[ID_TILE_GRAMA_BASE] = (Texture2D){0};
        return false;
    }

    bool sucesso = true;
    for (int i = 0; i < TOTAL_TILES; ++i) {
        if (i == ID_TILE_GRAMA_BASE) continue;
        ctx->tiles[i] = LoadTexture(CAMINHOS_TILES[i]);
        if (ctx->tiles[i].id == 0) {
            printf("Erro: Nao foi possivel carregar tile %s\n", CAMINHOS_TILES[i]);
            sucesso = false;
        }
    }
    if (!sucesso) {
        for (int i = 0; i < TOTAL_TILES; ++i) {
            if (ctx->tiles[i].id != 0) UnloadTexture(ctx->tiles[i]);
            ctx->tiles[i] = (Texture2D){0};
        }
        destruir_mapa_encadeado(ctx->mapa, MAP_L);
        ctx->mapa = NULL;
    }
    return sucesso;
}

static void DescarregarTilesEMapa(AppContext *ctx)
{
    if (!ctx) return;
    for (int i = 0; i < TOTAL_TILES; ++i) {
        if (ctx->tiles[i].id != 0) {
            UnloadTexture(ctx->tiles[i]);
            ctx->tiles[i] = (Texture2D){0};
        }
    }
    if (ctx->mapa) {
        destruir_mapa_encadeado(ctx->mapa, MAP_L);
        ctx->mapa = NULL;
    }
}

static bool IniciarJogadorPadrao(AppContext *ctx)
{
    if (!ctx) return false;
    if (!IniciarJogador(&ctx->jogador,
                        ctx->posInicial,
                        400.0f,
                        10.0f,
                        100.0f,
                        "assets/personagem/personagemParado.png",
                        "assets/personagem/personagemAndando1.png",
                        "assets/personagem/personagemAndando2.png")) {
        return false;
    }
    ctx->vidaBaseJogador = ctx->jogador.vidaMaxima;
    return true;
}

static void AtualizarEscalaUI(int largura, int altura)
{
    float escalaH = (float)largura / LARGURA_BASE_UI;
    float escalaV = (float)altura / ALTURA_BASE_UI;
    float escalaUIAtual = fminf(escalaH, escalaV);
    if (escalaUIAtual < 0.7f) escalaUIAtual = 0.7f;
    if (escalaUIAtual > 1.8f) escalaUIAtual = 1.8f;
    UI_SetEscala(escalaUIAtual);
}

static bool AppInicializar(AppContext *ctx)
{
    if (!ctx) return false;
    memset(ctx, 0, sizeof(*ctx));

    const int larguraInicial = 1280;
    const int alturaInicial = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(larguraInicial, alturaInicial, "Tarefa: Identificar No do Jogador");
    SetWindowMinSize(960, 540);
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    ctx->fonteNormal = LoadFont("assets/fontes/PixelOperator.ttf");
    ctx->fonteBold = LoadFont("assets/fontes/PixelOperator-Bold.ttf");

    if (!CarregarTilesEGerarMapa(ctx)) return false;

    ctx->posInicial = (Vector2){
        (MAP_C * ctx->tileW) / 2.0f,
        (MAP_L * ctx->tileH) / 2.0f
    };

    if (!IniciarJogadorPadrao(ctx)) return false;
    CarregarTexturasEquipamentos();

    MenuInicializarEstado(&ctx->estadoMenu);
    JogoInicializar(&ctx->estadoJogo, ctx->jogador.regeneracaoBase);
    PontuacaoInicializar(&ctx->estadoPontuacao);

    ctx->armaduraAtual = NULL;
    ctx->capaceteAtual = NULL;
    ctx->armaPrincipalAtual = NULL;
    ctx->armaSecundariaAtual = NULL;
    ctx->telaAtual = TELA_MENU;
    ctx->solicitarEncerramento = false;

    AtualizarNoAtualJogador(&ctx->jogador, ctx->mapa, MAP_L, MAP_C, ctx->tileW, ctx->tileH);

    ctx->camera.target = ctx->jogador.posicao;
    ctx->camera.offset = (Vector2){ larguraInicial / 2.0f, alturaInicial / 2.0f };
    ctx->camera.rotation = 0.0f;
    ctx->camera.zoom = 1.0f;

    return true;
}

static void AppFinalizar(AppContext *ctx)
{
    if (!ctx) return;
    JogoLiberarRecursos(&ctx->estadoJogo);
    DescarregarTexturasEquipamentos();
    DescarregarJogador(&ctx->jogador);
    DescarregarTilesEMapa(ctx);
    if (ctx->fonteNormal.baseSize > 0) UnloadFont(ctx->fonteNormal);
    if (ctx->fonteBold.baseSize > 0) UnloadFont(ctx->fonteBold);
    if (IsWindowReady()) CloseWindow();
}

static void ProcessarTelaMenu(AppContext *ctx, Vector2 mousePos, bool mouseClique,
                              int largura, int altura)
{
    AcaoMenu acao = MenuDesenharTelaInicial(&ctx->estadoMenu,
                                            mousePos,
                                            mouseClique,
                                            ctx->fonteBold,
                                            largura,
                                            altura);
    if (acao == MENU_ACAO_JOGAR) {
        ctx->telaAtual = TELA_CONFIG;
    } else if (acao == MENU_ACAO_SAIR) {
        ctx->solicitarEncerramento = true;
    } else if (acao == MENU_ACAO_LEADERBOARD) {
        PontuacaoRecarregarArquivo(&ctx->estadoPontuacao);
        ctx->telaAtual = TELA_LEADERBOARD;
    }
}

static void ProcessarTelaConfig(AppContext *ctx, Vector2 mousePos, bool mouseClique,
                                int largura, int altura)
{
    ResultadoMenu resultado = MenuDesenharTelaConfig(&ctx->estadoMenu,
                                                     mousePos,
                                                     mouseClique,
                                                     ctx->fonteNormal,
                                                     ctx->fonteBold,
                                                     &ctx->jogador,
                                                     largura,
                                                     altura);
    if (resultado.acao == MENU_ACAO_VOLTAR) {
        ctx->telaAtual = TELA_MENU;
    } else if (resultado.acao == MENU_ACAO_JOGAR) {
        ctx->capaceteAtual = resultado.capacete;
        ctx->armaduraAtual = resultado.armadura;
        ctx->armaPrincipalAtual = resultado.armaPrincipal;
        ctx->armaSecundariaAtual = resultado.armaSecundaria;
        if (ctx->armaPrincipalAtual) ctx->armaPrincipalAtual->tempoRecargaRestante = 0.0f;
        ctx->estadoJogo.regeneracaoAtual =
            AtualizarVidaJogadorComEquipamentos(&ctx->jogador,
                                                ctx->armaduraAtual,
                                                ctx->capaceteAtual,
                                                ctx->vidaBaseJogador,
                                                ctx->armaPrincipalAtual);
        ctx->jogador.vida = ctx->jogador.vidaMaxima;
        JogoReiniciar(&ctx->estadoJogo, &ctx->jogador, &ctx->camera, ctx->posInicial);
        ctx->telaAtual = TELA_JOGO;
    }
}

static void ProcessarTelaLeaderboard(AppContext *ctx, Vector2 mousePos, bool mouseClique,
                                     int largura, int altura)
{
    ResultadoLeaderboard res = PontuacaoDesenharLeaderboard(&ctx->estadoPontuacao,
                                                            ctx->fonteBold,
                                                            mousePos,
                                                            mouseClique,
                                                            largura,
                                                            altura);
    if (res.voltarMenu) {
        ctx->telaAtual = TELA_MENU;
    }
}

static void ProcessarTelaJogo(AppContext *ctx,
                              float dt,
                              Vector2 mousePos,
                              Vector2 mouseNoMundo,
                              bool mouseCliqueEsq,
                              bool mouseCliqueDir,
                              bool escapePress,
                              int largura,
                              int altura)
{
    JogoAtualizar(&ctx->estadoJogo,
                  &ctx->jogador,
                  &ctx->camera,
                  ctx->mapa,
                  MAP_L,
                  MAP_C,
                  ctx->tileW,
                  ctx->tileH,
                  dt,
                  mouseNoMundo,
                  mouseCliqueEsq,
                  mouseCliqueDir,
                  escapePress,
                  ctx->armaduraAtual,
                  ctx->capaceteAtual,
                  ctx->armaPrincipalAtual,
                  ctx->armaSecundariaAtual);
    JogoDesenhar(&ctx->estadoJogo,
                 &ctx->jogador,
                 &ctx->camera,
                 ctx->mapa,
                 ctx->tiles,
                 TOTAL_TILES,
                 ID_TILE_RUA,
                 MAP_L,
                 MAP_C,
                 ctx->tileW,
                 ctx->tileH,
                 largura,
                 altura,
                 ctx->fonteNormal,
                 ctx->fonteBold,
                 ctx->armaduraAtual,
                 ctx->capaceteAtual,
                 ctx->armaPrincipalAtual,
                 ctx->armaSecundariaAtual,
                 mousePos,
                 mouseCliqueEsq);

    if (ctx->estadoJogo.solicitouRetornoMenu) {
        ctx->estadoJogo.solicitouRetornoMenu = false;
        ctx->telaAtual = TELA_MENU;
    } else if (ctx->estadoJogo.jogadorMorto) {
        ctx->estadoJogo.jogadorMorto = false;
        PontuacaoPrepararCadastro(&ctx->estadoPontuacao, ctx->estadoJogo.pontuacaoTotal);
        ctx->telaAtual = TELA_PONTUACAO;
    }
}

static void ProcessarTelaPontuacao(AppContext *ctx,
                                   Vector2 mousePos,
                                   bool mouseClique,
                                   int largura,
                                   int altura)
{
    ResultadoTelaPontuacao res = PontuacaoDesenharTelaCadastro(&ctx->estadoPontuacao,
                                                               ctx->fonteBold,
                                                               mousePos,
                                                               mouseClique,
                                                               largura,
                                                               altura);
    if (res.pontuacaoSalva) {
        JogoReiniciar(&ctx->estadoJogo, &ctx->jogador, &ctx->camera, ctx->posInicial);
        ctx->jogador.vida = ctx->jogador.vidaMaxima;
        AtualizarNoAtualJogador(&ctx->jogador, ctx->mapa, MAP_L, MAP_C, ctx->tileW, ctx->tileH);
        ctx->telaAtual = TELA_MENU;
    }
}

static void AppExecutarLoop(AppContext *ctx)
{
    while (!ctx->solicitarEncerramento) {
        if (WindowShouldClose()) break;

        float dt = GetFrameTime();
        int largura = GetScreenWidth();
        int altura = GetScreenHeight();
        AtualizarEscalaUI(largura, altura);

        ctx->camera.offset = (Vector2){ largura / 2.0f, altura / 2.0f };
        Vector2 mousePos = GetMousePosition();
        Vector2 mouseNoMundo = GetScreenToWorld2D(mousePos, ctx->camera);
        bool mouseCliqueEsq = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        bool mouseCliqueDir = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
        bool escapePress = IsKeyPressed(KEY_ESCAPE);

        BeginDrawing();
        ClearBackground((Color){12, 12, 26, 255});
        switch (ctx->telaAtual) {
            case TELA_MENU:
                ProcessarTelaMenu(ctx, mousePos, mouseCliqueEsq, largura, altura);
                break;
            case TELA_CONFIG:
                ProcessarTelaConfig(ctx, mousePos, mouseCliqueEsq, largura, altura);
                break;
            case TELA_LEADERBOARD:
                ProcessarTelaLeaderboard(ctx, mousePos, mouseCliqueEsq, largura, altura);
                break;
            case TELA_JOGO:
                ProcessarTelaJogo(ctx, dt, mousePos, mouseNoMundo,
                                  mouseCliqueEsq, mouseCliqueDir, escapePress,
                                  largura, altura);
                break;
            case TELA_PONTUACAO:
                ProcessarTelaPontuacao(ctx, mousePos, mouseCliqueEsq, largura, altura);
                break;
        }
        EndDrawing();

        if (ctx->solicitarEncerramento) break;
    }
}

int AplicacaoExecutar(void)
{
    AppContext ctx;
    if (!AppInicializar(&ctx)) {
        AppFinalizar(&ctx);
        return 1;
    }
    AppExecutarLoop(&ctx);
    AppFinalizar(&ctx);
    return 0;
}
