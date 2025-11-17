#include "jogador.h"
#include "mapa.h"
#include <math.h>
#include <string.h>

static inline float ComprimentoVetor(float x, float y)
{
    return sqrtf(x * x + y * y);
}

bool IniciarJogador(Jogador* j,
                    Vector2 posInicial,
                    float velocidade,
                    float fpsAndar,
                    float vida,
                    const char* caminhoParado,
                    const char* caminhoAndando1,
                    const char* caminhoAndando2)
{
    if (!j) return false;

    j->posicao    = posInicial;
    j->velocidade = velocidade;
    j->velocidadeBase = velocidade;
    j->vida       = vida;
    j->vidaMaxima = vida;
    j->regeneracaoBase = 2.0f;

    j->parado   = LoadTexture(caminhoParado);
    j->andando1 = LoadTexture(caminhoAndando1);
    j->andando2 = LoadTexture(caminhoAndando2);

    if (j->parado.id == 0 || j->andando1.id == 0 || j->andando2.id == 0) {
        if (j->parado.id)   UnloadTexture(j->parado);
        if (j->andando1.id) UnloadTexture(j->andando1);
        if (j->andando2.id) UnloadTexture(j->andando2);
        memset(j, 0, sizeof(*j));
        return false;
    }

    j->fpsAndar      = (fpsAndar <= 0.0f) ? 10.0f : fpsAndar;
    j->acumulador    = 0.0f;
    j->alternarFrame = true;
    j->emMovimento   = false;
    j->noAtual = NULL;

    return true;
}

void AtualizarJogador(Jogador* j, float dt)
{
    if (!j) return;

    Vector2 direcao = (Vector2){0.0f, 0.0f};
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) direcao.x += 1.0f;
    if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) direcao.x -= 1.0f;
    if (IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W)) direcao.y -= 1.0f;
    if (IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S)) direcao.y += 1.0f;

    float magnitude = ComprimentoVetor(direcao.x, direcao.y);
    j->emMovimento = magnitude > 0.0001f;

    if (j->emMovimento) {
        direcao.x /= magnitude;
        direcao.y /= magnitude;

        j->posicao.x += direcao.x * j->velocidade * dt;
        j->posicao.y += direcao.y * j->velocidade * dt;

        float intervalo = 1.0f / j->fpsAndar;
        j->acumulador += dt;
        if (j->acumulador >= intervalo) {
            j->acumulador -= intervalo;
            j->alternarFrame = !j->alternarFrame;
        }
    } else {
        j->acumulador = 0.0f;
        j->alternarFrame = true;
    }
}

void DesenharJogador(const Jogador* j)
{
    if (!j) return;

    Texture2D spriteAtual = j->parado;
    if (j->emMovimento)
        spriteAtual = j->alternarFrame ? j->andando1 : j->andando2;

    float escala = 1.0f;

    DrawTextureEx(
        spriteAtual,
        (Vector2){ j->posicao.x - (spriteAtual.width * escala) / 2,
                   j->posicao.y - (spriteAtual.height * escala) / 2 },
        0.0f,
        escala,
        WHITE
    );
}

void DescarregarJogador(Jogador* j)
{
    if (!j) return;
    if (j->parado.id)   UnloadTexture(j->parado);
    if (j->andando1.id) UnloadTexture(j->andando1);
    if (j->andando2.id) UnloadTexture(j->andando2);
    memset(j, 0, sizeof(*j));
}

Vector2 TamanhoJogador(const Jogador* j)
{
    if (!j) return (Vector2){0, 0};

    Texture2D atual = j->parado;
    if (j->emMovimento)
        atual = j->alternarFrame ? j->andando1 : j->andando2;

    return (Vector2){ atual.width, atual.height };
}

void AplicarColisaoPosicaoJogador(Jogador* j, Vector2 posAnterior,
                                  Mapa **mapa, int linhas, int colunas,
                                  int tileLargura, int tileAltura)
{
    if (!j || !mapa) return;

    int i, jx;
    bool dentro = ConverterPosicaoParaIndice(j->posicao.x, j->posicao.y,
                                             tileLargura, tileAltura,
                                             linhas, colunas,
                                             &i, &jx);
    if (!dentro) {
        j->posicao = posAnterior;
        j->noAtual = NULL;
        return;
    }

    if (TilePossuiColisao(mapa, linhas, colunas, i, jx)) {
        j->posicao = posAnterior;
        j->noAtual = &mapa[i][jx];
    }
}

void AtualizarNoAtualJogador(Jogador* j, Mapa **mapa, int linhas, int colunas,
                             int tileLargura, int tileAltura)
{
    if (!j || !mapa) return;

    int i, jx;
    if (ConverterPosicaoParaIndice(j->posicao.x, j->posicao.y,
                                   tileLargura, tileAltura,
                                   linhas, colunas,
                                   &i, &jx)) {
        j->noAtual = &mapa[i][jx];
    } else {
        j->noAtual = NULL;
    }
}
