#ifndef ARMA_PRINCIPAL_H
#define ARMA_PRINCIPAL_H

#include "raylib.h"
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    TIPO_ALCANCE_CORPO = 0,
    TIPO_ALCANCE_DISTANCIA
} TipoAlcanceArma;

typedef enum {
    TIPO_DISPARO_AUTOMATICO = 0,
    TIPO_DISPARO_POINT_CLICK
} TipoDisparoArma;

typedef enum {
    TIPO_AREA_NENHUMA = 0,
    TIPO_AREA_CONE,
    TIPO_AREA_PONTO,
    TIPO_AREA_LINHA
} TipoAreaArma;

typedef struct {
    const char *nome;
    const char *descricao;
    TipoAlcanceArma alcance;
    bool ataqueEmArea;
    TipoAreaArma formatoArea;
    TipoDisparoArma tipoDisparo;
    float alcanceMaximo;
    float danoBase;
    float tempoRecarga;
    float tempoRecargaRestante;
    float aberturaConeGraus;
    const char *spriteBase;
    Texture2D spriteIdleOuFrame1;
    Texture2D spriteFrame2;
    Vector2 offsetFrame1;
    Vector2 offsetFrame2;
    float larguraLinha;
    float danoBaseOriginal;
    float tempoRecargaOriginal;
} ArmaPrincipal;

typedef struct {
    bool ativo;
    TipoAreaArma formato;
    bool ataqueEmArea;
    Vector2 origem;
    Vector2 destino;
    Vector2 direcao;
    float alcance;
    float raio;
    float larguraLinha;
    float coneAberturaGraus;
    float tempoRestante;
    Color cor;
} EfeitoVisualArmaPrincipal;

extern ArmaPrincipal gArmasPrincipais[];
extern const size_t gQuantidadeArmasPrincipais;

ArmaPrincipal *ObterArmaPrincipalPorIndice(size_t indice);
ArmaPrincipal *ObterArmaPrincipalPorNome(const char *nome);
bool CarregarSpritesArmaPrincipal(ArmaPrincipal *arma, const char *diretorioSprites);
void DescarregarSpritesArmaPrincipal(ArmaPrincipal *arma);
void DesenharArmaPrincipal(const ArmaPrincipal *arma, Vector2 posicaoCentroJogador,
                           bool emMovimento, bool frameAlternado, float escala);
const char *GerarNomeSpriteArmaPrincipal(const char *nome, char *buffer, int tamanhoBuffer);
void AtualizarArmaPrincipal(ArmaPrincipal *arma, float dt);
bool PodeAtacarArmaPrincipal(const ArmaPrincipal *arma);
bool AcionarAtaqueArmaPrincipal(ArmaPrincipal *arma, Vector2 origem, Vector2 alvo,
                                EfeitoVisualArmaPrincipal *efeito);
void GerarEfeitoArmaPrincipal(const ArmaPrincipal *arma, Vector2 origem, Vector2 destino,
                              Vector2 direcao, EfeitoVisualArmaPrincipal *efeito);
void AtualizarEfeitoArmaPrincipal(EfeitoVisualArmaPrincipal *efeito, float dt);
void DesenharEfeitoArmaPrincipal(const EfeitoVisualArmaPrincipal *efeito);

#endif // ARMA_PRINCIPAL_H
