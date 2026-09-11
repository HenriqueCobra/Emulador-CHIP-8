#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

#include "chip8.h"

/* Camada de vídeo: dona da janela/renderer/textura SDL.
 * O core NÃO conhece SDL — ele só escreve em c->display[]. */
typedef struct display display_t;

/* Cria janela (64*scale x 32*scale) e recursos de render.
 * Retorna NULL em falha. */
display_t *display_create(const char *title, int scale);

void display_destroy(display_t *d);

/* Copia c->display[] (1 byte/pixel) para a textura e apresenta na tela. */
void display_render(display_t *d, const chip8_t *c);

#endif /* DISPLAY_H */
