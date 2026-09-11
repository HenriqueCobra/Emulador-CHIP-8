#include "chip8.h"

#include <stdio.h>
#include <string.h>

/* ---------------------------------------------------------------------------
 * Fontset embutido: 16 caracteres hexadecimais (0-F), 5 bytes cada.
 * Cada byte é uma linha do sprite 4x5; só os 4 bits altos são desenhados.
 *
 * Ex.: o "1"
 *   0x20 -> 0010 0000   ->   . X . .
 *   0x60 -> 0110 0000   ->   X X . .
 *   0x20 -> 0010 0000   ->   . X . .
 *   0x20 -> 0010 0000   ->   . X . .
 *   0x70 -> 0111 0000   ->   X X X .
 * ------------------------------------------------------------------------- */
static const uint8_t CHIP8_FONTSET[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
    0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
    0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
    0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
    0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */
    0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
    0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */
    0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
    0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */
    0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */
    0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */
    0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
    0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */
    0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */
    0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */
    0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */
};

/* ---------------------------------------------------------------------------
 * chip8_init
 * ------------------------------------------------------------------------- */
void chip8_init(chip8_t *c)
{
    memset(c, 0, sizeof(*c));
    c->pc = CHIP8_PROG_START;

    /* copia o fontset para a região reservada da RAM */
    memcpy(&c->memory[CHIP8_FONT_START], CHIP8_FONTSET, sizeof(CHIP8_FONTSET));
}

/* ---------------------------------------------------------------------------
 * chip8_load_rom
 * ------------------------------------------------------------------------- */
bool chip8_load_rom(chip8_t *c, const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "erro: não consegui abrir a ROM '%s'\n", path);
        return false;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    long max = CHIP8_MEM_SIZE - CHIP8_PROG_START;
    if (size <= 0 || size > max) {
        fprintf(stderr, "erro: ROM inválida ou grande demais (%ld bytes, max %ld)\n",
                size, max);
        fclose(f);
        return false;
    }

    size_t read = fread(&c->memory[CHIP8_PROG_START], 1, (size_t)size, f);
    fclose(f);

    if (read != (size_t)size) {
        fprintf(stderr, "erro: leitura incompleta da ROM\n");
        return false;
    }
    return true;
}

/* ---------------------------------------------------------------------------
 * chip8_tick_timers
 * ------------------------------------------------------------------------- */
void chip8_tick_timers(chip8_t *c)
{
    if (c->delay_timer > 0) c->delay_timer--;
    if (c->sound_timer > 0) c->sound_timer--;
}

/* ---------------------------------------------------------------------------
 * chip8_cycle — fetch / decode / execute
 * ------------------------------------------------------------------------- */
