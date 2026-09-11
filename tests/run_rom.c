/* -----------------------------------------------------------------------------
 * Runner headless de validação.
 *
 * Carrega uma ROM, executa um número fixo de ciclos e imprime o framebuffer
 * como ASCII no stdout. Não depende de SDL: linka só contra o core (chip8.o).
 *
 * As ROMs de teste públicas (corax89, Timendus chip8-test-suite) "desenham"
 * o resultado na tela — então rodar aqui e olhar o ASCII já valida os 35
 * opcodes. Também serve para rodar em CI.
 *
 *   build/run_rom roms/tests/2-ibm-logo.ch8 500
 * -------------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>

#include "../src/chip8.h"

/* Proporção CPU/timer do loop real: 540 Hz / 60 Hz = 9 ciclos por tick. */
#define CYCLES_PER_TICK 9

static void dump_screen(const chip8_t *c)
{
    putchar('+');
    for (int i = 0; i < CHIP8_DISPLAY_W; i++) putchar('-');
    puts("+");

    for (int y = 0; y < CHIP8_DISPLAY_H; y++) {
        putchar('|');
        for (int x = 0; x < CHIP8_DISPLAY_W; x++)
            putchar(c->display[y * CHIP8_DISPLAY_W + x] ? '#' : ' ');
        puts("|");
    }

    putchar('+');
    for (int i = 0; i < CHIP8_DISPLAY_W; i++) putchar('-');
    puts("+");
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "uso: %s <rom> [ciclos]\n", argv[0]);
        return EXIT_FAILURE;
    }

    long cycles = (argc >= 3) ? strtol(argv[2], NULL, 10) : 2000;

    chip8_t chip8;
    chip8_init(&chip8);

    if (!chip8_load_rom(&chip8, argv[1]))
        return EXIT_FAILURE;

    for (long i = 0; i < cycles; i++) {
        chip8_cycle(&chip8);
        if (i % CYCLES_PER_TICK == CYCLES_PER_TICK - 1)
            chip8_tick_timers(&chip8);
    }

    printf("ROM: %s   (%ld ciclos)\n", argv[1], cycles);
    printf("I=0x%03X  pc=0x%03X  sp=%u  DT=%u  ST=%u\n",
           chip8.I, chip8.pc, chip8.sp, chip8.delay_timer, chip8.sound_timer);
    dump_screen(&chip8);

    /* Se um 3º argumento for dado, grava o framebuffer cru (2048 bytes,
     * 1 por pixel) nesse arquivo — usado para gerar PNG fora do C. */
    if (argc >= 4) {
        FILE *out = fopen(argv[3], "wb");
        if (out) {
            fwrite(chip8.display, 1, sizeof(chip8.display), out);
            fclose(out);
        }
    }
    return EXIT_SUCCESS;
}
