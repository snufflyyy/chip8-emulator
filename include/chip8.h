#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct Chip8 {
    uint8_t memory[4096];
    bool program_loaded;

    uint8_t registers[16];
    uint16_t program_counter;

    uint16_t address_register;

    uint8_t delay_timer;
    uint8_t sound_timer;

    uint16_t stack[16];
    uint8_t stack_pointer;

    uint8_t display[64 * 32];
    bool update_display;

    uint8_t keypad[16];
} Chip8;

Chip8 chip8_create();
void chip8_load_rom(Chip8* chip8, char* file);
void chip8_update(Chip8* chip8);
