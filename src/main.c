#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "chip8.h"
#include "display.h"
#include "input.h"

/* -----------------------------------------------------------------------------
 * Parâmetros de temporização
 *
 * O CHIP-8 tem DUAS frequências independentes:
 *   1) os timers (delay/sound) decrementam a exatamente 60 Hz;
 *   2) a "CPU" executa instruções numa taxa que a spec original não fixa.
 *      A convenção de fato é ~500-700 Hz. Usamos 540 Hz => 9 instruções por
 *      frame de 60 Hz (540 / 60 = 9), o que dá um número redondo.
 *
 * O loop roda a 60 Hz: a cada frame executa CYCLES_PER_FRAME instruções,
 * dá 1 tick nos timers e (se necessário) redesenha.
 * -------------------------------------------------------------------------- */
#define TARGET_FPS         60
#define CYCLES_PER_FRAME   9
#define FRAME_TIME_MS      (1000.0 / TARGET_FPS)
#define WINDOW_SCALE       12

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "uso: %s <rom>\n", argv[0]);
        return EXIT_FAILURE;
    }

    chip8_t chip8;
    chip8_init(&chip8);

    if (!chip8_load_rom(&chip8, argv[1]))
        return EXIT_FAILURE;

    display_t *display = display_create("Emulador CHIP-8", WINDOW_SCALE);
    if (!display)
        return EXIT_FAILURE;

    bool running = true;
    while (running) {
        uint64_t frame_start = SDL_GetPerformanceCounter();

        /* 1) INPUT — atualiza chip8.keypad[] e detecta pedido de saída */
        running = input_poll(&chip8);

        /* 2) CPU — executa um bloco de instruções para este frame */
        for (int i = 0; i < CYCLES_PER_FRAME; i++)
            chip8_cycle(&chip8);

        /* 3) TIMERS — um único tick de 60 Hz por frame */
        chip8_tick_timers(&chip8);

        /* 4) SOM — sound_timer > 0 significa "tocar bipe" (TODO: áudio) */
        /* if (chip8.sound_timer > 0) beep(); */

        /* 5) VÍDEO — só redesenha se algum DRW/CLS marcou o framebuffer */
        if (chip8.draw_flag) {
            display_render(display, &chip8);
            chip8.draw_flag = false;
        }

        /* 6) THROTTLE — segura o frame em ~16.67 ms (60 FPS) */
        double elapsed_ms =
            (double)(SDL_GetPerformanceCounter() - frame_start) * 1000.0 /
            (double)SDL_GetPerformanceFrequency();
        if (elapsed_ms < FRAME_TIME_MS)
            SDL_Delay((uint32_t)(FRAME_TIME_MS - elapsed_ms));
    }

    display_destroy(display);
    return EXIT_SUCCESS;
}
