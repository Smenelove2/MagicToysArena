#include "monstro.h"
#include "objeto.h"
#include "jogador.h"
#include "mapa.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

void CarregarAssetsMonstro(Monstro *m)
{
    if (!m)
        return;

    // Carrega sprites baseado no tipo de monstro
    switch (m->tipo)
    {
    case MONSTRO_ESQUELETO:
        m->sprite1 = LoadTexture("assets/esqueleto/esqueleto1.png");
        m->sprite2 = LoadTexture("assets/esqueleto/esqueleto2.png");
        m->sprite3 = LoadTexture("assets/esqueleto/esqueleto3.png");
        break;
    case MONSTRO_ZOMBIE:
        m->sprite1 = LoadTexture("assets/zombie/zombie1.png");
        m->sprite2 = LoadTexture("assets/zombie/zombie2.png");
        m->sprite3 = LoadTexture("assets/zombie/zombie3.png");
        break;
    case MONSTRO_URSO:
        m->sprite1 = LoadTexture("assets/urso/urso1.png");
        m->sprite2 = LoadTexture("assets/urso/urso2.png");
        m->sprite3 = LoadTexture("assets/urso/urso3.png");
        break;
    case MONSTRO_IT:
        m->sprite1 = LoadTexture("assets/IT/IT-1.png");
        m->sprite2 = LoadTexture("assets/IT/IT-2.png");
        m->sprite3 = LoadTexture("assets/IT/IT-3.png");
        break;
    case MONSTRO_CHUCKY:
        m->sprite1 = LoadTexture("assets/chucky/CHUCKY-1.png"); 
        m->sprite2 = LoadTexture("assets/chucky/CHUCKY-2.png");
        m->sprite3 = LoadTexture("assets/chucky/CHUCKY-3.png");
        break;
    case MONSTRO_MINECRAFT:
        m->sprite1 = LoadTexture("assets/minecraft/minecraft-1.png");
        m->sprite2 = LoadTexture("assets/minecraft/minecraft-2.png");
        m->sprite3 = LoadTexture("assets/minecraft/minecraft-3.png");
        break;
        
    case MONSTRO_ROXO:
        m->sprite1 = LoadTexture("assets/roxo/MONSTRO-1.png");
        m->sprite2 = LoadTexture("assets/roxo/MONSTRO-2.png");
        m->sprite3 = LoadTexture("assets/roxo/MONSTRO-3.png");
        break;

    case MONSTRO_SUPREMO:
        m->sprite1 = LoadTexture("assets/supremo/supremo-1.png");
        m->sprite2 = LoadTexture("assets/supremo/supremo-2.png");
        m->sprite3 = LoadTexture("assets/supremo/supremo-3.png");
        break;
    default:
        m->sprite1 = LoadTexture("assets/esqueleto/esqueleto1.png");
        m->sprite2 = LoadTexture("assets/esqueleto/esqueleto2.png");
        m->sprite3 = LoadTexture("assets/esqueleto/esqueleto3.png");
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

    m->cooldownArremesso = 2.0f; 
    m->acumuladorArremesso = 0.0f;
    
    m->objeto = NULL; 

    if (m->tipo == MONSTRO_ESQUELETO || m->tipo == MONSTRO_IT){
    const char* caminhoSprite = NULL;
    
    if (m->tipo == MONSTRO_ESQUELETO) {
        caminhoSprite = "assets/esqueleto/pedra.png";
    } else { 
        caminhoSprite = "assets/IT/balao.png";
    }

    if (caminhoSprite != NULL) {
    
    m->objeto = (struct ObjetoLancavel *)malloc(sizeof(ObjetoLancavel));
    if (!m->objeto){
        return false;
    }

    memset(m->objeto, 0, sizeof(ObjetoLancavel));

    m->objeto->dano = (m->tipo == MONSTRO_ESQUELETO) ? 10.0f : 5.0f;
    m->objeto->velocidade = (m->tipo == MONSTRO_ESQUELETO) ? 400.0f : 200.0f;

    if(!IniciarObjeto(m->objeto, caminhoSprite)){
        free(m->objeto);
        m->objeto = NULL; 
    }
    }
} else {
    m->objeto = NULL; 
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

bool TentarLancarObjeto(Monstro *m, float dt, Vector2 alvo) {
    if (m->tipo != MONSTRO_ESQUELETO && m->tipo != MONSTRO_IT) {
        return false;
    }
    
    if (!m->objeto || m->objeto->ativo) {
        return false; 
    }
    
    float dx = alvo.x - m->posicao.x;
    float dy = alvo.y - m->posicao.y;
    float distancia = sqrtf(dx * dx + dy * dy);
    
    if (distancia > m->raioDeteccao) {
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