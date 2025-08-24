#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_keycode.h>
#include <stdint.h>

typedef struct chip8_t chip8_t;

int keymap(SDL_Keycode);

void chip8_init(chip8_t **, char *);

void chip8_loop(chip8_t *);

void chip8_render_display(SDL_Renderer *renderer, chip8_t *c, int scale);

void chip8_decrease_timers(chip8_t *);

void chip8_set_key(chip8_t *, int, uint8_t);
