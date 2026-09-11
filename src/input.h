#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

#include "chip8.h"

/* Mapeamento do teclado físico -> teclado hex do CHIP-8.
 *
 * Layout clássico do CHIP-8 (COSMAC VIP):   Mapeado no teclado QWERTY:
 *   1 2 3 C                                   1 2 3 4
 *   4 5 6 D                                   Q W E R
 *   7 8 9 E                                   A S D F
 *   A 0 B F                                   Z X C V
 */

/* Processa a fila de eventos SDL, atualiza c->keypad[].
 * Retorna false quando o usuário pediu para fechar a janela (evento QUIT
 * ou ESC). */
bool input_poll(chip8_t *c);

#endif /* INPUT_H */
