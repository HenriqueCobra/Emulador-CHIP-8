# Emulador CHIP-8 — build
#
# Requer SDL2 instalado:
#   Linux (Debian/Ubuntu): sudo apt install libsdl2-dev
#   macOS (Homebrew):       brew install sdl2
#   Windows (MSYS2/MinGW):  pacman -S mingw-w64-x86_64-SDL2

CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -O2 -g
CFLAGS  += $(shell sdl2-config --cflags 2>/dev/null)
LDFLAGS := $(shell sdl2-config --libs 2>/dev/null)

# Fallback caso sdl2-config não exista (ex.: alguns setups Windows)
ifeq ($(strip $(LDFLAGS)),)
LDFLAGS := -lSDL2
endif

SRC_DIR := src
BUILD   := build
BIN     := $(BUILD)/chip8

SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(BUILD)/%.o,$(SOURCES))

.PHONY: all clean run

all: $(BIN)

$(BIN): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD)/%.o: $(SRC_DIR)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)

# Ex.: make run ROM=roms/games/PONG
run: $(BIN)
	./$(BIN) $(ROM)
