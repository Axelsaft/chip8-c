#include <stdint.h>
#include <stdbool.h>

typedef struct {
   uint8_t memory[4096];
   uint16_t i; // Index Register, points to memory addresses
   uint16_t pc; // Program Counter, points at the current instruction in memory
   uint16_t stack[32]; // Specs say 16x 16bit values, but I like to have more
   uint8_t v_reg[16]; // Variable Registers, 0-F hex, 0-15, called V0-VF || VF is a flag register
   bool display[64][32]; // Pixel by pixel, value 1 or 0
   uint8_t delay_timer;
   uint8_t sound_timer;
} CPU;

