#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <stdint.h>

/* ---------------------------------------------------------------------------
 * Constantes da arquitetura CHIP-8
 * ------------------------------------------------------------------------- */
#define CHIP8_MEM_SIZE      4096   /* 0x000 .. 0xFFF                          */
#define CHIP8_PROG_START    0x200  /* endereço onde a ROM é carregada        */
#define CHIP8_FONT_START    0x050  /* onde o fontset embutido fica na memória */
#define CHIP8_NUM_REGS      16     /* V0 .. VF                               */
#define CHIP8_STACK_SIZE    16     /* 16 níveis de sub-rotina                */
#define CHIP8_NUM_KEYS      16     /* teclado hexadecimal 0 .. F             */
#define CHIP8_DISPLAY_W     64
#define CHIP8_DISPLAY_H     32
#define CHIP8_DISPLAY_SIZE  (CHIP8_DISPLAY_W * CHIP8_DISPLAY_H)

/* ---------------------------------------------------------------------------
 * Estado completo da máquina virtual.
 * Um único struct = toda a "CPU + RAM + periféricos" do CHIP-8.
 * ------------------------------------------------------------------------- */
typedef struct {
    uint8_t  memory[CHIP8_MEM_SIZE];      /* RAM de 4KB                        */
    uint8_t  V[CHIP8_NUM_REGS];           /* registradores V0..VF (VF = flag)  */
    uint16_t I;                           /* registrador de índice (12 bits úteis) */
    uint16_t pc;                          /* program counter                   */

    uint16_t stack[CHIP8_STACK_SIZE];     /* pilha de endereços de retorno     */
    uint8_t  sp;                          /* stack pointer (próximo slot livre) */

    uint8_t  delay_timer;                 /* decrementa a 60Hz                 */
    uint8_t  sound_timer;                 /* decrementa a 60Hz; >0 => bipe     */

    uint8_t  keypad[CHIP8_NUM_KEYS];      /* 1 = pressionada, 0 = solta        */
    uint8_t  display[CHIP8_DISPLAY_SIZE]; /* 1 byte por pixel (0/1) p/ simplicidade */

    bool     draw_flag;                   /* true quando o framebuffer mudou   */
    uint16_t opcode;                      /* instrução atual (guardada p/ decode/debug) */
} chip8_t;

/* ---------------------------------------------------------------------------
 * API pública do core
 * ------------------------------------------------------------------------- */

/* Zera o estado, carrega o fontset. Chamar antes de tudo. */
void chip8_init(chip8_t *c);

/* Carrega a ROM do disco para memory[0x200..].
 * Retorna true em sucesso, false se o arquivo não abrir ou não couber. */
bool chip8_load_rom(chip8_t *c, const char *path);

/* Executa exatamente UM ciclo: fetch -> decode -> execute.
 * NÃO mexe nos timers (isso é responsabilidade do loop a 60Hz). */
void chip8_cycle(chip8_t *c);

/* Decrementa delay_timer e sound_timer (se > 0). Chamar 60x por segundo. */
void chip8_tick_timers(chip8_t *c);

#endif /* CHIP8_H */
