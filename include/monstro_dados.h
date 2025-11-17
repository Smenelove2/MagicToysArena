#ifndef MONSTRO_DADOS_H
#define MONSTRO_DADOS_H

#include <stdbool.h>
#include "monstro.h"

typedef struct MonstroInfo {
    TipoMonstro tipo;
    const char *nome;
    const char *sprites[3];
    const char *spriteObjeto;
    bool possuiObjeto;
    float vida;
    float velocidade;
    float fpsAnimacao;
    float alcanceAtaque;
    float cooldownAtaque;
    float danoContato;
    float danoObjeto;
    float velocidadeObjeto;
    float cooldownArremesso;
    int pontuacao;
} MonstroInfo;

extern const MonstroInfo gMonstrosInfo[MONSTRO_TIPOS_COUNT];
const MonstroInfo *ObterInfoMonstro(TipoMonstro tipo);

#endif
