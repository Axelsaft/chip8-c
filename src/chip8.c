#include <SDL3/SDL_rect.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>

typedef struct chip8_t{
   uint8_t memory[4096];
   uint16_t i; // Index Register, points to memory addresses
   uint16_t pc; // Program Counter, points at the current instruction in memory
   uint16_t stack[32]; // Specs say 16x 16bit values, but I like to have more
   uint8_t v_reg[16]; // Variable Registers, 0-F hex, 0-15, called V0-VF || VF is a flag register
   bool display[32][64]; // Pixel by pixel, value 1 or 0
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

typedef struct instruction_t {
   uint16_t opcode;
   uint8_t f; // f -> first nibblechi
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

   instruction->opcode = bytes; instruction->f = (bytes & 0xF000) >> 12;
   instruction->nnn = bytes & 0x0FFF;
   instruction->nn = bytes & 0x00FF;
   instruction->n = bytes & 0x000F;
   instruction->x = (bytes & 0x0F00) >> 8;
   instruction->y = (bytes & 0x00F0) >> 4;
   return instruction;
}

/// Instructions ///
void clear_screen(chip8_t *c) {
   for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 64; j++) {
         c->display[i][j] = false;
      }
   }
}


void jump(chip8_t *c, uint16_t location) {
   c->pc = location;
}

void set_register(chip8_t *c, uint8_t reg, uint8_t value) {
   c->v_reg[reg] = value;
}

void add_val_to_register(chip8_t *c, uint8_t reg, uint8_t value) {
   c->v_reg[reg] += value;
}

void set_index_register(chip8_t *c, uint16_t value) {
   c->i = value;
}

void display_draw(chip8_t *c, uint8_t x, uint8_t y, uint8_t n) {
   printf("Called display_draw\n");
   c->v_reg[15] = 0; //VF = 16 -> [15]
   if (x > 63 || y > 31) {
      fprintf(stderr, "Failed to draw screen! X or Y too high! x: %d, y: %d", x,y);
      return;
   }
   if (n <= 0) {
      fprintf(stderr, "Can't draw screen! N is 0!");
      return;
   }
   for (int j = 0; j < n; j++){  // n
      for (int k = 0; k < 8; k++) { // byte
         uint8_t byte = c->memory[c->i+j];
         uint8_t isSet = (byte>>(7-k))&0x1;
         uint8_t x_cord = (c->v_reg[x]+k) % 64;
         uint8_t y_cord = (c->v_reg[y]+j) % 32;
         if (isSet) {
            if (c->display[y_cord][x_cord] == true) {
               c->v_reg[15] = 1;
            }
         c->display[y_cord][x_cord] = !c->display[y_cord][x_cord];
         }
      }
   }
}

bool execute_instruction(chip8_t *c, instruction_t *instruction) {
   bool hasJumped = false;

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
         if (x == 0x0 && y == 0xE){
         printf("Calling clear_screen\n");
         clear_screen(c);
         }
         break;
      // Jump
      case 0x1:
         //printf("Calling jump\n");
         jump(c, (opcode&0x0FFF)); // 0x0NNN
         hasJumped = true;
         break;
      // set register vx
      case 0x6:
         printf("Calling set_register\n");
         set_register(c, (opcode&0x0F00)>>8, (opcode&0x00FF));
         break;
      // add value to register vx
      case 0x7:
         printf("Calling add_val_to_register\n");
         add_val_to_register(c, (opcode&0x0F00)>>8, (opcode&0x00FF));
         break;
      // set index register
      case 0xA:
         printf("Calling set_index_register\n");
         set_index_register(c, (opcode&0x0FFF));
         break;
      // display draw
      case 0xD:
         printf("Calling display_draw\n");
         display_draw(c, instruction->x, instruction->y, instruction->n);
         break;
      default:
         fprintf(stderr, "execute_instruction | opcode '%dXXX' not recognized!\n", f); // Don't know how I want to print this...
   }
   return !hasJumped;
}


int chip8_init(chip8_t **c, char * rom_path) {
   *c = calloc(1, sizeof(chip8_t));
   if (!c) return 0;

   memcpy((*c)->memory, font, sizeof(font));

   FILE *f;
   f = fopen(rom_path, "rb");;
   if (!f) {
      fprintf(stderr, "Failed to open file!\n");
      return 0;
   }
   printf("opened file: %s\n", rom_path);

   // Setting starting Address -> see documentation
   (*c)->pc = 0x200;

   fseek(f, 0, SEEK_END);
   long size = ftell(f);
   rewind(f);
   printf("ROM Size: %ld\n", size);

   unsigned long read = fread(&(*c)->memory[0x200], 1, size, f);
   if (read != size) {
      fprintf(stderr, "Error while reading file. File size does not match read size!\n");
      fclose(f);
      return 0;
   }
   fclose(f);

   return 1;
}

void chip8_loop(chip8_t *chip8) {
   // update pc -> TODO: Create a function for handling pc update + error handling
   uint16_t pc = chip8->pc;
   if (pc > 4095) {
      fprintf(stderr,"Memory PC overflow!\n");
      return;
   }
   uint8_t *memory = chip8->memory;
   uint16_t b_instruction = ((uint16_t)memory[pc] << 8) | (uint16_t)memory[pc+1];
   instruction_t *i = decode_instruction(b_instruction);
   if (i == NULL){ 
      fprintf(stderr, "chip8_loop | Error deconding instruction\n");
      return;
   }
   bool canIncrement = execute_instruction(chip8, i);
   if (canIncrement)
      chip8->pc += 2;
}

void chip8_render_display(SDL_Renderer *renderer, chip8_t *c, int scale) {
   for (int y = 0; y < 32; y++) {
      for (int x = 0; x < 64; x++) {
         if (c->display[y][x] == true) {
            SDL_FRect r = {x * scale, y*scale, scale, scale};
            SDL_RenderFillRect(renderer, &r);
         }
      }
   }
}; 
