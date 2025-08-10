#include <stdbool.h>
#include <SDL3/SDL.h>
#include <stdio.h>

const int width = 64;
const int height = 32;
const int window_scaling = 10;

bool init_sdl(SDL_Window *window, SDL_Renderer *renderer) {
   if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
      return false;
   
   window = SDL_CreateWindow("Chip8-Emulator", 
         width*window_scaling, 
         height*window_scaling, 
         0);
   if (window == NULL) {
      fprintf(stderr, "Failed to create SDL Window");
      return false;
   }

   return true;
}
