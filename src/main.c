#include "raylib.h"
#include "jogador.h"
#include "mapa.h"
#include "monstro.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>

#define MAP_L 65
#define MAP_C 65

static Texture2D gTiles[14];
static int TILE_W = 64;
static int TILE_H = 64;
static Mapa **gMapa = NULL;

// Converte de posição no mundo (pixels) para índice da grade
static inline bool WorldToGrid(Vector2 pos, int *outI, int *outJ)
{
    if (pos.x < 0 || pos.y < 0)
        return false;
    int j = (int)(pos.x / TILE_W);
    int i = (int)(pos.y / TILE_H);
    if (i < 0 || j < 0 || i >= MAP_L || j >= MAP_C)
        return false;
    if (outI)
        *outI = i;
    if (outJ)
        *outJ = j;
    return true;
}

static inline bool TileColideIJ(int i, int j)
{
    if (i < 0 || j < 0 || i >= MAP_L || j >= MAP_C)
        return true; // fora do mapa = bloqueia
    return gMapa[i][j].colisao;
}

// Colisão simples: se o centro do jogador cair em um tile de cerca, desfaz o movimento
static void AplicarColisaoPosicaoJogador(Jogador *j, Vector2 posAnterior)
{
    int i, jx;
    if (!WorldToGrid(j->posicao, &i, &jx))
    {
        j->posicao = posAnterior;
        return;
    }
    if (TileColideIJ(i, jx))
    {
        j->posicao = posAnterior;
    }
}

// Desenha apenas o que está visível no retângulo da câmera
static void DesenharMapaVisivel(Camera2D *cam, int telaW, int telaH)
{
    Vector2 topoEsq = GetScreenToWorld2D((Vector2){0, 0}, *cam);
    Vector2 botDir = GetScreenToWorld2D((Vector2){(float)telaW, (float)telaH}, *cam);

    int jIni = (int)(topoEsq.x / TILE_W) - 1;
    int iIni = (int)(topoEsq.y / TILE_H) - 1;
    int jFim = (int)(botDir.x / TILE_W) + 1;
    int iFim = (int)(botDir.y / TILE_H) + 1;

    if (jIni < 0)
        jIni = 0;
    if (iIni < 0)
        iIni = 0;
    if (jFim > MAP_C - 1)
        jFim = MAP_C - 1;
    if (iFim > MAP_L - 1)
        iFim = MAP_L - 1;

    for (int i = iIni; i <= iFim; ++i)
    {
        for (int j = jIni; j <= jFim; ++j)
        {
            int id = gMapa[i][j].id_tile; // 0..13
            // Usa a posição em pixels armazenada no nó
            Vector2 pos = {gMapa[i][j].posX, gMapa[i][j].posY};
            DrawTextureV(gTiles[id], pos, WHITE);
        }
    }
}

// --- NOVA FUNÇÃO ---
// Atualiza o ponteiro 'noAtual' do jogador para o tile em que ele se encontra
static void AtualizarNoAtualJogador(Jogador *j)
{
    int i, jx;
    // Converte a posição de pixels do jogador para índices da grade
    if (WorldToGrid(j->posicao, &i, &jx))
    {
        // Atualiza o ponteiro para apontar para o nó correto na matriz global
        j->noAtual = &gMapa[i][jx];
    }
    else
    {
        // Se estiver fora do mapa, seta como NULL
        j->noAtual = NULL;
    }
}

// ------------------------------------------------------------

