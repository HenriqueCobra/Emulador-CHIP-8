#include "display.h"

#include <SDL2/SDL.h>
#include <stdlib.h>

/* Cores do "fósforo": pixel apagado / pixel aceso (ARGB8888). */
#define COLOR_OFF 0x000000FFu
#define COLOR_ON  0xFFFFFFFFu

struct display {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    SDL_Texture  *texture;   /* 64x32, streaming, ARGB8888 */
    uint32_t      pixels[CHIP8_DISPLAY_SIZE];
};

display_t *display_create(const char *title, int scale)
{
    if (scale < 1) scale = 10;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init falhou: %s", SDL_GetError());
        return NULL;
    }

    display_t *d = calloc(1, sizeof(*d));
    if (!d) return NULL;

    d->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        CHIP8_DISPLAY_W * scale, CHIP8_DISPLAY_H * scale,
        SDL_WINDOW_SHOWN);

    d->renderer = SDL_CreateRenderer(d->window, -1, SDL_RENDERER_ACCELERATED);

    d->texture = SDL_CreateTexture(
        d->renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        CHIP8_DISPLAY_W, CHIP8_DISPLAY_H);

    if (!d->window || !d->renderer || !d->texture) {
        SDL_Log("erro criando recursos SDL: %s", SDL_GetError());
        display_destroy(d);
        return NULL;
    }
    return d;
}

void display_destroy(display_t *d)
{
    if (!d) return;
    if (d->texture)  SDL_DestroyTexture(d->texture);
    if (d->renderer) SDL_DestroyRenderer(d->renderer);
    if (d->window)   SDL_DestroyWindow(d->window);
    free(d);
    SDL_Quit();
}

void display_render(display_t *d, const chip8_t *c)
{
    for (int i = 0; i < CHIP8_DISPLAY_SIZE; i++)
        d->pixels[i] = c->display[i] ? COLOR_ON : COLOR_OFF;

    SDL_UpdateTexture(d->texture, NULL, d->pixels,
                      CHIP8_DISPLAY_W * (int)sizeof(uint32_t));
    SDL_RenderClear(d->renderer);
    SDL_RenderCopy(d->renderer, d->texture, NULL, NULL);
    SDL_RenderPresent(d->renderer);
}
