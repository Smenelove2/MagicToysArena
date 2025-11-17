# Magic Toys Arena🧸⚔️

Projeto 2D feito em **C + Raylib** onde você seleciona equipamentos (capacete, armadura, armas principal/secundária) antes de entrar no combate contra ondas de monstros.

## ⚙️ Configuração e build
Pré-requisitos gerais:
* Git e Make
* Compilador C (GCC/Clang ou MinGW)
* Não é necessário instalar Raylib manualmente – o repositório já contém o código via submódulo.

### Windows (MSYS2/MinGW64 recomendado)
1. Instale o MSYS2: https://www.msys2.org/
2. Abra o terminal **MSYS2 MinGW64** e rode:
   ```bash
   pacman -Syu
   pacman -S mingw-w64-x86_64-gcc make git
   ```
3. Clone e compile:
   ```bash
   git clone --recurse-submodules https://github.com/SEU_USUARIO/jogoAED.git
   cd jogoAED
   make setup    # inicializa o submódulo da raylib (apenas na primeira vez)
   make          # compila raylib e o jogo
   make run      # executa
   ```

### Linux (Debian/Ubuntu ou derivados)
```bash
sudo apt update && sudo apt install build-essential git make
git clone --recurse-submodules https://github.com/SEU_USUARIO/jogoAED.git
cd jogoAED
make setup
make
make run
```

### macOS (Homebrew)
```bash
xcode-select --install
brew install git make
git clone --recurse-submodules https://github.com/SEU_USUARIO/jogoAED.git
cd jogoAED
make setup
make
make run
```

> Clonou sem `--recurse-submodules`? Rode:
> ```bash
> git submodule update --init --recursive
> ```

## 📝 Sobre o jogo
* **Seleção pré-jogo**: escolha 1 capacete, 1 armadura, arma principal e secundária. Cada item altera vida máxima, regeneração, dano, velocidade e comportamento dos ataques.
* **Loop gameplay**: explore o mapa isolado, use o mouse esquerdo/direito para disparar armas principal/secundária, enfrente monstros com IA simples e veja efeitos visuais a cada golpe.
* **Monstros**: variedade com velocidades, projéteis e pontuações diferentes; derrotá-los soma pontos que aparecem em um placar global.
* **HUD**: barra de vida, cooldown das habilidades (Mouse ESQ/DIR), pontuação e instrução para pausar.
* **Leaderboard**: ao morrer informe seu nome; a pontuação fica salva em `pontuacoes.txt` e pode ser consultada no menu via um painel ordenado (merge sort -> top 10).
* **Pausa (ESC)**: abre mini menu para retomar ou voltar ao menu principal.

### Controles
* **WASD** ou **setas** – movimentação.
* **Mouse esquerdo** – arma principal (varia entre melee/cone/linha/projéteis).
* **Mouse direito** – arma secundária (defesa, área contínua, cone de empurrão etc.).
* **ESC** – pause.

## 📂 Estrutura principal
```
jogoAED/
├─ src/            # Códigos-fonte (app, jogo, menu, itens, monstros…)
├─ include/        # Headers
├─ assets/         # Sprites, fontes e mapas
├─ external/
│  └─ raylib/      # Submódulo Raylib
├─ Makefile
└─ README.md
```

## 🔧 Comandos úteis
* `make` – compila o projeto completo.
* `make run` – executa `bin/jogoAED`.
* `make clean` – apaga objetos.
* `make distclean` – `clean` + remove `bin/` e artefatos Raylib.
* `make setup` – clona/atualiza o submódulo Raylib (quando necessário).
