#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>

typedef struct {
   uint8_t memory[4096];
   uint16_t i; // Index Register, points to memory addresses
   uint16_t pc; // Program Counter, points at the current instruction in memory
   uint16_t stack[32]; // Specs say 16x 16bit values, but I like to have more
   uint8_t v_reg[16]; // Variable Registers, 0-F hex, 0-15, called V0-VF || VF is a flag register
   bool display[64][32]; // Pixel by pixel, value 1 or 0
   uint8_t delay_timer; // Decremented 60 Times a second
   uint8_t sound_timer; // Like delay_timer, beeps if not 0
} chip8_t;

const uint8_t font[] = {
   0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
   0x20, 0x60, 0x20, 0x20, 0x70, // 1
   0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
   0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
   0x90, 0x90, 0xF0, 0x10, 0x10, // 4
   0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
   0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
   0xF0, 0x10, 0x20, 0x40, 0x40, // 7
   0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
   0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
   0xF0, 0x90, 0xF0, 0x90, 0x90, // A
   0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
   0xF0, 0x80, 0x80, 0x80, 0xF0, // C
   0xE0, 0x90, 0x90, 0x90, 0xE0, // D
   0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
   0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

typedef struct {
   uint16_t opcode;
   uint8_t f; // f -> first nibble
   uint8_t x;
   uint8_t y;
   uint8_t n;
   uint8_t nn;
   uint8_t nnn;
} instruction_t;

instruction_t * decode_instruction(uint16_t bytes) {
   instruction_t *instruction = calloc(1, sizeof(instruction_t));
   if (instruction == NULL) {
      fprintf(stderr, "decode_instruction | Error allocating instruction_t!");
      return NULL;
   }

   instruction->opcode = bytes;
   instruction->f = (bytes & 0xF000) >> 12;
   instruction->nnn = bytes & 0x0FFF;
   instruction->nn = bytes & 0x00FF;
   instruction->n = bytes & 0x000F;
   instruction->x = (bytes & 0x0F00) >> 8;
   instruction->y = (bytes & 0x00F0) >> 4;
   return instruction;
}

void execute_instruction(instruction_t *instruction) {
   uint16_t opcode = instruction->opcode;
   uint8_t f = instruction->f;
   uint8_t nnn = instruction->nnn;
   uint8_t nn = instruction->nn;
   uint8_t n = instruction->n;
   uint8_t x = instruction->x;
   uint8_t y = instruction->y;

   switch ((opcode & 0xF000) >> 12) {
      // Clear Screen
      case 0x0:
         break;
      // Jump
      case 0x1:
         break;
      // set register vx
      case 0x6:
         break;
      // add value to register vx
      case 0x7:
         break;
      // set index register
      case 0xA:
         break;
      // display draw
      case 0xD:
         break;
      default:
         fprintf(stderr, "execute_instruction | opcode '%dXXX' not recognized!", f); // Don't know how I want to print this...
   }
}

/// Instructions ///
void clear_screen() {

}

void jump() {

}

void set_register() {

}

void add_val_to_register() {

}

void set_index_register() {

}

void display_draw() {

}

void chip8_init() {

}

void chip8_loop(chip8_t *chip8) {
   // update pc -> TODO: Create a function for handling pc update + error handling
   chip8->pc += 2;

   uint16_t pc = chip8->pc;
   uint8_t *memory = chip8->memory;
   uint16_t b_instruction = ((uint16_t)memory[pc] << 8) | (uint16_t)memory[pc+1];
   instruction_t *i = decode_instruction(b_instruction);
   if (i == NULL){ 
      fprintf(stderr, "chip8_loop | Error deconding instruction");
      return;
   }
   execute_instruction(i);
}
