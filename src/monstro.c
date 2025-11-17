#include "monstro.h"
#include "monstro_dados.h"
#include "objeto.h"
#include "jogador.h"
#include "mapa.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

void CarregarAssetsMonstro(Monstro *m)
{
    if (!m) return;
    const MonstroInfo *info = ObterInfoMonstro(m->tipo);
    if (!info) return;

    for (int i = 0; i < 3; ++i) {
        if (!info->sprites[i]) continue;
        Texture2D sprite = LoadTexture(info->sprites[i]);
        if (sprite.id == 0) continue;
        switch (i) {
            case 0: m->sprite1 = sprite; break;
            case 1: m->sprite2 = sprite; break;
            case 2: m->sprite3 = sprite; break;
        }
    }
}

bool IniciarMonstro(Monstro *m,
                    Vector2 posInicial,
                    const MonstroInfo *info)
{
    if (!m || !info)
        return false;

    m->posicao = posInicial;
    m->vidaMaxima = info->vida;
    m->vida = info->vida;
    m->velocidade = info->velocidade;
    m->tipo = info->tipo;
    m->ativo = true;

    memset(&m->sprite1, 0, sizeof(Texture2D) * 3);

    m->fpsAnimacao = info->fpsAnimacao;
    m->acumulador = 0.0f;
    m->frameAtual = 1;

    m->alcanceAtaque = info->alcanceAtaque;
    m->cooldownAtaque = info->cooldownAtaque;
    m->acumuladorAtaque = 0.0f;
    m->danoContato = info->danoContato;
    m->pontuacao = info->pontuacao;

    m->cooldownArremesso = info->cooldownArremesso;
    m->acumuladorArremesso = 0.0f;
    
    m->objeto = NULL;
    if (info->possuiObjeto && info->spriteObjeto) {
        m->objeto = (struct ObjetoLancavel *)malloc(sizeof(ObjetoLancavel));
        if (!m->objeto){
            return false;
        }

        memset(m->objeto, 0, sizeof(ObjetoLancavel));
        m->objeto->dano = info->danoObjeto;
        m->objeto->velocidade = info->velocidadeObjeto;

        if(!IniciarObjeto(m->objeto, info->spriteObjeto)){
            free(m->objeto);
            m->objeto = NULL; 
        }
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

    if (m->acumuladorArremesso > 0)
        m->acumuladorArremesso -= dt;
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
    if (spriteAtual.id == 0) return;
    float escala = 2.0f; // Dobra o tamanho do monstro

    Vector2 posSprite = {
        m->posicao.x - (spriteAtual.width * escala) / 2.0f,
        m->posicao.y - (spriteAtual.height * escala) / 2.0f
    };

    DrawTextureEx(spriteAtual, posSprite, 0.0f, escala, WHITE);

    if (m->vidaMaxima > 0.0f) {
        float barraLarg = spriteAtual.width * escala * 0.7f;
        float barraAlt = 6.0f;
        float topoSprite = posSprite.y;
        Rectangle fundo = {
            m->posicao.x - barraLarg / 2.0f,
            topoSprite - barraAlt - 6.0f,
            barraLarg,
            barraAlt
        };
        DrawRectangleRec(fundo, (Color){30, 30, 30, 220});

        float proporcao = m->vida / m->vidaMaxima;
        if (proporcao < 0.0f) proporcao = 0.0f;
        if (proporcao > 1.0f) proporcao = 1.0f;
        Rectangle barra = fundo;
        barra.width *= proporcao;
        DrawRectangleRec(barra, (Color){200, 60, 60, 240});
    }
}

Vector2 GerarMonstros(struct Jogador *jogador, int mapL, int mapC, int tileW, int tileH)
{
    Vector2 resultado = {0, 0};

    if (jogador == NULL || jogador->noAtual == NULL)
        return resultado;

    int jl = jogador->noAtual->linha;
    int jc = jogador->noAtual->coluna;

    // Distância mínima (em tiles) do jogador para o spawn. 
    // 15 tiles deve ser suficiente para garantir que o monstro nasça fora da tela.
    const int DISTANCIA_MINIMA_TILES = 15; 
    
    // Limites de busca (a área de grama, excluindo cercas)
    const int LINHA_MIN_MAP = 1;
    const int LINHA_MAX_MAP = mapL - 2;
    const int COLUNA_MIN_MAP = 1;
    const int COLUNA_MAX_MAP = mapC - 2;

    int linha, coluna;
    int maxTentativas = 300; // Aumentar tentativas para ser robusto

    while (maxTentativas-- > 0)
    {
        // 1. Escolhe um ponto aleatório em qualquer lugar da área jogável do mapa.
        linha = GetRandomValue(LINHA_MIN_MAP, LINHA_MAX_MAP);
        coluna = GetRandomValue(COLUNA_MIN_MAP, COLUNA_MAX_MAP);
        
        // 2. Calcula a distância em tiles (dl = delta linha, dc = delta coluna)
        int dl = abs(linha - jl);
        int dc = abs(coluna - jc);
        
        // 3. O spawn só é válido se a distância em LINHA OU COLUNA for maior que a distância mínima.
        // Isso garante que o ponto gerado está LONGE o suficiente do jogador, fora da tela.
        if (dl >= DISTANCIA_MINIMA_TILES || dc >= DISTANCIA_MINIMA_TILES)
        {
            // Ponto de spawn encontrado!
            // Converte coordenadas do nó para posição em pixels (centro do tile)
            resultado.x = coluna * tileW + tileW / 2;
            resultado.y = linha * tileH + tileH / 2;
            return resultado;
        }
    }
    
    // Fallback de emergência (deve estar longe, no canto)
    // Se falhar nas tentativas, força o spawn no canto superior esquerdo para garantir o afastamento.
    resultado.x = COLUNA_MIN_MAP * tileW + tileW / 2;
    resultado.y = LINHA_MIN_MAP * tileH + tileH / 2;
    return resultado;
}

// IA básica: move continuamente em direção ao jogador
void IAAtualizarMonstro(Monstro *m, struct Jogador *jogador, float dt)
{
    if (!m || !m->ativo || !jogador)
        return;

    // Calcula distância entre monstro e jogador
    float dx = jogador->posicao.x - m->posicao.x;
    float dy = jogador->posicao.y - m->posicao.y;
    float distancia = sqrtf(dx * dx + dy * dy);
    if (distancia <= 0.01f)
        return;

    bool ehArqueiro = (m->tipo == MONSTRO_ESQUELETO || m->tipo == MONSTRO_IT);
    const float LIMITE_PARADA_ARQUEIRO = 140.0f;
    if (ehArqueiro && distancia <= LIMITE_PARADA_ARQUEIRO) {
        return;
    }

    float dirX = dx / distancia;
    float dirY = dy / distancia;
    m->posicao.x += dirX * m->velocidade * dt;
    m->posicao.y += dirY * m->velocidade * dt;
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

bool TentarLancarObjeto(Monstro *m, float dt, Vector2 alvo) {
    if (!m || !m->objeto) return false;
    
    if (m->objeto->ativo) {
        return false;
    }
    
    if (m->acumuladorArremesso > 0.0f) {
        m->acumuladorArremesso -= dt;
        if (m->acumuladorArremesso > 0.0f) {
            return false;
        }
    }
    
    float dx = alvo.x - m->posicao.x;
    float dy = alvo.y - m->posicao.y;
    float distancia = sqrtf(dx * dx + dy * dy);
    if (distancia <= 0.01f) {
        return false;
    }

    if (distancia > m->alcanceAtaque) {
        return false;
    }
    
    if (m->acumuladorArremesso <= 0.0f) {
        m->acumuladorArremesso = m->cooldownArremesso; 
        
        m->objeto->posicao = m->posicao;
        m->objeto->direcao = (Vector2){dx / distancia, dy / distancia}; 
        m->objeto->ativo = true;
        
        return true;
    }
    return false;
}
