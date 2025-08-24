#include "../include/chip8.h"
#include "../include/display.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
   SDL_Renderer *renderer;
   SDL_Window *window;
   if(!init_sdl(&window, &renderer))
      return -1;

   chip8_t *c;
	//chip8_init(&c, "../corax.ch8");
	//chip8_init(&c, "../ibm.ch8");
   //chip8_init(&c, "../test_opcode.ch8");
	//chip8_init(&c, "../spaceinvaders.ch8");
	//chip8_init(&c, "../Pong (alt).ch8");
	//chip8_init(&c, "../flightrunner.ch8");
	chip8_init(&c, "../snake.ch8");
   if (!c) {
      fprintf(stderr, "Failed to create chip8_t struct!");
      return -1;
   }

   bool done = 0;
   uint32_t frameStart;
   uint32_t frameTime;
   while (!done) {
      frameStart = SDL_GetTicks();
		chip8_decrease_timers(c);
      for (int i = 0; i < 10; i++) {
         chip8_loop(c);
      }
      display_loop(window, renderer, c, &done);
      frameTime = SDL_GetTicks() - frameStart;
      if ((1000 / 60 /*fps*/) > frameTime) {
         SDL_Delay((1000/60)-frameTime);
      }

   }
   
   return 0;
}
