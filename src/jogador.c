#include "jogador.h"
#include "mapa.h"  
#include <math.h>
#include <string.h>

#define MAP_L 65 
#define MAP_C 65

static inline float ComprimentoVetor(float x, float y) {
    return sqrtf(x*x + y*y);
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
    j->vida       = vida;

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

    float escala = 1.0f; // ⬅️ dobra o tamanho do personagem

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

Vector4 DistanciaBordasJogador(const Jogador* j) {
    Vector4 distancias = { 0.0f, 0.0f, 0.0f, 0.0f };

    if (!j || !j->noAtual) {
        return distancias;
    }

    const int i = j->noAtual->linha;
    const int jx = j->noAtual->coluna;
    
    distancias.y = (float)i; 
    
    distancias.w = (float)(MAP_L - 1 - i); 
    
    distancias.x = (float)jx; 
    
    distancias.z = (float)(MAP_C - 1 - jx); 
    
    return distancias;
}

