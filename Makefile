# jogoAED - Raylib project Makefile (submodule build)
# Usage:
#   make setup      -> init and fetch submodule (first time)
#   make            -> build jogoAED (also builds raylib if needed)
#   make run        -> run the binary
#   make clean      -> remove object files
#   make distclean  -> clean and also remove raylib build artifacts

PROJECT_NAME := jogoAED
SRC_DIR      := src
BUILD_DIR    := build
BIN_DIR      := bin
RAYLIB_DIR   := external/raylib
RAYLIB_SRC   := $(RAYLIB_DIR)/src

SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

# Detect OS
UNAME_S := $(shell uname -s 2>/dev/null || echo Unknown)

# Common flags
CC      ?= gcc
CSTD    ?= c99
CFLAGS  += -Wall -Wextra -std=$(CSTD) -I$(RAYLIB_SRC) -I$(RAYLIB_SRC)/external -Iinclude
LDFLAGS :=

# Platform-specific link flags
ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
    # MSYS2/MinGW on Windows
    LIBS := -L$(RAYLIB_SRC) -lraylib -lopengl32 -lgdi32 -lwinmm -lws2_32
    EXE  := .exe
else ifeq ($(UNAME_S),Linux)
    LIBS := -L$(RAYLIB_SRC) -lraylib -lm -lpthread -ldl -lrt -lX11
    EXE  :=
else ifeq ($(UNAME_S),Darwin)
    # macOS
    LIBS := -L$(RAYLIB_SRC) -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    EXE  :=
else
    $(warning Unknown platform '$(UNAME_S)'; defaulting to Linux-like linking)
    LIBS := -L$(RAYLIB_SRC) -lraylib -lm -lpthread -ldl -lrt -lX11
    EXE  :=
endif

# Default target
all: $(BIN_DIR)/$(PROJECT_NAME)$(EXE)

# Build binary
$(BIN_DIR)/$(PROJECT_NAME)$(EXE): deps $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS)

# Compile objects
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure directories exist
$(BUILD_DIR) $(BIN_DIR):
	mkdir -p $@

# Build raylib from submodule if the static lib is not present
deps: $(RAYLIB_SRC)/libraylib.a

$(RAYLIB_SRC)/libraylib.a:
	@if [ ! -e "$(RAYLIB_DIR)/.git" ]; then \
		echo ">> Raylib submodule not present. Fetching..."; \
		git submodule update --init --recursive || exit 1; \
	fi
	@echo ">> Building raylib..."
	$(MAKE) -C $(RAYLIB_SRC) PLATFORM=PLATFORM_DESKTOP


# Initialize git submodule
setup:
	@git submodule update --init --recursive
	@echo ">> Raylib submodule initialized."

run: all
	@./$(BIN_DIR)/$(PROJECT_NAME)$(EXE)

clean:
	@echo ">> Cleaning objects"
	@rm -rf $(BUILD_DIR)

distclean: clean
	@echo ">> Cleaning raylib build artifacts"
	@$(MAKE) -C $(RAYLIB_SRC) clean || true
	@rm -rf $(BIN_DIR)

.PHONY: all deps setup run clean distclean
