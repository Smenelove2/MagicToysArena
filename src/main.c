#include "raylib.h"
#include "jogador.h"
#include "mapa.h"
#include <stdio.h>
#include <stdbool.h>

// ---------------------- Config do mapa ----------------------
#define MAP_L 65
#define MAP_C 65

static Texture2D gTiles[4];
static int TILE_W = 64;
static int TILE_H = 64;
static Mapa **gMapa = NULL;

// Converte de posição no mundo (pixels) para índice da grade
static inline bool WorldToGrid(Vector2 pos, int *outI, int *outJ) {
    if (pos.x < 0 || pos.y < 0) return false;
    int j = (int)(pos.x / TILE_W);
    int i = (int)(pos.y / TILE_H);
    if (i < 0 || j < 0 || i >= MAP_L || j >= MAP_C) return false;
    if (outI) *outI = i;
    if (outJ) *outJ = j;
    return true;
}

static inline bool TileColideIJ(int i, int j) {
    if (i < 0 || j < 0 || i >= MAP_L || j >= MAP_C) return true; // fora do mapa = bloqueia
    return gMapa[i][j].colisao;
}

// Colisão simples: se o centro do jogador cair em um tile de cerca, desfaz o movimento
static void AplicarColisaoPosicaoJogador(Jogador *j, Vector2 posAnterior) {
    int i, jx;
    if (!WorldToGrid(j->posicao, &i, &jx)) {
        // fora do mapa: volta
        j->posicao = posAnterior;
        return;
    }
    if (TileColideIJ(i, jx)) {
        j->posicao = posAnterior; // volta
    }
}

// Desenha apenas o que está visível no retângulo da câmera
static void DesenharMapaVisivel(Camera2D *cam, int telaW, int telaH) {
    // calcula AABB visível em coordenadas de mundo
    // offset centraliza a câmera na tela, então o canto superior esquerdo do mundo é:
    Vector2 topoEsq = GetScreenToWorld2D((Vector2){0,0}, *cam);
    Vector2 botDir  = GetScreenToWorld2D((Vector2){(float)telaW,(float)telaH}, *cam);

    int jIni = (int)(topoEsq.x / TILE_W) - 1;
    int iIni = (int)(topoEsq.y / TILE_H) - 1;
    int jFim = (int)(botDir.x  / TILE_W) + 1;
    int iFim = (int)(botDir.y  / TILE_H) + 1;

    if (jIni < 0) jIni = 0;
    if (iIni < 0) iIni = 0;
    if (jFim > MAP_C-1) jFim = MAP_C-1;
    if (iFim > MAP_L-1) iFim = MAP_L-1;

    for (int i = iIni; i <= iFim; ++i) {
        for (int j = jIni; j <= jFim; ++j) {
            int id = gMapa[i][j].id_tile; // 0..3
            Vector2 pos = { j * (float)TILE_W, i * (float)TILE_H };
            DrawTextureV(gTiles[id], pos, WHITE);
        }
    }
}

// ------------------------------------------------------------

int main(void) {
    const int largura = 1920;
    const int altura  = 1080;

    InitWindow(largura, altura, "Teste de Asset do Jogador com Câmera + Mapa");
    SetTargetFPS(60);

    // ----- Mapa: criar e carregar tiles -----
    gMapa = criar_mapa_encadeado(MAP_L, MAP_C);
    if (!gMapa) { CloseWindow(); return 1; }

    gTiles[0] = LoadTexture("assets/tiles/cercado1.png");
    gTiles[1] = LoadTexture("assets/tiles/cercado2.png");
    gTiles[2] = LoadTexture("assets/tiles/grama1.png");
    gTiles[3] = LoadTexture("assets/tiles/grama2.png");

    // Valida carregamento e descobre tamanho do tile
    if (gTiles[0].id == 0 || gTiles[1].id == 0 || gTiles[2].id == 0 || gTiles[3].id == 0) {
        UnloadTexture(gTiles[0]); UnloadTexture(gTiles[1]);
        UnloadTexture(gTiles[2]); UnloadTexture(gTiles[3]);
        destruir_mapa_encadeado(gMapa, MAP_L);
        CloseWindow();
        return 1;
    }
    TILE_W = gTiles[0].width;
    TILE_H = gTiles[0].height;

    // ----- Inicializa o jogador -----
    // Posiciona no centro do mapa em coordenadas de mundo
    Vector2 posInicial = {
        (MAP_C * TILE_W) / 2.0f,
        (MAP_L * TILE_H) / 2.0f
    };

    Jogador jogador;
    bool ok = IniciarJogador(
        &jogador,
        posInicial,
        400.0f,   // velocidade
        10.0f,    // fps da caminhada
        100.0f,   // vida
        "assets/personagem/personagemParado.png",
        "assets/personagem/personagemAndando1.png",
        "assets/personagem/personagemAndando2.png"
    );
    if (!ok) {
        for (int k=0;k<4;++k) UnloadTexture(gTiles[k]);
        destruir_mapa_encadeado(gMapa, MAP_L);
        CloseWindow();
        return 1;
    }

    // ----- Configura a câmera -----
    Camera2D camera = {0};
    camera.target = jogador.posicao;
    camera.offset = (Vector2){ largura / 2.0f, altura / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    Vector2 tam = TamanhoJogador(&jogador);
    printf("Sprite atual do jogador: %.0fx%.0f\n", tam.x, tam.y);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // Guarda posição anterior para corrigir se colidir
        Vector2 posAnterior = jogador.posicao;

        // Atualiza jogador (movimento, animação etc.)
        AtualizarJogador(&jogador, dt);

        // Colisão com tiles (checa tile sob o centro do jogador)
        AplicarColisaoPosicaoJogador(&jogador, posAnterior);

        // Mantém o jogador dentro do mapa (clamp mundo)
        float maxX = MAP_C * (float)TILE_W - 1.0f;
        float maxY = MAP_L * (float)TILE_H - 1.0f;
        if (jogador.posicao.x < 0) jogador.posicao.x = 0;
        if (jogador.posicao.y < 0) jogador.posicao.y = 0;
        if (jogador.posicao.x > maxX) jogador.posicao.x = maxX;
        if (jogador.posicao.y > maxY) jogador.posicao.y = maxY;

        // Atualiza a câmera
        camera.target = jogador.posicao;

        // ---------- Desenho ----------
        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);
            // Mapa (apenas visível)
            DesenharMapaVisivel(&camera, largura, altura);

            // Jogador
            DesenharJogador(&jogador);

            // (Opcional) grade debug por cima:
            // for (int j=0;j<MAP_C;++j) for (int i=0;i<MAP_L;++i)
            //     DrawRectangleLines(j*TILE_W, i*TILE_H, TILE_W, TILE_H, Fade(WHITE, 0.05f));
        EndMode2D();

        // HUD (fixo na tela)
        DrawText("ESC para sair", 20, 20, 20, WHITE);
        DrawText(TextFormat("Posicao: (%.0f, %.0f)", jogador.posicao.x, jogador.posicao.y), 20, 50, 20, LIGHTGRAY);

        EndDrawing();
    }

    // ----- Cleanup -----
    DescarregarJogador(&jogador);
    for (int k=0;k<4;++k) UnloadTexture(gTiles[k]);
    destruir_mapa_encadeado(gMapa, MAP_L);
    CloseWindow();
    return 0;
}
