#include "menu.h"
#include "ui_utils.h"
#include "arma_principal.h"
#include "armadura.h"
#include "capacete.h"
#include "arma_secundaria.h"
#include <string.h>

void MenuInicializarEstado(EstadoMenu *estado)
{
    if (!estado) return;
    estado->categoriaSelecionada = CAT_ARMA_PRINCIPAL;
    for (int i = 0; i < CAT_TOTAL; ++i) {
        estado->indicesSelecionados[i] = 0;
    }
}

AcaoMenu MenuDesenharTelaInicial(const EstadoMenu *estado, Vector2 mousePos, bool mouseClick,
                                 Font fonteBold, int largura, int altura)
{
    (void)estado;
    AcaoMenu acao = MENU_ACAO_NENHUMA;
    const float escalaUI = UI_GetEscala();
    const char *titulo = "Magic Toys Arena";
    float tituloTam = UI_AjustarTamanhoFonte(60.0f);
    Vector2 medidaTitulo = MeasureTextEx(fonteBold, titulo, tituloTam, 1.0f);
    DrawTextEx(fonteBold, titulo,
               (Vector2){ largura / 2.0f - medidaTitulo.x / 2.0f, altura * 0.15f },
               tituloTam, 1.0f, WHITE);

    float btnWidth = 320.0f * escalaUI;
    float btnHeight = 70.0f * escalaUI;
    float btnSpacing = 50.0f * escalaUI;
    float baseX = largura / 2.0f - btnWidth / 2.0f;
    float inicioY = altura * 0.35f;
    Rectangle btnJogar = { baseX, inicioY, btnWidth, btnHeight };
    Rectangle btnLeaderboard = { baseX, inicioY + btnHeight + btnSpacing, btnWidth, btnHeight };
    Rectangle btnSair = { baseX, btnLeaderboard.y + btnHeight + btnSpacing, btnWidth, btnHeight };

    if (UI_BotaoTexto(btnJogar, "Jogar", mousePos, mouseClick,
                      (Color){90, 90, 150, 255}, (Color){200, 200, 255, 255}, fonteBold, true)) {
        acao = MENU_ACAO_JOGAR;
    }

    if (UI_BotaoTexto(btnLeaderboard, "Leaderboard", mousePos, mouseClick,
                      (Color){70, 70, 120, 255}, (Color){180, 180, 230, 255}, fonteBold, true)) {
        acao = MENU_ACAO_LEADERBOARD;
    }

    if (UI_BotaoTexto(btnSair, "Sair", mousePos, mouseClick,
                      (Color){120, 60, 60, 255}, (Color){220, 120, 120, 255}, fonteBold, true)) {
        acao = MENU_ACAO_SAIR;
    }

    return acao;
}

