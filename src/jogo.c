#include "jogo.h"
#include "ui_utils.h"
#include "mapa.h"
#include "monstro_dados.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define RAYGUN_PROJETIL_VELOCIDADE 650.0f
#define INTERVALO_SPAWN_FIXO 0.9f

static float ComprimentoV2(Vector2 v);
static Vector2 NormalizarV2(Vector2 v);
static float ProdutoEscalar(Vector2 a, Vector2 b);

static void SolicitarRetornoMenu(EstadoJogo *estado)
{
    if (!estado) return;
    estado->pausado = false;
    estado->projetilRaygun.ativo = false;
    estado->armaSecundaria.ativo = false;
    estado->armaSecundaria.dados = NULL;
    estado->cooldownArmaSecundaria = 0.0f;
    estado->solicitouRetornoMenu = true;
}

static bool EscudoProtegePosicao(const EstadoArmaSecundaria *estadoSec, Vector2 posicao, Vector2 posJogador)
{
    if (!estadoSec || !estadoSec->ativo || !estadoSec->dados) return false;
    if (estadoSec->dados->tipo != TIPO_ARMA_SECUNDARIA_DEFESA_TEMPORARIA) return false;
    float raio = (estadoSec->dados->raioOuAlcance > 0.0f) ? estadoSec->dados->raioOuAlcance : 90.0f;
    Vector2 centro = estadoSec->segueJogador ? posJogador : estadoSec->centro;
    Vector2 delta = { posicao.x - centro.x, posicao.y - centro.y };
    return ComprimentoV2(delta) <= raio;
}

static bool PontoDentroConeSecundaria(Vector2 origem, Vector2 direcao, float alcance, float abertura, Vector2 ponto)
{
    Vector2 delta = { ponto.x - origem.x, ponto.y - origem.y };
    float dist = ComprimentoV2(delta);
    if (dist <= 0.0001f || dist > alcance) return false;
    Vector2 dirNorm = NormalizarV2(direcao);
    Vector2 pontoNorm = { delta.x / dist, delta.y / dist };
    float dot = ProdutoEscalar(dirNorm, pontoNorm);
    if (dot > 1.0f) dot = 1.0f;
    if (dot < -1.0f) dot = -1.0f;
    float angulo = acosf(dot) * RAD2DEG;
    return angulo <= (abertura * 0.5f);
}

static void AplicarEfeitosArmaSecundaria(EstadoJogo *estado, Jogador *jogador, float dt)
{
    if (!estado || !jogador) return;
    EstadoArmaSecundaria *sec = &estado->armaSecundaria;
    if (!sec->ativo || !sec->dados) return;
    Vector2 centro = sec->segueJogador ? jogador->posicao : sec->centro;

    switch (sec->dados->tipo) {
        case TIPO_ARMA_SECUNDARIA_AREA_PULSANTE: {
            float raio = (sec->dados->raioOuAlcance > 0.0f) ? sec->dados->raioOuAlcance : 150.0f;
            float danoTick = sec->dados->dano * dt;
            if (danoTick <= 0.0f) break;
            for (int i = 0; i < MAX_MONSTROS; ++i) {
                Monstro *monstro = &estado->monstros[i];
                if (!monstro->ativo) continue;
                Vector2 delta = { monstro->posicao.x - centro.x, monstro->posicao.y - centro.y };
                if (ComprimentoV2(delta) <= raio) {
                    monstro->vida -= danoTick;
                }
            }
        } break;
        case TIPO_ARMA_SECUNDARIA_CONE_EMPURRAO: {
            if (sec->impactoAplicado) break;
            Vector2 dir = sec->direcao;
            if (ComprimentoV2(dir) <= 0.0001f) dir = (Vector2){1.0f, 0.0f};
            float alcance = (sec->dados->raioOuAlcance > 0.0f) ? sec->dados->raioOuAlcance : 200.0f;
            float abertura = 80.0f;
            float empurrao = alcance * 0.4f;
            for (int i = 0; i < MAX_MONSTROS; ++i) {
                Monstro *monstro = &estado->monstros[i];
                if (!monstro->ativo) continue;
                if (PontoDentroConeSecundaria(centro, dir, alcance, abertura, monstro->posicao)) {
                    monstro->vida -= sec->dados->dano;
                    monstro->posicao.x += dir.x * empurrao;
                    monstro->posicao.y += dir.y * empurrao;
                }
            }
            sec->impactoAplicado = true;
        } break;
        default:
            break;
    }
}

