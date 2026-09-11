# Emulador CHIP-8 — build
#
# Requer SDL2:
#   Linux (Debian/Ubuntu): sudo apt install libsdl2-dev
#   macOS (Homebrew):       brew install sdl2
#   Windows (MSYS2 UCRT64): pacman -S mingw-w64-ucrt-x86_64-SDL2
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

# Flags de SDL só para os módulos que incluem <SDL2/SDL.h>
SDL_CFLAGS  := $(shell sdl2-config --cflags 2>/dev/null)
SDL_LIBS    := $(shell sdl2-config --libs 2>/dev/null)
ifeq ($(strip $(SDL_LIBS)),)
SDL_LIBS    := -lSDL2
endif

BUILD := build

# chip8.o é o core puro (sem SDL); os demais precisam dos headers de SDL.
CORE_OBJ := $(BUILD)/chip8.o
SDL_OBJ  := $(BUILD)/display.o $(BUILD)/input.o $(BUILD)/main.o

.PHONY: all tests run check clean

all: $(BUILD)/chip8

$(BUILD)/chip8: $(CORE_OBJ) $(SDL_OBJ)
	$(CC) $^ -o $@ $(SDL_LIBS)

$(CORE_OBJ): src/chip8.c src/chip8.h | $(BUILD)
	$(CC) $(CORE_CFLAGS) -c $< -o $@

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CORE_CFLAGS) $(SDL_CFLAGS) -c $< -o $@

# Runner headless: linka só contra o core, sem SDL.
tests: $(BUILD)/run_rom

$(BUILD)/run_rom: tests/run_rom.c $(CORE_OBJ) | $(BUILD)
	$(CC) $(CORE_CFLAGS) $^ -o $@

# Roda o conjunto de ROMs de teste e mostra o framebuffer resultante.
check: $(BUILD)/run_rom
	@for rom in roms/tests/*.ch8; do \
		echo "==== $$rom ===="; \
		./$(BUILD)/run_rom $$rom 1500; \
		echo; \
	done

$(BUILD):
	mkdir -p $(BUILD)

run: $(BUILD)/chip8
	./$(BUILD)/chip8 $(ROM)

clean:
	rm -rf $(BUILD)
