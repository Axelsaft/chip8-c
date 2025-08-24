#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>

typedef struct chip8_t{
   uint8_t memory[4096];
   uint16_t i; // Index Register, points to memory addresses
   uint16_t pc; // Program Counter, points at the current instruction in memory
   uint16_t stack[32]; // Specs say 16x 16bit values, but I like to have more
   uint8_t stack_pointer;
   uint8_t v_reg[16]; // Variable Registers, 0-F hex, 0-15, called V0-VF || VF is a flag register
   bool display[32][64]; // Pixel by pixel, value 1 or 0
   uint8_t delay_timer; // Decremented 60 Times a second
   uint8_t sound_timer; // Like delay_timer, beeps if not 0
	uint8_t keypad[16];
	SDL_AudioStream *as;
	uint8_t *beep_data;
	uint32_t beep_len;
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

/*** Keypad ***/

int keymap(SDL_Scancode scancode) {
	switch (scancode) {
		case SDL_SCANCODE_1: return 0x1;
		case SDL_SCANCODE_2: return 0x2;
		case SDL_SCANCODE_3: return 0x3;
		case SDL_SCANCODE_Q: return 0x4;
		case SDL_SCANCODE_W: return 0x5;
		case SDL_SCANCODE_E: return 0x6;
		case SDL_SCANCODE_A: return 0x7;
		case SDL_SCANCODE_S: return 0x8;
		case SDL_SCANCODE_D: return 0x9;
		case SDL_SCANCODE_X: return 0x0;
		case SDL_SCANCODE_Y: return 0xA;
		case SDL_SCANCODE_C: return 0xB;
		case SDL_SCANCODE_4: return 0xC;
		case SDL_SCANCODE_R: return 0xD;
		case SDL_SCANCODE_F: return 0xE;
		case SDL_SCANCODE_V: return 0xF;
		default:
			return -1;
	}
}

void chip8_set_key(chip8_t *c, int key, uint8_t set) {
	c->keypad[key] = set;
	printf("Set key %x to %d\n", key, set);
}

/*** ***/

typedef struct instruction_t {
   uint16_t opcode;
   uint8_t f; // f -> first nibble
   uint8_t x;
   uint8_t y;
   uint8_t n;
   uint8_t nn;
   uint16_t nnn;
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

/*** STACK ***/
void add_to_stack(chip8_t *c, uint16_t val) {
   if (c->stack_pointer+1 > 32) {
      fprintf(stderr, "Stack Overflow!");
      return;
   }
   c->stack[c->stack_pointer] = val;
   c->stack_pointer++;
}

uint16_t pop_from_stack(chip8_t *c) {
   if (c->stack_pointer-1 < 0) {
      fprintf(stderr, "Stack Underflow! Returning stack[0]...\n");
      return c->stack[0];
   }
   c->stack_pointer--;
   return c->stack[c->stack_pointer];
}
/*** ***/

/*** OPCODES ***/
// 00E0
void clear_screen(chip8_t *c) {
   for (int i = 0; i < 32; i++) {
      for (int j = 0; j < 64; j++) {
         c->display[i][j] = false;
      }
   }
}

// 1NNN
void jump(chip8_t *c, uint16_t location) {
   c->pc = location;
}

// 2NNN
void enter_subroutine(chip8_t *c, uint16_t nnn){
   add_to_stack(c, c->pc);
   c->pc = nnn;
}

// 00EE
void exit_subroutine(chip8_t *c) {
   c->pc = pop_from_stack(c);
}

// 3XNN, 4XNN, 5XY0, 9XY0
void condition(chip8_t *c, instruction_t *i) {
   switch (i->f) {
      case 0x3:
         if (c->v_reg[i->x] == i->nn)
            c->pc+=2;
         break;
      case 0x4:
         if (c->v_reg[i->x] != i->nn)
            c->pc+=2;
         break;
      case 0x5:
         if (c->v_reg[i->x] == c->v_reg[i->y])
            c->pc+=2;
         break;
      case 0x9:
         if (c->v_reg[i->x] != c->v_reg[i->y])
            c->pc+=2;
         break;
   }
}

// 6XNN
void set_register(chip8_t *c, uint8_t reg, uint8_t value) {
   c->v_reg[reg] = value;
}

// 7XNN
void add_val_to_register(chip8_t *c, uint8_t reg, uint8_t value) {
   c->v_reg[reg] += value;
}

//8XY0
void set_vx_vy(chip8_t *c, uint8_t x, uint8_t y) {
   c->v_reg[x] = c->v_reg[y];
}

void logical_arithmetic_instruction(chip8_t *c, instruction_t *i) {
   uint8_t bit;
   switch(i->n) {
      case 0x0:
         c->v_reg[i->x] = c->v_reg[i->y];
         break;
      case 0x1:
         c->v_reg[i->x] = c->v_reg[i->x] | c->v_reg[i->y];
         break;
      case 0x2:
         c->v_reg[i->x] = c->v_reg[i->x] & c->v_reg[i->y];
         break;
      case 0x3:
         c->v_reg[i->x] = c->v_reg[i->x] ^ c->v_reg[i->y];
         break;
      case 0x4:
         c->v_reg[i->x] = c->v_reg[i->x] + c->v_reg[i->y];
         break;
      case 0x5:
         c->v_reg[i->x] = c->v_reg[i->x] - c->v_reg[i->y];
         break;
      case 0x6:
         //c->v_reg[i->x] = c->v_reg[i->y]; not wanted in modern chip8
         bit = c->v_reg[i->x] & 0x1;
         c->v_reg[i->x] = c->v_reg[i->x] >> 1;
         c->v_reg[0xF] = bit;
         break;
      case 0x7:
         c->v_reg[i->x] = c->v_reg[i->y] - c->v_reg[i->x];
         break;
      case 0xE:
         bit = c->v_reg[i->x] & 0x80;
         c->v_reg[i->x] = c->v_reg[i->x] << 1;
         c->v_reg[0xF] = bit;
         break;
   }
}

// ANNN
void set_index_register(chip8_t *c, uint16_t value) {
   c->i = value;
}

// BNNN
void jump_with_offset(chip8_t *c, instruction_t *i) {
   c->pc = i->nnn + c->v_reg[0];
}

// CXNN
void random_number(chip8_t *c, instruction_t *i) {
   srand(time(NULL));
   uint8_t rand_num = rand() % UINT8_MAX;
   c->v_reg[i->x] = rand_num & i->nn;
}

// DXYN
void display_draw(chip8_t *c, uint8_t x, uint8_t y, uint8_t n) {
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

// EX9E & EXA1 Skip if key pressed
// todo
void skip_if_key(chip8_t *c, instruction_t *i) {
	uint8_t key = c->v_reg[i->x];
	if (key <= 16) {
		switch (i->nn){
			case (0x9E):
				if (c->keypad[key] == 1)
					c->pc+=2;
				break;
			case (0xA1):
				if (c->keypad[key] != 1)
					c->pc+=2;
				break;
		}
	}
}

// FX07 FX15 FX18
void handle_timers(chip8_t *c, instruction_t *i) {
   switch (i->nn){
      case 0x07:
         c->v_reg[i->x] = c->delay_timer;
         break;
      case 0x15:
         c->delay_timer = c->v_reg[i->x];
         break;
      case 0x18:
         c->sound_timer = c->v_reg[i->x];
         break;
   }
} 

// FX1E:
void add_to_index(chip8_t *c, instruction_t *i){
   c->i += c->v_reg[i->x];
}

// FX0A todo get key
bool get_key(chip8_t *c, instruction_t *i) {
	for (int j = 0; j < 16; j++) {
		uint8_t keypress = c->keypad[j];
		if (keypress == 1) {
			c->v_reg[i->x] = j;
			printf("Key was pressed!");
			return false;
		}
	}
	return true;
}

// FX29 
void font_character(chip8_t *c, instruction_t *i) {
	uint16_t addr = c->v_reg[i->x]*5;
	c->i = addr;
}

//FX33
void decimal_conversion(chip8_t *c, instruction_t *i) {
	uint8_t first, second, third;
	first = c->v_reg[i->x] / 100;
	second = (c->v_reg[i->x] / 10) % 10;
	third = c->v_reg[i->x] % 10;
	c->memory[c->i] = first;
	c->memory[c->i+1] = second;
	c->memory[c->i+2] = third;
}

// FX55 FX65
void store_load_memory(chip8_t *c, instruction_t *i) {
	switch (i->nn) {
		case 0x55:
			for (int j = 0; j < i->x+1; j++) {
				c->memory[c->i+j] = c->v_reg[j];
			}
			break;
		case 0x65:
			for (int j = 0; j < i->x+1; j++) {
				uint8_t value = c->memory[c->i+j];
				c->v_reg[j] = value;
			}
			break;
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

	//printf("PC: %03X | Opcode: %04X\n", c->pc, ((c->memory[c->pc] << 8) | c->memory[c->pc+1]));

   switch ((opcode & 0xF000) >> 12) {
      // Clear Screen
      case 0x0:
         if (x == 0x0 && y == 0xE && instruction->nn != 0xEE){
         clear_screen(c);
         } else if (instruction->nn==0xEE) {
				exit_subroutine(c);
			}
         break;
      // Jump
      case 0x1:
         jump(c, instruction->nnn); // 0x0NNN
         hasJumped = true;
         break;
		case 0x2:
			enter_subroutine(c, instruction->nnn);
			hasJumped = true;
			break;
		case 0x3:
			condition(c, instruction);
			break;
		case 0x4:
			condition(c, instruction);
			break;
		case 0x5:
			condition(c, instruction);
			break;
      // set register vx
      case 0x6:
         set_register(c, (opcode&0x0F00)>>8, (opcode&0x00FF));
         break;
      // add value to register vx
      case 0x7:
         add_val_to_register(c, (opcode&0x0F00)>>8, (opcode&0x00FF));
         break;
		case 0x8:
			logical_arithmetic_instruction(c, instruction);
			break;
		case 0x9:
			condition(c, instruction);
			break;
      // set index register
      case 0xA:
         set_index_register(c, (opcode&0x0FFF));
         break;
		case 0xB:
			jump_with_offset(c, instruction);
			hasJumped = true;
			break;
		case 0xC:
			random_number(c, instruction);
			break;
      // display draw
      case 0xD:
         display_draw(c, instruction->x, instruction->y, instruction->n);
         break;
		case 0xE:
			skip_if_key(c, instruction);
			break;
		case 0xF:
			if (instruction->nn == 0x07 || instruction->nn == 0x15 || instruction->nn == 0x18 )
				handle_timers(c, instruction);
			if (instruction->nn == 0x1E)
				add_to_index(c, instruction);
			if (instruction->nn == 0x0A){
				hasJumped = get_key(c, instruction);}
			if (instruction->nn == 0x29)
				font_character(c, instruction);
			if (instruction->nn == 0x33)
				decimal_conversion(c, instruction);
			if (instruction->nn == 0x55 || instruction->nn == 0x65)
				store_load_memory(c, instruction);
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

	printf("Opening Audio Stream...");
	SDL_AudioSpec spec;
	char *wav_path;
	uint8_t *wav_data = NULL;
	uint32_t wav_len = 0;
	SDL_asprintf(&wav_path, "../sounds/beep.wav"); 
	if (!SDL_LoadWAV(wav_path, &spec, &wav_data, &wav_len)) {
		fprintf(stderr, "Failed to load beep.wav!");
		return 0;
	}
	SDL_free(wav_path);
	(*c)->beep_data = wav_data;
	(*c)->beep_len = wav_len;

	(*c)->as = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
	if (!(*c)->as) {
		fprintf(stderr, "Failed to create Audio Stream!");
		return 0;
	}
	SDL_ResumeAudioStreamDevice((*c)->as);

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
	//printf("Decoded nnn: %X, nn: %X, n: %X, opcode: %X\n", i->nnn, i->nn, i->n, i->opcode);
   bool canIncrement = execute_instruction(chip8, i);
   if (canIncrement)
      chip8->pc += 2;
}

void chip8_render_display(SDL_Renderer *renderer, chip8_t *c, int scale) {
   for (int y = 0; y < 32; y++) {
      for (int x = 0; x < 64; x++) {
         if (c->display[y][x] == true) {
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
         } else {
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			}
			SDL_FRect r = {x * scale, y*scale, scale, scale};
			SDL_RenderFillRect(renderer, &r);
      }
   }
}; 

void chip8_decrease_timers(chip8_t *c) {
	if (c->delay_timer > 0)
		c->delay_timer--;
	if (c->sound_timer > 0)
		c->sound_timer--;
	
	if (c->sound_timer > 0) {
		SDL_PutAudioStreamData(c->as, c->beep_data, c->beep_len);
		SDL_FlushAudioStream(c->as);
	}
}
