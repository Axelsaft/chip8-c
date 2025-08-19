#pragma once

#include <stdint.h>
#include <SDL3/SDL.h>

typedef struct chip8_t chip8_t;

void chip8_init(chip8_t **, char *);

void chip8_loop(chip8_t *);

void chip8_render_display(SDL_Renderer *renderer, chip8_t *c, int scale);
