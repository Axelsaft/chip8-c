#include "../include/chip8.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

const int width = 64;
const int height = 32;
const int window_scaling = 20;

bool init_sdl(SDL_Window **window, SDL_Renderer **renderer) {
   if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
      return false;
   *window = SDL_CreateWindow("Chip8-Emulator", 
         width*window_scaling, 
         height*window_scaling, 
         0);
   if (*window == NULL) {
      fprintf(stderr, "Failed to create SDL Window");
      return false;
   }
   *renderer = SDL_CreateRenderer(*window, NULL);
   if (*renderer == NULL) {
      fprintf(stderr, "Failed to create SDL Renderer");
      return false;
   }

   return true;
}

bool display_loop(SDL_Window *window, SDL_Renderer *renderer, chip8_t *c, bool *done) {
   SDL_Event event;
   while(SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT)
         *done = true;

		if (event.type == SDL_EVENT_KEY_UP || event.type == SDL_EVENT_KEY_DOWN) {
			int key = keymap(event.key.scancode);
			if (key != -1) {
				uint8_t set = 0;
				if (event.type == SDL_EVENT_KEY_DOWN)
					set = 1;
				chip8_set_key(c, key, set);
			}
		}
   }
   SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
   //SDL_RenderClear(renderer);
   SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
   
   // rendering here
   chip8_render_display(renderer, c, window_scaling);
   SDL_RenderPresent(renderer);
   return true;
}
