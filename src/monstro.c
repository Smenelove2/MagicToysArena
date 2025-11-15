#include "monstro.h"
#include "objeto.h"
#include "jogador.h"
#include "mapa.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

void CarregarAssetsMonstro(Monstro *m)
{
    if (!m)
        return;

    // Carrega sprites baseado no tipo de monstro
    switch (m->tipo)
    {
    case MONSTRO_ESQUELETO:
        m->sprite1 = LoadTexture("assets/esqueleto/esqueleto1.png.png");
        m->sprite2 = LoadTexture("assets/esqueleto/esqueleto2.png.png");
        m->sprite3 = LoadTexture("assets/esqueleto/esqueleto3.png.png");
        break;
    case MONSTRO_ZOMBIE:
        m->sprite1 = LoadTexture("assets/zombie/zombie1.png.png");
        m->sprite2 = LoadTexture("assets/zombie/zombie2.png.png");
        m->sprite3 = LoadTexture("assets/zombie/zombie3.png.png");
        break;
    case MONSTRO_URSO:
        m->sprite1 = LoadTexture("assets/urso/urso1.png.png");
        m->sprite2 = LoadTexture("assets/urso/urso2.png.png");
        m->sprite3 = LoadTexture("assets/urso/urso3.png.png");
        break;
    case MONSTRO_IT:
        m->sprite1 = LoadTexture("assets/IT/IT-1.png.png");
        m->sprite2 = LoadTexture("assets/IT/IT-2.png.png");
        m->sprite3 = LoadTexture("assets/IT/IT-3.png.png");
        break;
    default:
        m->sprite1 = LoadTexture("assets/esqueleto/esqueleto1.png.png");
        m->sprite2 = LoadTexture("assets/esqueleto/esqueleto2.png.png");
        m->sprite3 = LoadTexture("assets/esqueleto/esqueleto3.png.png");
        break;
    }
}

bool IniciarMonstro(Monstro *m,
                    Vector2 posInicial,
                    float vida,
                    float velocidade,
                    float fpsAnimacao,
                    float alcanceAtaque,
                    float cooldownAtaque,
                    TipoMonstro tipo)
{
    if (!m)
        return false;

    m->posicao = posInicial;
    m->vida = vida;
    m->velocidade = velocidade;
    m->tipo = tipo;
    m->ativo = true; // Monstro inicia ativo

    memset(&m->sprite1, 0, sizeof(Texture2D) * 3);

    m->fpsAnimacao = fpsAnimacao;
    m->acumulador = 0.0f;
    m->frameAtual = 1;

    m->alcanceAtaque = alcanceAtaque;
    m->raioDeteccao = 300.0f; // Detecta o jogador a 300 pixels de distância
    m->cooldownAtaque = cooldownAtaque;
    m->acumuladorAtaque = 0.0f;

    m->objeto = (struct ObjetoLancavel *)malloc(sizeof(ObjetoLancavel));
    if (!m->objeto)
        return false;

    if (!IniciarObjeto(m->objeto, "assets/sprites/bone.png"))
    {
        free(m->objeto);
        m->objeto = NULL;
        return false;
    }

    return true;
}

void DescarregarMonstro(Monstro *m)
{
    if (!m)
        return;

    if (m->sprite1.id != 0)
        UnloadTexture(m->sprite1);
    if (m->sprite2.id != 0)
        UnloadTexture(m->sprite2);
    if (m->sprite3.id != 0)
        UnloadTexture(m->sprite3);

    if (m->objeto)
    {
        DescarregarObjeto(m->objeto);
        free(m->objeto);
        m->objeto = NULL;
    }

    memset(m, 0, sizeof(*m));
}

void AtualizarMonstro(Monstro *m, float dt)
{
    if (!m || m->vida <= 0)
        return;

    // Atualiza animação
    float intervalo = 1.0f / m->fpsAnimacao;
    m->acumulador += dt;
    if (m->acumulador >= intervalo)
    {
        m->acumulador -= intervalo;
        m->frameAtual++;
        if (m->frameAtual > 3)
            m->frameAtual = 1;
    }

    // Atualiza cooldown de ataque
    if (m->acumuladorAtaque > 0)
        m->acumuladorAtaque -= dt;
}

void DesenharMonstro(const Monstro *m)
{
    if (!m)
        return;

    Texture2D spriteAtual = m->sprite1;
    switch (m->frameAtual)
    {
    case 1:
        spriteAtual = m->sprite1;
        break;
    case 2:
        spriteAtual = m->sprite2;
        break;
    case 3:
        spriteAtual = m->sprite3;
        break;
    }

    float escala = 2.0f; // Dobra o tamanho do monstro

    DrawTextureEx(
        spriteAtual,
        (Vector2){m->posicao.x - (spriteAtual.width * escala) / 2,
                  m->posicao.y - (spriteAtual.height * escala) / 2},
        0.0f,
        escala,
        WHITE);
}

Vector2 GerarMonstros(struct Jogador *jogador, int mapL, int mapC, int tileW, int tileH)
{
    Vector2 resultado = {0, 0};

    if (jogador == NULL || jogador->noAtual == NULL)
        return resultado;

    int jl = jogador->noAtual->linha;
    int jc = jogador->noAtual->coluna;

    int raio = 3;

    int linha, coluna;

    while (1)
    {
        linha = jl + GetRandomValue(-raio, raio);
        coluna = jc + GetRandomValue(-raio, raio);

        if (linha < 1)
            linha = 1;
        if (linha > mapL - 2)
            linha = mapL - 2;

        if (coluna < 1)
            coluna = 1;
        if (coluna > mapC - 2)
            coluna = mapC - 2;

        if (linha == jl && coluna == jc)
            continue;

        break;
    }

    resultado.x = coluna * tileW + tileW / 2;
    resultado.y = linha * tileH + tileH / 2;

    return resultado;
}

// IA: Move o monstro em direção ao jogador se estiver no raio de detecção
void IAAtualizarMonstro(Monstro *m, struct Jogador *jogador, float dt)
{
    if (!m || !m->ativo || !jogador)
        return;

    // Calcula distância entre monstro e jogador
    float dx = jogador->posicao.x - m->posicao.x;
    float dy = jogador->posicao.y - m->posicao.y;
    float distancia = sqrtf(dx * dx + dy * dy);

    // Se o jogador está dentro do raio de detecção, move em sua direção
    if (distancia < m->raioDeteccao && distancia > 0.01f)
    {
        // Normaliza a direção
        float dirX = dx / distancia;
        float dirY = dy / distancia;

        // Move na direção do jogador
        m->posicao.x += dirX * m->velocidade * dt;
        m->posicao.y += dirY * m->velocidade * dt;
    }
}

// Colisão: Retorna true se o monstro colidiu com o jogador
bool VerificarColisaoMonstroJogador(Monstro *m, struct Jogador *jogador)
{
    if (!m || !m->ativo || !jogador)
        return false;

    // Define raio de colisão (metade da largura do sprite)
    float raioColisao = 20.0f;

    float dx = m->posicao.x - jogador->posicao.x;
    float dy = m->posicao.y - jogador->posicao.y;
    float distancia = sqrtf(dx * dx + dy * dy);

    // Colide se a distância é menor que a soma dos raios
    return distancia < (raioColisao * 2);
}