void chip8_cycle(chip8_t *c)
{
    /* -------- FETCH ------------------------------------------------------
     * Toda instrução tem 16 bits (big-endian): byte alto no endereço menor.
     * pc SEMPRE aponta para uma instrução; avançamos de 2 em 2. */
    c->opcode = (uint16_t)(c->memory[c->pc] << 8 | c->memory[c->pc + 1]);
    c->pc += 2;

    /* -------- DECODE ---------------------------------------------------
     * Campos padrão de um opcode CHIP-8 (nibble = 4 bits):
     *
     *   opcode = 0xWXYZ
     *            │ │ │ └─ N   : nibble baixo                (0x000F)
     *            │ │ └─── Y   : nibble do 2º byte, bits 4-7 (0x00F0) -> reg Vy
     *            │ └───── X   : nibble do 1º byte, bits 0-3 (0x0F00) -> reg Vx
     *            └─────── prefixo da família de instrução   (0xF000)
     *
     *   NN  : byte baixo inteiro          (0x00FF) — constante de 8 bits
     *   NNN : 12 bits baixos              (0x0FFF) — endereço
     *
     * A CPU decide a instrução pelo nibble alto; famílias 0x0/0x8/0xE/0xF
     * precisam de um segundo nível de switch. */
    uint16_t nnn = c->opcode & 0x0FFF;             /* endereço (12 bits)      */
    uint8_t  nn  = (uint8_t)(c->opcode & 0x00FF);  /* constante de 8 bits     */
    uint8_t  n   = (uint8_t)(c->opcode & 0x000F);  /* nibble baixo            */
    uint8_t  x   = (uint8_t)((c->opcode & 0x0F00) >> 8); /* índice de reg.    */
    uint8_t  y   = (uint8_t)((c->opcode & 0x00F0) >> 4); /* índice de reg.    */

    /* silencia -Wunused enquanto os opcodes restantes não estão implementados */
    (void)n;

    switch (c->opcode & 0xF000) {

    case 0x0000:
        switch (c->opcode & 0x00FF) {
        case 0x00E0: /* CLS  — limpa a tela                        */ /* TODO */ break;

        case 0x00EE: /* RET — retorna de sub-rotina: desempilha o
                      * endereço de retorno para o pc. */
            if (c->sp == 0) {
                fprintf(stderr, "stack underflow em RET (pc=0x%03X)\n",
                        c->pc - 2);
                break;
            }
            c->pc = c->stack[--c->sp];
            break;

        default:     /* 0NNN — SYS addr (ignorado em emuladores)   */ break;
        }
        break;

    case 0x1000: /* 1NNN — JP addr: salto incondicional.
                  * pc já foi incrementado no fetch; aqui simplesmente
                  * sobrescrevemos com o alvo de 12 bits. */
        c->pc = nnn;
        break;

    case 0x2000: /* 2NNN — CALL addr: chama sub-rotina.
                  * Empilha o pc atual (que o fetch já avançou para a
                  * instrução seguinte) e desvia para o alvo. */
        if (c->sp >= CHIP8_STACK_SIZE) {
            fprintf(stderr, "stack overflow em CALL (pc=0x%03X)\n", c->pc - 2);
            break;
        }
        c->stack[c->sp++] = c->pc;
        c->pc = nnn;
        break;
    case 0x3000: /* 3XNN — SE Vx, NN: pula a próxima instrução se Vx == NN.
                  * "Pular" = pc += 2 (o fetch já avançou uma instrução). */
        if (c->V[x] == nn) c->pc += 2;
        break;

    case 0x4000: /* 4XNN — SNE Vx, NN: pula se Vx != NN */
        if (c->V[x] != nn) c->pc += 2;
        break;

    case 0x5000: /* 5XY0 — SE Vx, Vy: pula se Vx == Vy.
                  * O padrão só define esta forma (nibble baixo = 0). */
        if (c->V[x] == c->V[y]) c->pc += 2;
        break;

    case 0x6000: /* 6XNN — LD Vx, NN: carrega a constante de 8 bits em Vx.
                  * Não há flag nem overflow: NN já cabe em uint8_t. */
        c->V[x] = nn;
        break;

    case 0x7000: /* 7XNN — ADD Vx, NN: Vx += NN.
                  * ATENÇÃO: não altera VF, mesmo estourando 255.
                  * O wrap mod 256 é a própria semântica de uint8_t em C;
                  * o cast só torna a intenção explícita. */
        c->V[x] = (uint8_t)(c->V[x] + nn);
        break;

    case 0x8000:
        switch (c->opcode & 0x000F) {
        case 0x0: /* 8XY0 — LD Vx, Vy: cópia registrador→registrador */
            c->V[x] = c->V[y];
            break;
        case 0x1: /* 8XY1 — OR Vx, Vy         */ /* TODO */ break;
        case 0x2: /* 8XY2 — AND Vx, Vy        */ /* TODO */ break;
        case 0x3: /* 8XY3 — XOR Vx, Vy        */ /* TODO */ break;
        case 0x4: /* 8XY4 — ADD Vx, Vy (carry)*/ /* TODO */ break;
        case 0x5: /* 8XY5 — SUB Vx, Vy (borrow)*/ /* TODO */ break;
        case 0x6: /* 8XY6 — SHR Vx {, Vy}     */ /* TODO */ break;
        case 0x7: /* 8XY7 — SUBN Vx, Vy       */ /* TODO */ break;
        case 0xE: /* 8XYE — SHL Vx {, Vy}     */ /* TODO */ break;
        default:  /* opcode inválido */ break;
        }
        break;

    case 0x9000: /* 9XY0 — SNE Vx, Vy: pula se Vx != Vy */
        if (c->V[x] != c->V[y]) c->pc += 2;
        break;

    case 0xA000: /* ANNN — LD I, addr: registrador de índice recebe
                  * um endereço de 12 bits. I é a base de quase todo
                  * acesso à memória (sprites, BCD, load/store). */
        c->I = nnn;
        break;

    case 0xB000: /* BNNN — JP V0, addr        */ /* TODO */ break;
    case 0xC000: /* CXNN — RND Vx, NN         */ /* TODO */ break;
    case 0xD000: /* DXYN — DRW Vx, Vy, N      */ /* TODO */ break;

    case 0xE000:
        switch (c->opcode & 0x00FF) {
        case 0x9E: /* EX9E — SKP Vx  (tecla Vx pressionada) */ /* TODO */ break;
        case 0xA1: /* EXA1 — SKNP Vx (tecla Vx solta)       */ /* TODO */ break;
        default:   break;
        }
        break;

    case 0xF000:
        switch (c->opcode & 0x00FF) {
        case 0x07: /* FX07 — LD Vx, DT         */ /* TODO */ break;
        case 0x0A: /* FX0A — LD Vx, K (espera tecla) */ /* TODO */ break;
        case 0x15: /* FX15 — LD DT, Vx         */ /* TODO */ break;
        case 0x18: /* FX18 — LD ST, Vx         */ /* TODO */ break;
        case 0x1E: /* FX1E — ADD I, Vx         */ /* TODO */ break;
        case 0x29: /* FX29 — LD F, Vx (sprite do dígito) */ /* TODO */ break;
        case 0x33: /* FX33 — LD B, Vx (BCD)    */ /* TODO */ break;
        case 0x55: /* FX55 — LD [I], V0..Vx    */ /* TODO */ break;
        case 0x65: /* FX65 — LD V0..Vx, [I]    */ /* TODO */ break;
        default:   break;
        }
        break;

    default:
        fprintf(stderr, "opcode desconhecido: 0x%04X (pc=0x%03X)\n",
                c->opcode, c->pc - 2);
        break;
    }
}