static float ComprimentoV2(Vector2 v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

static float CalcularIntervaloSpawnDinamico(float tempoTotal)
{
    (void)tempoTotal;
    return INTERVALO_SPAWN_FIXO;
}

static Vector2 NormalizarV2(Vector2 v)
{
    float len = ComprimentoV2(v);
    if (len <= 0.0001f) return (Vector2){1.0f, 0.0f};
    return (Vector2){ v.x / len, v.y / len };
}

static float ProdutoEscalar(Vector2 a, Vector2 b)
{
    return a.x * b.x + a.y * b.y;
}

static float DistanciaPontoSegmento(Vector2 ponto, Vector2 inicio, Vector2 fim)
{
    Vector2 segmento = { fim.x - inicio.x, fim.y - inicio.y };
    float comprimento2 = segmento.x * segmento.x + segmento.y * segmento.y;
    if (comprimento2 <= 0.0001f) {
        return ComprimentoV2((Vector2){ ponto.x - inicio.x, ponto.y - inicio.y });
    }
    float t = ProdutoEscalar((Vector2){ ponto.x - inicio.x, ponto.y - inicio.y }, segmento) / comprimento2;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    Vector2 proj = { inicio.x + segmento.x * t, inicio.y + segmento.y * t };
    return ComprimentoV2((Vector2){ ponto.x - proj.x, ponto.y - proj.y });
}

static void ResetarMonstros(EstadoJogo *estado)
{
    if (!estado) return;
    for (int i = 0; i < MAX_MONSTROS; ++i) {
        if (estado->monstros[i].sprite1.id != 0 ||
            estado->monstros[i].sprite2.id != 0 ||
            estado->monstros[i].sprite3.id != 0) {
            DescarregarMonstro(&estado->monstros[i]);
        } else {
            memset(&estado->monstros[i], 0, sizeof(Monstro));
        }
        estado->monstros[i].ativo = false;
    }
    for (int i = 0; i < MAX_OBJETOS_VOO; ++i) {
        if (estado->objetosEmVoo[i].sprite.id != 0) {
            UnloadTexture(estado->objetosEmVoo[i].sprite);
        }
    }
    memset(estado->objetosEmVoo, 0, sizeof(estado->objetosEmVoo));
    estado->monstrosAtivos = 0;
    estado->tempoSpawnMonstro = 0.0f;
    estado->tempoTotalJogo = 0.0f;
    estado->intervaloSpawnMonstro = INTERVALO_SPAWN_FIXO;
}

static void DesativarMonstro(EstadoJogo *estado, Monstro *monstro, bool concederPontos)
{
    if (!estado || !monstro || !monstro->ativo) return;
    if (concederPontos && monstro->pontuacao > 0) {
        estado->pontuacaoTotal += monstro->pontuacao;
    }
    DescarregarMonstro(monstro);
    monstro->ativo = false;
    if (estado->monstrosAtivos > 0) estado->monstrosAtivos--;
}

static bool PontoDentroDoCone(Vector2 ponto, const EfeitoVisualArmaPrincipal *efeito)
{
    Vector2 delta = { ponto.x - efeito->origem.x, ponto.y - efeito->origem.y };
    float distancia = ComprimentoV2(delta);
    float alcance = (efeito->alcance > 0.0f) ? efeito->alcance : 1.0f;
    if (distancia > alcance) return false;
    Vector2 direcaoPonto = NormalizarV2(delta);
    float dot = ProdutoEscalar(direcaoPonto, efeito->direcao);
    if (dot > 1.0f) dot = 1.0f;
    if (dot < -1.0f) dot = -1.0f;
    float angulo = acosf(dot) * RAD2DEG;
    return angulo <= (efeito->coneAberturaGraus * 0.5f);
}

static bool PontoDentroDoEfeito(const Monstro *monstro, const EfeitoVisualArmaPrincipal *efeito)
{
    if (!monstro || !efeito) return false;
    const float raioMonstro = 18.0f;
    Vector2 pos = monstro->posicao;

    switch (efeito->formato) {
        case TIPO_AREA_CONE:
            return PontoDentroDoCone(pos, efeito);
        case TIPO_AREA_PONTO: {
            float raio = (efeito->raio > 0.0f) ? efeito->raio : 32.0f;
            return ComprimentoV2((Vector2){ pos.x - efeito->destino.x, pos.y - efeito->destino.y }) <= (raio + raioMonstro);
        }
        case TIPO_AREA_LINHA:
        case TIPO_AREA_NENHUMA: {
            float largura = efeito->larguraLinha > 0.0f ? efeito->larguraLinha : 18.0f;
            float distancia = DistanciaPontoSegmento(pos, efeito->origem, efeito->destino);
            return distancia <= (largura * 0.5f + raioMonstro);
        }
        default:
            return false;
    }
}

static void AplicarDanoMonstrosEfeito(EstadoJogo *estado, const EfeitoVisualArmaPrincipal *efeito, float dano)
{
    if (!estado || !efeito || dano <= 0.0f) return;
    for (int i = 0; i < MAX_MONSTROS; ++i) {
        Monstro *monstro = &estado->monstros[i];
        if (!monstro->ativo) continue;
        if (PontoDentroDoEfeito(monstro, efeito)) {
            monstro->vida -= dano;
            if (monstro->vida <= 0.0f) {
                DesativarMonstro(estado, monstro, true);
            }
        }
    }
}

static void RegistrarObjetoLancado(EstadoJogo *estado, const ObjetoLancavel *origem)
{
    if (!estado || !origem) return;
    for (int k = 0; k < MAX_OBJETOS_VOO; ++k) {
        if (!estado->objetosEmVoo[k].ativo) {
            if (estado->objetosEmVoo[k].sprite.id != 0) {
                UnloadTexture(estado->objetosEmVoo[k].sprite);
            }
            estado->objetosEmVoo[k] = *origem;
            estado->objetosEmVoo[k].tempoVida = 0.0f;
            if (estado->objetosEmVoo[k].caminhoSprite[0] != '\0') {
                estado->objetosEmVoo[k].sprite = LoadTexture(estado->objetosEmVoo[k].caminhoSprite);
            }
            return;
        }
    }
}

static void AtualizarObjetosLancados(EstadoJogo *estado, Jogador *jogador, float dt)
{
    if (!estado || !jogador) return;
    for (int i = 0; i < MAX_OBJETOS_VOO; ++i) {
        ObjetoLancavel *obj = &estado->objetosEmVoo[i];
        if (!obj->ativo) continue;
        AtualizarObjeto(obj, dt);
        if (EscudoProtegePosicao(&estado->armaSecundaria, obj->posicao, jogador->posicao)) {
            obj->ativo = false;
            continue;
        }
        if (VerificarColisaoObjetoJogador(obj, jogador)) {
            if (!EscudoProtegePosicao(&estado->armaSecundaria, jogador->posicao, jogador->posicao)) {
                jogador->vida -= obj->dano;
                if (jogador->vida < 0.0f) jogador->vida = 0.0f;
            }
            obj->ativo = false;
            continue;
        }
        if (obj->tempoVida > MAX_TEMPO) {
            obj->ativo = false;
        }
    }
}

static void DesenharObjetosLancados(const EstadoJogo *estado)
{
    if (!estado) return;
    for (int i = 0; i < MAX_OBJETOS_VOO; ++i) {
        if (estado->objetosEmVoo[i].ativo) {
            DesenharObjeto(&estado->objetosEmVoo[i]);
        }
    }
}

void JogoInicializar(EstadoJogo *estado, float regeneracaoBase)
{
    if (!estado) return;
    memset(estado, 0, sizeof(*estado));
    estado->regeneracaoAtual = regeneracaoBase;
    estado->solicitouRetornoMenu = false;
    estado->pontuacaoTotal = 0;
    estado->jogadorMorto = false;
    ResetarMonstros(estado);
}

void JogoReiniciar(EstadoJogo *estado, Jogador *jogador, Camera2D *camera, Vector2 posInicial)
{
    if (!estado || !jogador || !camera) return;
    estado->projetilRaygun = (ProjetilRaygun){0};
    estado->armaSecundaria = (EstadoArmaSecundaria){0};
    estado->cooldownArmaSecundaria = 0.0f;
    estado->pausado = false;
    estado->efeitoArmaPrincipal.ativo = false;
    estado->solicitouRetornoMenu = false;
    estado->pontuacaoTotal = 0;
    estado->jogadorMorto = false;
    estado->tempoSpawnMonstro = 0.0f;
    estado->tempoTotalJogo = 0.0f;
    estado->intervaloSpawnMonstro = INTERVALO_SPAWN_FIXO;
    ResetarMonstros(estado);
    jogador->posicao = posInicial;
    camera->target = jogador->posicao;
}

static void AtualizarProjetilRaygun(EstadoJogo *estado, float dt)
{
    if (!estado || !estado->projetilRaygun.ativo) return;
    estado->projetilRaygun.posicao.x += estado->projetilRaygun.velocidade.x * dt;
    estado->projetilRaygun.posicao.y += estado->projetilRaygun.velocidade.y * dt;
    for (int i = 0; i < MAX_MONSTROS; ++i) {
        Monstro *monstro = &estado->monstros[i];
        if (!monstro->ativo) continue;
        float distancia = ComprimentoV2((Vector2){ monstro->posicao.x - estado->projetilRaygun.posicao.x,
                                                   monstro->posicao.y - estado->projetilRaygun.posicao.y });
        if (distancia <= 18.0f) {
            float dano = estado->projetilRaygun.arma ? estado->projetilRaygun.arma->danoBase : 0.0f;
            monstro->vida -= dano;
            if (monstro->vida <= 0.0f) {
                DesativarMonstro(estado, monstro, true);
            }
            estado->projetilRaygun.ativo = false;
            estado->projetilRaygun.arma = NULL;
            return;
        }
    }
    estado->projetilRaygun.tempoRestante -= dt;
    if (estado->projetilRaygun.tempoRestante <= 0.0f) {
        estado->projetilRaygun.ativo = false;
        estado->projetilRaygun.arma = NULL;
    }
}

static bool TentarSpawnMonstro(EstadoJogo *estado,
                               Jogador *jogador,
                               int linhasMapa,
                               int colunasMapa,
                               int tileLargura,
                               int tileAltura)
{
    if (!estado || !jogador) return false;
    for (int i = 0; i < MAX_MONSTROS; ++i) {
        Monstro *monstro = &estado->monstros[i];
        if (monstro->ativo) continue;

        Vector2 spawn = GerarMonstros(jogador, linhasMapa, colunasMapa, tileLargura, tileAltura);
        TipoMonstro tipo = (TipoMonstro)GetRandomValue(0, MONSTRO_TIPOS_COUNT - 1);
        const MonstroInfo *info = ObterInfoMonstro(tipo);
        if (!info) continue;
        memset(monstro, 0, sizeof(Monstro));
        if (IniciarMonstro(monstro, spawn, info)) {
            CarregarAssetsMonstro(monstro);
            monstro->ativo = true;
            estado->monstrosAtivos++;
            return true;
        }
        break;
    }
    return false;
}

void JogoAtualizar(EstadoJogo *estado,
                   Jogador *jogador,
                   Camera2D *camera,
                   Mapa **mapa,
                   int linhasMapa,
                   int colunasMapa,
                   int tileLargura,
                   int tileAltura,
                   float dt,
                   Vector2 mouseNoMundo,
                   bool mouseClickEsq,
                   bool mouseClickDir,
                   bool escapePress,
                   const Armadura *armaduraAtual,
                   const Capacete *capaceteAtual,
                   ArmaPrincipal *armaPrincipalAtual,
                   const ArmaSecundaria *armaSecundariaAtual)
{
    if (!estado || !jogador || !camera) return;
    (void)armaduraAtual;
    (void)capaceteAtual;

    if (estado->jogadorMorto) {
        AtualizarProjetilRaygun(estado, dt);
        return;
    }

    if (escapePress) estado->pausado = !estado->pausado;

    if (estado->cooldownArmaSecundaria > 0.0f) {
        estado->cooldownArmaSecundaria -= dt;
        if (estado->cooldownArmaSecundaria < 0.0f) estado->cooldownArmaSecundaria = 0.0f;
    }

    if (estado->armaSecundaria.ativo) {
        estado->armaSecundaria.tempoRestante -= dt;
        estado->armaSecundaria.tempoDecorrido += dt;
        if (estado->armaSecundaria.segueJogador) {
            estado->armaSecundaria.centro = jogador->posicao;
        }
        if (estado->armaSecundaria.tempoRestante <= 0.0f) {
            estado->armaSecundaria.ativo = false;
            estado->armaSecundaria.dados = NULL;
        }
    }

    if (!estado->pausado) {
        AtualizarArmaPrincipal(armaPrincipalAtual, dt);
        AtualizarEfeitoArmaPrincipal(&estado->efeitoArmaPrincipal, dt);
        estado->tempoTotalJogo += dt;
        estado->intervaloSpawnMonstro = CalcularIntervaloSpawnDinamico(estado->tempoTotalJogo);

        Vector2 posAnterior = jogador->posicao;
        AtualizarJogador(jogador, dt);
        AtualizarNoAtualJogador(jogador, mapa, linhasMapa, colunasMapa, tileLargura, tileAltura);

        if (jogador->noAtual && jogador->noAtual->colisao) {
            jogador->posicao = posAnterior;
            AtualizarNoAtualJogador(jogador, mapa, linhasMapa, colunasMapa, tileLargura, tileAltura);
        }

        float maxX = colunasMapa * (float)tileLargura - 1.0f;
        float maxY = linhasMapa * (float)tileAltura - 1.0f;
        if (jogador->posicao.x < 0) jogador->posicao.x = 0;
        if (jogador->posicao.y < 0) jogador->posicao.y = 0;
        if (jogador->posicao.x > maxX) jogador->posicao.x = maxX;
        if (jogador->posicao.y > maxY) jogador->posicao.y = maxY;

        camera->target = jogador->posicao;

        if (armaPrincipalAtual && mouseClickEsq) {
            bool ehRaygun = armaPrincipalAtual && (strcmp(armaPrincipalAtual->nome, "RayGun") == 0);
            if (ehRaygun) {
                if (PodeAtacarArmaPrincipal(armaPrincipalAtual)) {
                    Vector2 delta = { mouseNoMundo.x - jogador->posicao.x, mouseNoMundo.y - jogador->posicao.y };
                    Vector2 direcao = NormalizarV2(delta);
                    float distancia = ComprimentoV2(delta);
                    if (distancia <= 0.01f) distancia = armaPrincipalAtual->alcanceMaximo;
                    float alcanceUtil = fminf(distancia, armaPrincipalAtual->alcanceMaximo);
                    if (alcanceUtil <= 0.0f) alcanceUtil = armaPrincipalAtual->alcanceMaximo;
                    Vector2 destino = {
                        jogador->posicao.x + direcao.x * alcanceUtil,
                        jogador->posicao.y + direcao.y * alcanceUtil
                    };

                    armaPrincipalAtual->tempoRecargaRestante = armaPrincipalAtual->tempoRecarga;
                    estado->projetilRaygun.ativo = true;
                    estado->projetilRaygun.arma = armaPrincipalAtual;
                    estado->projetilRaygun.origem = jogador->posicao;
                    estado->projetilRaygun.posicao = jogador->posicao;
                    estado->projetilRaygun.destino = destino;
                    estado->projetilRaygun.direcao = direcao;
                    estado->projetilRaygun.velocidade = (Vector2){
                        direcao.x * RAYGUN_PROJETIL_VELOCIDADE,
                        direcao.y * RAYGUN_PROJETIL_VELOCIDADE
                    };
                    float distProj = ComprimentoV2((Vector2){ destino.x - jogador->posicao.x, destino.y - jogador->posicao.y });
                    float tempoProj = distProj / RAYGUN_PROJETIL_VELOCIDADE;
                    if (tempoProj < 0.08f) tempoProj = 0.08f;
                    estado->projetilRaygun.tempoRestante = tempoProj;
                }
            } else {
                if (PodeAtacarArmaPrincipal(armaPrincipalAtual)) {
                    bool sucesso = AcionarAtaqueArmaPrincipal(
                        armaPrincipalAtual,
                        jogador->posicao,
                        mouseNoMundo,
                        &estado->efeitoArmaPrincipal
                    );
                    if (sucesso) {
                        AplicarDanoMonstrosEfeito(estado, &estado->efeitoArmaPrincipal, armaPrincipalAtual->danoBase);
                    }
                }
            }
        }

        if (estado->projetilRaygun.ativo) {
            AtualizarProjetilRaygun(estado, dt);
        }

        if (estado->regeneracaoAtual > 0.0f && jogador->vida < jogador->vidaMaxima) {
            jogador->vida += estado->regeneracaoAtual * dt;
            if (jogador->vida > jogador->vidaMaxima) jogador->vida = jogador->vidaMaxima;
        }

        estado->tempoSpawnMonstro += dt;
        if (estado->tempoSpawnMonstro >= estado->intervaloSpawnMonstro &&
            estado->monstrosAtivos < MAX_MONSTROS) {
            estado->tempoSpawnMonstro = 0.0f;
            TentarSpawnMonstro(estado, jogador, linhasMapa, colunasMapa, tileLargura, tileAltura);
        }

        for (int i = 0; i < MAX_MONSTROS; ++i) {
            Monstro *monstro = &estado->monstros[i];
            if (!monstro->ativo) continue;
            AtualizarMonstro(monstro, dt);
            float velocidadeOriginal = monstro->velocidade;
            if (estado->armaSecundaria.ativo &&
                estado->armaSecundaria.dados &&
                estado->armaSecundaria.dados->tipo == TIPO_ARMA_SECUNDARIA_ZONA_LENTIDAO) {
                Vector2 centroZona = estado->armaSecundaria.segueJogador ? jogador->posicao : estado->armaSecundaria.centro;
                float raioZona = (estado->armaSecundaria.dados->raioOuAlcance > 0.0f) ? estado->armaSecundaria.dados->raioOuAlcance : 160.0f;
                Vector2 deltaZona = { monstro->posicao.x - centroZona.x, monstro->posicao.y - centroZona.y };
                if (ComprimentoV2(deltaZona) <= raioZona) {
                    monstro->velocidade = velocidadeOriginal * 0.4f;
                }
            }
            IAAtualizarMonstro(monstro, jogador, dt);
            monstro->velocidade = velocidadeOriginal;

            if (monstro->objeto && TentarLancarObjeto(monstro, dt, jogador->posicao)) {
                RegistrarObjetoLancado(estado, monstro->objeto);
                monstro->objeto->ativo = false;
            }

            if (VerificarColisaoMonstroJogador(monstro, jogador)) {
                if (!EscudoProtegePosicao(&estado->armaSecundaria, jogador->posicao, jogador->posicao) &&
                    monstro->acumuladorAtaque <= 0.0f) {
                    jogador->vida -= monstro->danoContato;
                    if (jogador->vida < 0.0f) jogador->vida = 0.0f;
                    monstro->acumuladorAtaque = monstro->cooldownAtaque;
                }
            }

            if (monstro->vida <= 0.0f) {
                DesativarMonstro(estado, monstro, true);
            }
        }

        AtualizarObjetosLancados(estado, jogador, dt);
        AplicarEfeitosArmaSecundaria(estado, jogador, dt);

        if (armaSecundariaAtual && mouseClickDir) {
            if (!(estado->armaSecundaria.ativo || estado->cooldownArmaSecundaria > 0.0f)) {
                estado->cooldownArmaSecundaria = armaSecundariaAtual->tempoRecarga;
                estado->armaSecundaria.ativo = true;
                estado->armaSecundaria.dados = armaSecundariaAtual;
                estado->armaSecundaria.tempoRestante = armaSecundariaAtual->duracao;
                estado->armaSecundaria.tempoDecorrido = 0.0f;
                estado->armaSecundaria.impactoAplicado = false;
                estado->armaSecundaria.centro = jogador->posicao;
                Vector2 dir = NormalizarV2((Vector2){ mouseNoMundo.x - jogador->posicao.x,
                                                      mouseNoMundo.y - jogador->posicao.y });
                estado->armaSecundaria.direcao = dir;
                estado->armaSecundaria.segueJogador =
                    (armaSecundariaAtual->tipo == TIPO_ARMA_SECUNDARIA_DEFESA_TEMPORARIA) ||
                    (armaSecundariaAtual->tipo == TIPO_ARMA_SECUNDARIA_AREA_PULSANTE);
                if (armaSecundariaAtual->tipo == TIPO_ARMA_SECUNDARIA_ZONA_LENTIDAO) {
                    estado->armaSecundaria.centro = mouseNoMundo;
                } else if (armaSecundariaAtual->tipo == TIPO_ARMA_SECUNDARIA_CONE_EMPURRAO) {
                    estado->armaSecundaria.centro = jogador->posicao;
                }
            }
        }
    } else {
        AtualizarProjetilRaygun(estado, dt);
    }

    if (jogador->vida <= 0.0f && !estado->jogadorMorto) {
        jogador->vida = 0.0f;
        estado->jogadorMorto = true;
    }
    if (estado->pausado) {
        // mantém objetos parados quando pausado
    } else {
        // nada extra, já atualizado acima
    }
}

void JogoDesenhar(EstadoJogo *estado,
                  const Jogador *jogador,
                  const Camera2D *camera,
                  Mapa **mapa,
                  Texture2D *tiles,
                  int quantidadeTiles,
                  int idTileForaMapa,
                  int linhasMapa,
                  int colunasMapa,
                  int tileLargura,
                  int tileAltura,
                  int largura, int altura,
                  Font fonteNormal,
                  Font fonteBold,
                  const Armadura *armaduraAtual,
                  const Capacete *capaceteAtual,
                  const ArmaPrincipal *armaPrincipalAtual,
                  const ArmaSecundaria *armaSecundariaAtual,
                  Vector2 mousePos,
                  bool mouseClick)
{
    if (!estado || !jogador || !camera) return;
    (void)fonteNormal;
    const float escalaUI = UI_GetEscala();

    BeginMode2D(*camera);
        DesenharMapaVisivel((Camera2D *)camera, largura, altura,
                            mapa, linhasMapa, colunasMapa,
                            tiles, quantidadeTiles,
                            tileLargura, tileAltura,
                            idTileForaMapa);

        DesenharEfeitoArmaPrincipal(&estado->efeitoArmaPrincipal);
        UI_DesenharEfeitoArmaSecundaria(&estado->armaSecundaria, jogador->posicao);
        if (estado->projetilRaygun.ativo) {
            DrawCircleV(estado->projetilRaygun.posicao, 6.0f, SKYBLUE);
        }
        DesenharJogador(jogador);
        if (capaceteAtual) {
            DesenharCapacete(capaceteAtual, jogador->posicao, 1.0f);
        }
        if (armaduraAtual) {
            DesenharArmadura(armaduraAtual, jogador->posicao, jogador->emMovimento, jogador->alternarFrame, 1.0f);
        }
        if (armaSecundariaAtual) {
            DesenharArmaSecundaria(armaSecundariaAtual, jogador->posicao, jogador->emMovimento, jogador->alternarFrame, 1.0f);
        }
        if (armaPrincipalAtual) {
            DesenharArmaPrincipal(armaPrincipalAtual, jogador->posicao, jogador->emMovimento, jogador->alternarFrame, 1.0f);
        }
        DesenharObjetosLancados(estado);
        for (int i = 0; i < MAX_MONSTROS; ++i) {
            if (estado->monstros[i].ativo) {
                DesenharMonstro(&estado->monstros[i]);
            }
        }
    EndMode2D();

    DrawText("ESC para pausar", 20, 20, UI_AjustarTamanhoFonteInt(20.0f), WHITE);
    DrawText(TextFormat("Pontos: %d", estado->pontuacaoTotal),
             20, 80, UI_AjustarTamanhoFonteInt(20.0f), GOLD);

    float vidaAtual = jogador->vida;
    float vidaMax = jogador->vidaMaxima;
    if (vidaMax <= 0.0f) vidaMax = 1.0f;
    int margemBase = (int)lroundf(20.0f * escalaUI);
    float barraLarg = 300.0f * escalaUI;
    float barraAlt = 28.0f * escalaUI;
    Rectangle barraBase = { (float)margemBase, altura - barraAlt - margemBase, barraLarg, barraAlt };
    DrawRectangleRounded(barraBase, 0.2f, 8, (Color){40, 40, 60, 255});
    float preenchimento = (vidaAtual / vidaMax);
    if (preenchimento < 0.0f) preenchimento = 0.0f;
    if (preenchimento > 1.0f) preenchimento = 1.0f;
    float paddingBarra = 4.0f * escalaUI;
    Rectangle barraVida = { barraBase.x + paddingBarra, barraBase.y + paddingBarra,
                            (barraBase.width - paddingBarra * 2.0f) * preenchimento, barraBase.height - paddingBarra * 2.0f };
    DrawRectangleRounded(barraVida, 0.2f, 8, (Color){200, 60, 60, 255});
    float textoVidaTam = UI_AjustarTamanhoFonte(20.0f);
    DrawTextEx(fonteBold,
               TextFormat("Vida: %.0f / %.0f", vidaAtual, vidaMax),
               (Vector2){ barraBase.x + 10.0f * escalaUI, barraBase.y - textoVidaTam - 4.0f },
               textoVidaTam, 1.0f, WHITE);

    float quadSize = 90.0f * escalaUI;
    float padding = 12.0f * escalaUI;
    Rectangle quadDir = { largura - quadSize - padding, altura - quadSize - padding, quadSize, quadSize };
    Rectangle quadEsq = { quadDir.x - quadSize - padding, quadDir.y, quadSize, quadSize };

    float cdPrincipal = armaPrincipalAtual ? armaPrincipalAtual->tempoRecargaRestante : 0.0f;
    float cdPrincipalMax = armaPrincipalAtual ? armaPrincipalAtual->tempoRecarga : 1.0f;
    UI_DesenharQuadradoCooldown(quadEsq, "Mouse ESQ", cdPrincipal, cdPrincipalMax, fonteBold);

    float cdSec = estado->cooldownArmaSecundaria;
    float cdSecMax = (armaSecundariaAtual) ? armaSecundariaAtual->tempoRecarga : 1.0f;
    UI_DesenharQuadradoCooldown(quadDir, "Mouse DIR", cdSec, cdSecMax, fonteBold);

    if (estado->pausado) {
        DrawRectangle(0, 0, largura, altura, ColorAlpha(BLACK, 0.5f));
        const char *tituloPausa = "Jogo Pausado";
        float pausaTam = UI_AjustarTamanhoFonte(40.0f);
        Vector2 tituloMedida = MeasureTextEx(fonteBold, tituloPausa, pausaTam, 1.0f);
        DrawTextEx(fonteBold, tituloPausa, (Vector2){ largura / 2.0f - tituloMedida.x / 2.0f, altura / 3.0f }, pausaTam, 1.0f, WHITE);

        float larguraBtn = 260.0f * escalaUI;
        float alturaBtn = 60.0f * escalaUI;
        float espacoBtn = 20.0f * escalaUI;
        Rectangle btnRetomar = { largura / 2.0f - larguraBtn / 2.0f, altura / 2.0f - alturaBtn - espacoBtn * 0.5f, larguraBtn, alturaBtn };
        Rectangle btnMenu = { largura / 2.0f - larguraBtn / 2.0f, altura / 2.0f + espacoBtn * 0.5f, larguraBtn, alturaBtn };

        if (UI_BotaoTexto(btnRetomar, "Retomar", mousePos, mouseClick,
                           (Color){80, 120, 80, 255}, (Color){180, 220, 180, 255}, fonteBold, true)) {
            estado->pausado = false;
        }
        if (UI_BotaoTexto(btnMenu, "Sair para o Menu", mousePos, mouseClick,
                           (Color){120, 60, 60, 255}, (Color){220, 120, 120, 255}, fonteBold, true)) {
            SolicitarRetornoMenu(estado);
        }
    }
}

void JogoLiberarRecursos(EstadoJogo *estado)
{
    ResetarMonstros(estado);
}
