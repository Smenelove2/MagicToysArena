#include "objeto.h"
#include "jogador.h"
#include <math.h>
#include <string.h>

bool IniciarObjeto(ObjetoLancavel* o, const char* caminhoSprite) {
    if (!o) return false;

    o->sprite = LoadTexture(caminhoSprite);
    if (o->sprite.id == 0) {
        o->ativo = false;
        return false; 
    }
    if (caminhoSprite) {
        strncpy(o->caminhoSprite, caminhoSprite, sizeof(o->caminhoSprite) - 1);
        o->caminhoSprite[sizeof(o->caminhoSprite) - 1] = '\0';
    } else {
        o->caminhoSprite[0] = '\0';
    }

    o->posicao = (Vector2){0, 0};
    o->direcao = (Vector2){0, 0};
    o->ativo = false;
    o->tempoVida = 0.0f;
    
    return true;
}

void DescarregarObjeto(ObjetoLancavel* o) {
    if (!o) return;
    if (o->sprite.id != 0) {
        UnloadTexture(o->sprite);
    }
    memset(o, 0, sizeof(*o));
}

void AtualizarObjeto(ObjetoLancavel* o, float dt) {
    if (!o || !o->ativo) return;

    // 1. Movimento: Posição += Direção * Velocidade * Tempo
    o->posicao.x += o->direcao.x * o->velocidade * dt;
    o->posicao.y += o->direcao.y * o->velocidade * dt;

    // 2. Contador de vida
    o->tempoVida += dt;

    // 3. Fim de vida (se for a constante MAX_TEMPO do objeto.h)
    if (o->tempoVida >= MAX_TEMPO) {
        o->ativo = false;
        o->tempoVida = 0.0f; 
    }
}

void DesenharObjeto(const ObjetoLancavel* o) {
    if (!o || !o->ativo) return;
    
    float escala = 1.0f; 
    DrawTextureEx(
        o->sprite,
        (Vector2){o->posicao.x - (o->sprite.width * escala) / 2,
                  o->posicao.y - (o->sprite.height * escala) / 2},
        0.0f,
        escala,
        WHITE);
}

bool VerificarColisaoObjetoJogador(ObjetoLancavel* o, struct Jogador* jogador) {
    if (!o || !o->ativo || !jogador) return false;

    const float raioObjeto = 10.0f; 
    const float raioJogador = 20.0f;

    float dx = o->posicao.x - jogador->posicao.x;
    float dy = o->posicao.y - jogador->posicao.y;
    float distancia = sqrtf(dx * dx + dy * dy);

    return distancia < (raioObjeto + raioJogador);
}