int main()
{
    const int largura = 1920;
    const int altura = 1080;

    InitWindow(largura, altura, "Tarefa: Identificar No do Jogador");
    SetTargetFPS(60);

    // ----- Mapa: Etapa 1: Descobrir tamanho do Tile -----
    // Carrega um tile de referência (grama) primeiro
    gTiles[12] = LoadTexture("assets/tiles/grama1.png");
    if (gTiles[12].id == 0)
    {
        printf("Erro: Não foi possível carregar tile de referência 'grama1.png'\n");
        CloseWindow();
        return 1;
    }
    // Define o tamanho global dos tiles baseado na textura carregada
    TILE_W = gTiles[12].width;
    TILE_H = gTiles[12].height;
    printf("Tamanho do tile detectado: %dx%d\n", TILE_W, TILE_H);

    // ----- Mapa: Etapa 2: Criar estrutura de dados -----
    // Agora que TILE_W/H são conhecidos, podemos criar o mapa
    gMapa = criar_mapa_encadeado(MAP_L, MAP_C, TILE_W, TILE_H); // <-- PASSANDO OS PARÂMETROS
    if (!gMapa)
    {
        UnloadTexture(gTiles[12]); // Libera o tile de referência
        CloseWindow();
        return 1;
    }

    // ----- Mapa: Etapa 3: Carregar os tiles restantes -----
    gTiles[0] = LoadTexture("assets/tiles/cercadoPonta1.png"); // topo-esq
    gTiles[1] = LoadTexture("assets/tiles/cercadoPonta2.png"); // topo-dir
    gTiles[2] = LoadTexture("assets/tiles/cercadoPonta3.png"); // baixo-esq
    gTiles[3] = LoadTexture("assets/tiles/cercadoPonta4.png"); // baixo-dir
    gTiles[4] = LoadTexture("assets/tiles/cercadoEsquerda1.png");
    gTiles[5] = LoadTexture("assets/tiles/cercadoEsquerda2.png");
    gTiles[6] = LoadTexture("assets/tiles/cercadoDireita1.png");
    gTiles[7] = LoadTexture("assets/tiles/cercadoDireita2.png");
    gTiles[8] = LoadTexture("assets/tiles/cercadoCima1.png");
    gTiles[9] = LoadTexture("assets/tiles/cercadoCima2.png");
    gTiles[10] = LoadTexture("assets/tiles/cercadoBaixo1.png");
    gTiles[11] = LoadTexture("assets/tiles/cercadoBaixo2.png");
    // gTiles[12] já foi carregado
    gTiles[13] = LoadTexture("assets/tiles/grama2.png");

    // Valida carregamento (agora checa todos)
    bool okTiles = true;
    for (int t = 0; t < 14; ++t)
    {
        if (gTiles[t].id == 0)
        {
            printf("Erro: Falha ao carregar tile ID %d\n", t);
            okTiles = false;
        }
    }
    if (!okTiles)
    {
        for (int t = 0; t < 14; ++t)
            if (gTiles[t].id != 0)
                UnloadTexture(gTiles[t]);
        destruir_mapa_encadeado(gMapa, MAP_L);
        CloseWindow();
        return 1;
    }
    // TILE_W e TILE_H já foram definidos, não precisa mais

    // ----- Inicializa o jogador -----
    Vector2 posInicial = {
        (MAP_C * TILE_W) / 2.0f,
        (MAP_L * TILE_H) / 2.0f};

    Jogador jogador;
    bool ok = IniciarJogador(
        &jogador,
        posInicial,
        400.0f, // velocidade
        10.0f,  // fps da caminhada
        100.0f, // vida
        "assets/personagem/personagemParado.png",
        "assets/personagem/personagemAndando1.png",
        "assets/personagem/personagemAndando2.png");
    if (!ok)
    {
        for (int t = 0; t < 14; ++t)
            UnloadTexture(gTiles[t]);
        destruir_mapa_encadeado(gMapa, MAP_L);
        CloseWindow();
        return 1;
    }

    //  Definir o nó inicial do jogador
    // (IniciarJogador setou como NULL, agora atualizamos para o nó inicial)
    AtualizarNoAtualJogador(&jogador);
    if (jogador.noAtual)
    {
        printf("Jogador iniciou no nó (%d, %d)\n", jogador.noAtual->linha, jogador.noAtual->coluna);
    }

    // Geração progressiva de monstros (um por vez)
    Monstro monstros[MAX_MONSTROS];
    int numMonstrosAtivos = 0;  // Quantidade de monstros ativos no momento
    int numMonstrosMaximos = 5; // Total de monstros a gerar
    float timerGeracaoMonstro = 0.0f;
    float intervaloGeracaoMonstro = 3.0f; // Gera um monstro a cada 3 segundos

    // Inicializa array de monstros como inativos
    for (int i = 0; i < MAX_MONSTROS; i++)
    {
        memset(&monstros[i], 0, sizeof(Monstro));
        monstros[i].ativo = false;
    }

    Camera2D camera = {0};
    camera.target = jogador.posicao;
    camera.offset = (Vector2){largura / 2.0f, altura / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    Vector2 tam = TamanhoJogador(&jogador);
    printf("Sprite atual do jogador: %.0fx%.0f\n", tam.x, tam.y);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // Guarda posição anterior para corrigir se colidir
        Vector2 posAnterior = jogador.posicao;

        // Atualiza jogador
        AtualizarJogador(&jogador, dt);

        // --- FIM DA LOGICA ADICIONADA ---

        // --- GERACAO PROGRESSIVA DE MONSTROS ---
        timerGeracaoMonstro += dt;
        if (timerGeracaoMonstro >= intervaloGeracaoMonstro && numMonstrosAtivos < numMonstrosMaximos)
        {
            timerGeracaoMonstro = 0.0f;

            // Encontra primeiro monstro inativo
            for (int i = 0; i < MAX_MONSTROS; i++)
            {
                if (!monstros[i].ativo)
                {
                    Vector2 pontoGerado = GerarMonstros(&jogador, MAP_L, MAP_C, TILE_W, TILE_H);
                    TipoMonstro tipoAleatorio = (TipoMonstro)GetRandomValue(0, MONSTRO_TIPOS_COUNT - 1);

                    if (IniciarMonstro(&monstros[i], pontoGerado,
                                       100, // vida
                                       60,  // velocidade
                                       8,   // fps animação
                                       150, // alcance ataque
                                       1,   // cooldown
                                       tipoAleatorio))
                    {
                        CarregarAssetsMonstro(&monstros[i]);
                        monstros[i].ativo = true;
                        numMonstrosAtivos++;

                        const char *nomeTipo[] = {"Esqueleto", "Zumbi", "Urso", "IT"};
                        printf("Monstro gerado: %s na posição (%.0f, %.0f) [Total: %d]\n",
                               nomeTipo[tipoAleatorio], pontoGerado.x, pontoGerado.y, numMonstrosAtivos);
                    }
                    break;
                }
            }
        }
        // --- FIM GERACAO PROGRESSIVA ---

        // Atualiza todos os monstros (animação e cooldown)
        for (int i = 0; i < MAX_MONSTROS; i++)
        {
            if (monstros[i].ativo)
            {
                AtualizarMonstro(&monstros[i], dt);
                // IA: Monstro persegue o jogador
                IAAtualizarMonstro(&monstros[i], &jogador, dt);

                // Colisão com jogador
                if (VerificarColisaoMonstroJogador(&monstros[i], &jogador))
                {
                    jogador.vida -= 10.0f;   // Jogador perde 10 de vida
                    monstros[i].vida = 0.0f; // Monstro morre
                    monstros[i].ativo = false;
                    numMonstrosAtivos--;
                    printf("COLISAO! Jogador atingido. Vida: %.0f | Monstros: %d\n",
                           jogador.vida, numMonstrosAtivos);
                }
            }
        }

        // Colisão com tiles (checa tile sob o centro do jogador)
        // AplicarColisaoPosicaoJogador(&jogador, posAnterior); // <- Você pode descomentar isso

        // Colisão alternativa (usa o ponteiro noAtual que acabamos de atualizar)
        if (jogador.noAtual && jogador.noAtual->colisao)
        {
            jogador.posicao = posAnterior;
            // Re-atualiza o noAtual para o nó anterior (válido)
            AtualizarNoAtualJogador(&jogador);
        }

        // Mantém o jogador dentro do mapa (clamp mundo)
        float maxX = MAP_C * (float)TILE_W - 1.0f;
        float maxY = MAP_L * (float)TILE_H - 1.0f;
        if (jogador.posicao.x < 0)
            jogador.posicao.x = 0;
        if (jogador.posicao.y < 0)
            jogador.posicao.y = 0;
        if (jogador.posicao.x > maxX)
            jogador.posicao.x = maxX;
        if (jogador.posicao.y > maxY)
            jogador.posicao.y = maxY;

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

        // Todos os monstros ativos
        for (int i = 0; i < MAX_MONSTROS; i++)
        {
            if (monstros[i].ativo)
            {
                DesenharMonstro(&monstros[i]);
            }
        }
        EndMode2D();

        // HUD (fixo na tela)
        DrawText("ESC para sair", 20, 20, 20, WHITE);
        DrawText(TextFormat("Vida: %.0f", jogador.vida), 20, 50, 20, RED);
        DrawText(TextFormat("Monstros Ativos: %d/%d", numMonstrosAtivos, numMonstrosMaximos), 20, 80, 20, YELLOW);
        DrawText(TextFormat("Posicao: (%.0f, %.0f)", jogador.posicao.x, jogador.posicao.y), 20, 110, 20, LIGHTGRAY);

        // --- HUD ADICIONADO PARA DEBUG ---
        if (jogador.noAtual)
        {
            DrawText(TextFormat("No Atual: (%d, %d)", jogador.noAtual->linha, jogador.noAtual->coluna), 20, 140, 20, LIME);
            DrawText(TextFormat("  TileID: %d", jogador.noAtual->id_tile), 20, 170, 20, LIME);
            DrawText(TextFormat("  Colisao: %s", jogador.noAtual->colisao ? "Sim" : "Nao"), 20, 200, 20, LIME);
            DrawText(TextFormat("  Pos (px): (%.0f, %.0f)", jogador.noAtual->posX, jogador.noAtual->posY), 20, 230, 20, LIME);
            Vector4 distBordas = DistanciaBordasJogador(&jogador);

            DrawText(TextFormat("Dist. Bordas (Nós): CIMA:%.0f, BAIXO:%.0f, ESQ:%.0f, DIR:%.0f", distBordas.y, distBordas.w, distBordas.x, distBordas.z), 20, 200, 20, YELLOW);

            float maxDist = fmaxf(fmaxf(distBordas.x, distBordas.y), fmaxf(distBordas.z, distBordas.w));

            if (maxDist == distBordas.y)
            {
                DrawText("Melhor Spawm: CIMA", 20, 230, 20, ORANGE);
            }
            else if (maxDist == distBordas.w)
            {
                DrawText("Melhor Spawm: BAIXO", 20, 230, 20, ORANGE);
            }
            else if (maxDist == distBordas.x)
            {
                DrawText("Melhor Spawm: ESQUERDA", 20, 230, 20, ORANGE);
            }
            else if (maxDist == distBordas.z)
            {
                DrawText("Melhor Spawm: DIREITA", 20, 230, 20, ORANGE);
            }
        }
        else
        {
            DrawText("No Atual: (Fora do Mapa)", 20, 80, 20, RED);
        }

        EndDrawing();
    }

    // ----- Cleanup -----
    for (int i = 0; i < MAX_MONSTROS; i++)
    {
        DescarregarMonstro(&monstros[i]);
    }
    DescarregarJogador(&jogador);
    for (int t = 0; t < 14; ++t)
        UnloadTexture(gTiles[t]);
    destruir_mapa_encadeado(gMapa, MAP_L);
    CloseWindow();
    return 0;
}
