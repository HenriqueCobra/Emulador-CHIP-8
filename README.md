# Emulador CHIP-8

Emulador de CHIP-8 escrito em C11 + SDL2, feito do zero para portfólio.

> 🚧 Em construção. O README completo (o que é CHIP-8, arquitetura, decisões de
> implementação, GIFs dos jogos) vem quando o conjunto de opcodes estiver pronto.

## Build

Requer SDL2:

```bash
# Linux (Debian/Ubuntu)
sudo apt install libsdl2-dev
# macOS
brew install sdl2
# Windows (MSYS2 / UCRT64)
pacman -S mingw-w64-ucrt-x86_64-SDL2
```

```bash
make
./build/chip8 roms/games/PONG
```

## Estrutura

| Arquivo         | Responsabilidade                                           |
|-----------------|-----------------------------------------------------------|
| `src/chip8.*`   | Core: memória, registradores, pilha, timers, fetch-decode-execute |
| `src/display.*` | Camada de vídeo (SDL): janela, textura 64×32, render      |
| `src/input.*`   | Mapeamento teclado físico → teclado hex do CHIP-8         |
| `src/main.c`    | Loop principal a 60 Hz (input → CPU → timers → render)    |

## Controles

```
Teclado CHIP-8      Teclado físico
  1 2 3 C             1 2 3 4
  4 5 6 D             Q W E R
  7 8 9 E             A S D F
  A 0 B F             Z X C V
```

`ESC` fecha o emulador.