ResultadoMenu MenuDesenharTelaConfig(EstadoMenu *estado,
                                     Vector2 mousePos, bool mouseClick,
                                     Font fonteNormal, Font fonteBold,
                                     const Jogador *jogadorBase,
                                     int largura, int altura)
{
    ResultadoMenu resultado = {
        .acao = MENU_ACAO_NENHUMA,
        .armadura = NULL,
        .capacete = NULL,
        .armaPrincipal = NULL,
        .armaSecundaria = NULL
    };
    const float escalaUI = UI_GetEscala();

    float margemTopo = 20.0f * escalaUI;
    float btnPequenoLarg = 150.0f * escalaUI;
    float btnPequenoAlt = 50.0f * escalaUI;
    Rectangle btnVoltar = { margemTopo, margemTopo, btnPequenoLarg, btnPequenoAlt };
    Rectangle btnIniciar = { largura - btnPequenoLarg - margemTopo, margemTopo, btnPequenoLarg, btnPequenoAlt };

    if (UI_BotaoTexto(btnVoltar, "Voltar", mousePos, mouseClick,
                      (Color){80, 80, 120, 255}, (Color){180, 180, 230, 255}, fonteBold, true)) {
        resultado.acao = MENU_ACAO_VOLTAR;
    }

    if (UI_BotaoTexto(btnIniciar, "Jogar", mousePos, mouseClick,
                      (Color){80, 130, 90, 255}, (Color){170, 230, 180, 255}, fonteBold, true)) {
        resultado.acao = MENU_ACAO_JOGAR;
        resultado.capacete = (gQuantidadeCapacetes > 0)
            ? &gCapacetes[estado->indicesSelecionados[CAT_CAPACETE]] : NULL;
        resultado.armadura = (gQuantidadeArmaduras > 0)
            ? &gArmaduras[estado->indicesSelecionados[CAT_ARMADURA]] : NULL;
        resultado.armaPrincipal = (gQuantidadeArmasPrincipais > 0)
            ? &gArmasPrincipais[estado->indicesSelecionados[CAT_ARMA_PRINCIPAL]] : NULL;
        resultado.armaSecundaria = (gQuantidadeArmasSecundarias > 0)
            ? &gArmasSecundarias[estado->indicesSelecionados[CAT_ARMA_SECUNDARIA]] : NULL;
    }

    const char *categoriaLabels[CAT_TOTAL] = { "Arma 1", "Arma 2", "Armadura", "Capacete" };
    float catOffset = 220.0f * escalaUI;
    float catSpacing = 110.0f * escalaUI;
    float catTam = 100.0f * escalaUI;
    float catBaseY = margemTopo;
    float catStartX = largura / 2.0f - catOffset;
    for (int i = 0; i < CAT_TOTAL; ++i) {
        CategoriaEquipamento cat = (CategoriaEquipamento)i;
        Rectangle rect = { catStartX + i * catSpacing, catBaseY, catTam, catTam };
        Color base = (estado->categoriaSelecionada == cat)
            ? (Color){130, 100, 210, 255}
            : (Color){60, 60, 105, 255};
        if (UI_BotaoTexto(rect, categoriaLabels[i], mousePos, mouseClick,
                          base, (Color){220, 220, 250, 255}, fonteBold,
                          estado->categoriaSelecionada == cat)) {
            estado->categoriaSelecionada = cat;
        }
    }

    size_t totalItensCategoria = Equipamento_QuantidadePorCategoria(estado->categoriaSelecionada);
    size_t indiceAtual = (totalItensCategoria > 0) ? estado->indicesSelecionados[estado->categoriaSelecionada] : 0;

    Rectangle painelDir = { largura * 0.55f, altura * 0.28f, largura * 0.35f, altura * 0.42f };
    DrawRectangleRounded(painelDir, 0.1f, 8, (Color){25, 25, 45, 230});

    char desc[512] = {0};
    char status[512] = {0};

    if (totalItensCategoria > 0) {
        if (totalItensCategoria > 1) {
            float setaLarg = 40.0f * escalaUI;
            float setaAlt = 60.0f * escalaUI;
            Rectangle setaEsq = { painelDir.x + 10.0f * escalaUI, painelDir.y + painelDir.height / 2 - setaAlt / 2, setaLarg, setaAlt };
            Rectangle setaDir = { painelDir.x + painelDir.width - setaLarg - 10.0f * escalaUI, painelDir.y + painelDir.height / 2 - setaAlt / 2, setaLarg, setaAlt };
            if (UI_BotaoTexto(setaEsq, "<", mousePos, mouseClick,
                              (Color){80, 80, 120, 255}, (Color){200, 200, 250, 255}, fonteBold, true)) {
                Equipamento_AjustarIndice(estado->categoriaSelecionada, -1, estado->indicesSelecionados);
            }
            if (UI_BotaoTexto(setaDir, ">", mousePos, mouseClick,
                              (Color){80, 80, 120, 255}, (Color){200, 200, 250, 255}, fonteBold, true)) {
                Equipamento_AjustarIndice(estado->categoriaSelecionada, +1, estado->indicesSelecionados);
            }
            indiceAtual = estado->indicesSelecionados[estado->categoriaSelecionada];
        }

        const char *nomeItem = Equipamento_NomeCategoria(estado->categoriaSelecionada, indiceAtual);
        float nomeTam = UI_AjustarTamanhoFonte(30.0f);
        Vector2 nomeMedida = MeasureTextEx(fonteBold, nomeItem, nomeTam, 1.0f);
        DrawTextEx(fonteBold, nomeItem,
                   (Vector2){ painelDir.x + painelDir.width / 2 - nomeMedida.x / 2, painelDir.y + 20.0f * escalaUI },
                   nomeTam, 1.0f, WHITE);

        Equipamento_DescricaoEStatus(estado->categoriaSelecionada, indiceAtual,
                                     desc, sizeof(desc), status, sizeof(status));
    } else {
        DrawTextEx(fonteBold, "Nenhum item cadastrado nesta categoria.",
                   (Vector2){ painelDir.x + 20.0f * escalaUI, painelDir.y + 20.0f * escalaUI },
                   UI_AjustarTamanhoFonte(20.0f), 1.0f, RED);
    }

    Rectangle painelInferior = { largura * 0.1f, altura * 0.78f, largura * 0.8f, altura * 0.18f };
    DrawRectangleRounded(painelInferior, 0.08f, 8, (Color){30, 30, 50, 240});

    float descPadding = 10.0f * escalaUI;
    Rectangle descArea = { painelInferior.x + descPadding, painelInferior.y + descPadding,
                           painelInferior.width - descPadding * 2.0f, painelInferior.height - descPadding * 2.0f };
    DrawTextEx(fonteNormal, desc, (Vector2){ descArea.x, descArea.y },
               UI_AjustarTamanhoFonte(34.0f), 1.0f, LIGHTGRAY);
    if (status[0] != '\0') {
        float inicioY = painelDir.y + painelDir.height * 0.29f;
        float centroX = painelDir.x + painelDir.width / 2;
        const char *ptr = status;
        char linha[128];
        while (*ptr) {
            size_t len = strcspn(ptr, "|");
            memcpy(linha, ptr, len);
            linha[len] = '\0';
            while (len > 0 && linha[len-1] == ' ') linha[--len] = '\0';
            while (*ptr == ' ') ptr++;
            float linhaTam = UI_AjustarTamanhoFonte(24.0f);
            Vector2 medida = MeasureTextEx(fonteBold, linha, linhaTam, 1.0f);
            DrawTextEx(fonteBold, linha, (Vector2){ centroX - medida.x / 2, inicioY }, linhaTam, 1.0f, SKYBLUE);
            inicioY += 30.0f * escalaUI;
            ptr += len;
            if (*ptr == '|') ptr++;
            while (*ptr == ' ') ptr++;
        }
    }

    Rectangle previewArea = { largura * 0.08f, altura * 0.25f, largura * 0.38f, altura * 0.45f };
    DrawRectangleRounded(previewArea, 0.08f, 8, (Color){25, 25, 45, 200});
    Vector2 previewPos = { previewArea.x + previewArea.width / 2,
                           previewArea.y + previewArea.height * 0.6f };

    const Armadura *previewArmadura = (gQuantidadeArmaduras > 0)
        ? &gArmaduras[estado->indicesSelecionados[CAT_ARMADURA]] : NULL;
    const Capacete *previewCapacete = (gQuantidadeCapacetes > 0)
        ? &gCapacetes[estado->indicesSelecionados[CAT_CAPACETE]] : NULL;
    const ArmaPrincipal *previewArma = (gQuantidadeArmasPrincipais > 0)
        ? &gArmasPrincipais[estado->indicesSelecionados[CAT_ARMA_PRINCIPAL]] : NULL;
    const ArmaSecundaria *previewArmaSec = (gQuantidadeArmasSecundarias > 0)
        ? &gArmasSecundarias[estado->indicesSelecionados[CAT_ARMA_SECUNDARIA]] : NULL;
    float escalaPreview = 4.0f * escalaUI;
    if (escalaPreview < 2.5f) escalaPreview = 2.5f;
    if (escalaPreview > 5.0f) escalaPreview = 5.0f;
    UI_DesenharPersonagemPreview(jogadorBase, previewPos,
                                 previewArmadura, previewCapacete,
                                 previewArma, previewArmaSec, escalaPreview);

    return resultado;
}
