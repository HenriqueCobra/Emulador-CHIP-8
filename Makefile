# Emulador CHIP-8 — build
#
# Requer SDL2:
#   Linux (Debian/Ubuntu): sudo apt install libsdl2-dev
#   macOS (Homebrew):       brew install sdl2
#   Windows (MSYS2 UCRT64): pacman -S mingw-w64-ucrt-x86_64-SDL2
#
# No Windows: "make", "make run" e "make tests" funcionam de qualquer
# terminal (PowerShell, cmd ou MSYS2) — o SDL2 é localizado direto pelo
# prefixo de instalação do MSYS2, sem depender de sdl2-config estar no
# PATH nem de sintaxe de shell POSIX. Já "make clean" e "make check" usam
# rm/for e precisam de um shell POSIX (Git Bash, ou o terminal "MSYS2
# UCRT64" em C:\msys64\ucrt64.exe).
#
# Alvos:
#   make            -> build/chip8      (emulador com SDL)
#   make tests      -> build/run_rom    (runner headless, sem SDL)
#   make run ROM=roms/games/PONG.ch8
#   make check      -> roda as ROMs de teste no runner headless
#   make clean

CC          := gcc
WARN        := -Wall -Wextra -Wpedantic
CORE_CFLAGS := -std=c11 $(WARN) -O2 -g

ifeq ($(OS),Windows_NT)
  EXE := .exe
else
  EXE :=
endif

# --- Localização do SDL2 ----------------------------------------------------
# sdl2-config só está no PATH dentro de um shell MSYS2/UCRT64; chamando
# "make" de um PowerShell/cmd puro ele não é achado, e o "$(shell ... 2>
# /dev/null)" clássico falha porque cmd.exe não entende essa redireção.
# Por isso localizamos o SDL2 direto pelo prefixo de instalação (via
# $(wildcard), que é resolvido pelo próprio make, sem invocar um shell) e
# só caímos para sdl2-config em Linux/macOS.
MSYS2_PREFIX := $(firstword $(wildcard C:/msys64/ucrt64) $(wildcard C:/msys64/mingw64))

ifneq ($(strip $(MSYS2_PREFIX)),)
  SDL_CFLAGS := -I$(MSYS2_PREFIX)/include/SDL2 -Dmain=SDL_main
  SDL_LIBS   := -L$(MSYS2_PREFIX)/lib -lmingw32 -lSDL2main -lSDL2 -mwindows
else
  SDL_CFLAGS := $(shell sdl2-config --cflags 2>/dev/null)
  SDL_LIBS   := $(shell sdl2-config --libs 2>/dev/null)
  ifeq ($(strip $(SDL_LIBS)),)
    SDL_LIBS := -lSDL2
  endif
endif

BUILD    := build
BIN      := $(BUILD)/chip8$(EXE)
TEST_BIN := $(BUILD)/run_rom$(EXE)

# chip8.o é o core puro (sem SDL); os demais precisam dos headers de SDL.
CORE_OBJ := $(BUILD)/chip8.o
SDL_OBJ  := $(BUILD)/display.o $(BUILD)/input.o $(BUILD)/main.o

.PHONY: all tests run check clean

all: $(BIN)

$(BIN): $(CORE_OBJ) $(SDL_OBJ)
	$(CC) $^ -o $@ $(SDL_LIBS)

$(CORE_OBJ): src/chip8.c src/chip8.h | $(BUILD)
	$(CC) $(CORE_CFLAGS) -c $< -o $@

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CORE_CFLAGS) $(SDL_CFLAGS) -c $< -o $@

# Runner headless: linka só contra o core, sem SDL.
tests: $(TEST_BIN)

$(TEST_BIN): tests/run_rom.c $(CORE_OBJ) | $(BUILD)
	$(CC) $(CORE_CFLAGS) $^ -o $@

# Roda o conjunto de ROMs de teste e mostra o framebuffer resultante.
# Precisa de shell POSIX (for/echo) — use Git Bash ou o terminal MSYS2 UCRT64.
check: $(TEST_BIN)
	@for rom in roms/tests/*.ch8; do \
		echo "==== $$rom ===="; \
		./$(TEST_BIN) $$rom 1500; \
		echo; \
	done

$(BUILD):
	mkdir $(BUILD)

run: $(BIN)
	./$(BIN) $(ROM)

# Precisa de shell POSIX (rm). No PowerShell/cmd, apague build\ manualmente.
clean:
	rm -rf $(BUILD)
