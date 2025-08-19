#include "chip8.h"
#include <stdbool.h>
#include <SDL3/SDL.h>

bool init_sdl(SDL_Window **window, SDL_Renderer **renderer);

bool display_loop(SDL_Window *window, SDL_Renderer *renderer, chip8_t *c, bool *done);
