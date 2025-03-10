/*
 * the Chip-8 reference used: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#memmap
*/

#include "chip8.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// 0nnn is not implemented as modern computers do not need it
static void instruction_00E0(Chip8* chip8);
static void instruction_00EE(Chip8* chip8);
static void instruction_1nnn(Chip8* chip8, uint16_t location);
static void instruction_2nnn(Chip8* chip8, uint16_t location);
static void instruction_3xkk(Chip8* chip8, uint8_t Vx, uint8_t value);
static void instruction_4xkk(Chip8* chip8, uint8_t Vx, uint8_t value);
static void instruction_5xy0(Chip8* chip8, uint8_t Vx, uint8_t Vy);
static void instruction_6xkk(Chip8* chip8, uint8_t Vx, uint8_t value);
static void instruction_7xkk(Chip8* chip8, uint8_t Vx, uint8_t value);
static void instruction_8xy0(Chip8* chip8, uint8_t Vx, uint8_t Vy);
static void instruction_8xy1(Chip8* chip8, uint8_t Vx, uint8_t Vy);
static void instruction_8xy2(Chip8* chip8, uint8_t Vx, uint8_t Vy);
static void instruction_8xy3(Chip8* chip8, uint8_t Vx, uint8_t Vy);
static void instruction_8xy4(Chip8* chip8, uint8_t Vx, uint8_t Vy);
static void instruction_8xy5(Chip8* chip8, uint8_t Vx, uint8_t Vy);
static void instruction_8xy6(Chip8* chip8, uint8_t Vx, uint8_t Vy);
static void instruction_8xy7(Chip8* chip8, uint8_t Vx, uint8_t Vy);
static void instruction_8xyE(Chip8* chip8, uint8_t Vx, uint8_t Vy);
static void instruction_9xy0(Chip8* chip8, uint8_t Vx, uint8_t Vy);
static void instruction_Annn(Chip8* chip8, uint16_t location);
static void instruction_Bnnn(Chip8* chip8, uint16_t location);
static void instruction_Cxkk(Chip8* chip8, uint8_t Vx, uint8_t value);
static void instruction_Dxyn(Chip8* chip8, uint8_t Vx, uint8_t Vy, uint8_t size);
static void instruction_Ex9E(Chip8* chip8, uint8_t Vx);
static void instruction_ExA1(Chip8* chip8, uint8_t Vx);
static void instruction_Fx07(Chip8* chip8, uint8_t Vx);
static void instruction_Fx0A(Chip8* chip8, uint8_t Vx);
static void instruction_Fx15(Chip8* chip8, uint8_t Vx);
static void instruction_Fx18(Chip8* chip8, uint8_t Vx);
static void instruction_Fx1E(Chip8* chip8, uint8_t Vx);
static void instruction_Fx29(Chip8* chip8, uint8_t Vx);
static void instruction_Fx33(Chip8* chip8, uint8_t Vx);
static void instruction_Fx55(Chip8* chip8, uint8_t Vx);
static void instruction_Fx65(Chip8* chip8, uint8_t Vx);

Chip8 chip8_create() {
    Chip8 chip8 = {0};

    uint8_t font_data[5 * 16] = {0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
                                 0x20, 0x60, 0x20, 0x20, 0x70,  // 1
                                 0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
                                 0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
                                 0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
                                 0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
                                 0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
                                 0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
                                 0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
                                 0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
                                 0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
                                 0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
                                 0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
                                 0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
                                 0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
                                 0xF0, 0x80, 0xF0, 0x80, 0x80}; // F

    // load font into memory
    for (int i = 0; i < 80; i++) {
        chip8.memory[i] = font_data[i];
    }

    return chip8;
}

void chip8_load_rom(Chip8* chip8, char* file) {
    FILE* rom_file = fopen(file, "rb");
    if (!rom_file) {
        printf("ERROR: Failed to open rom file!\n");
        return;
    }

    // load rom into memory starting at 0x200 (512)
    int current_byte = fgetc(rom_file);
    int index = 0x200;
    while (current_byte != EOF) {
        chip8->memory[index++] = (uint8_t) current_byte;

        current_byte = fgetc(rom_file);
    }

    // set program counter to start of program
    chip8->program_counter = 0x200;

    fclose(rom_file);

    chip8->program_loaded = true;
}

// fetch -> decode -> execute
uint16_t fetch_instruction(Chip8* chip8) {
    uint16_t instruction;

    instruction = chip8->memory[chip8->program_counter] << 8;
    instruction |= chip8->memory[chip8->program_counter + 1];
    chip8->program_counter += 2;

    return instruction;
}

void chip8_update(Chip8* chip8) {
    if (chip8->program_loaded) {
        uint16_t instruction = fetch_instruction(chip8);
        printf("%04x - ", chip8->program_counter);

        uint8_t opcode = (instruction >> 12) & 0xF;
        switch (opcode) {
            case 0x0:
                switch (instruction & 0x00FF) {
                    case 0xE0: instruction_00E0(chip8); break;
                    case 0xEE: instruction_00EE(chip8); break;
                }
                break;
            case 0x1: instruction_1nnn(chip8, instruction & 0x0FFF); break;
            case 0x2: instruction_2nnn(chip8, instruction & 0x0FFF); break;
            case 0x3: instruction_3xkk(chip8, (instruction >> 8) & 0x0F, instruction & 0x00FF); break;
            case 0x4: instruction_4xkk(chip8, (instruction >> 8) & 0x0F, instruction & 0x00FF); break;
            case 0x5: instruction_5xy0(chip8, (instruction >> 8) & 0x0F, (instruction >> 4) & 0x0F); break;
            case 0x6: instruction_6xkk(chip8, (instruction >> 8) & 0x0F, instruction & 0x00FF); break;
            case 0x7: instruction_7xkk(chip8, (instruction >> 8) & 0x0F, instruction & 0x00FF); break;
            case 0x8:
                switch (instruction & 0x000F) {
                    case 0x0: instruction_8xy0(chip8, (instruction >> 8) & 0x0F, (instruction >> 4) & 0x0F); break;
                    case 0x1: instruction_8xy1(chip8, (instruction >> 8) & 0x0F, (instruction >> 4) & 0x0F); break;
                    case 0x2: instruction_8xy2(chip8, (instruction >> 8) & 0x0F, (instruction >> 4) & 0x0F); break;
                    case 0x3: instruction_8xy3(chip8, (instruction >> 8) & 0x0F, (instruction >> 4) & 0x0F); break;
                    case 0x4: instruction_8xy4(chip8, (instruction >> 8) & 0x0F, (instruction >> 4) & 0x0F); break;
                    case 0x5: instruction_8xy5(chip8, (instruction >> 8) & 0x0F, (instruction >> 4) & 0x0F); break;
                    case 0x6: instruction_8xy6(chip8, (instruction >> 8) & 0x0F, (instruction >> 4) & 0x0F); break;
                    case 0x7: instruction_8xy7(chip8, (instruction >> 8) & 0x0F, (instruction >> 4) & 0x0F); break;
                    case 0xE: instruction_8xyE(chip8, (instruction >> 8) & 0x0F, (instruction >> 4) & 0x0F); break;
                }
                break;
            case 0x9: instruction_9xy0(chip8, (instruction >> 8) & 0x0F, (instruction >> 4) & 0x0F); break;
            case 0xA: instruction_Annn(chip8, instruction & 0x0FFF); break;
            case 0xB: instruction_Bnnn(chip8, instruction & 0x0FFF); break;
            case 0xC: instruction_Cxkk(chip8, (instruction >> 8) & 0x0F, instruction & 0x00FF); break;
            case 0xD: instruction_Dxyn(chip8, (instruction >> 8) & 0x0F, (instruction >> 4) & 0x0F, instruction & 0x000F); break;
            case 0xE:
                switch (instruction & 0x00FF) {
                    case 0x9E: instruction_Ex9E(chip8, (instruction >> 8) & 0x0F); break;
                    case 0xA1: instruction_ExA1(chip8, (instruction >> 8) & 0x0F); break;
                }
                break;
            case 0xF:
                switch (instruction & 0x00FF) {
                    case 0x07: instruction_Fx07(chip8, (instruction >> 8) & 0x0F); break;
                    case 0x0A: instruction_Fx0A(chip8, (instruction >> 8) & 0x0F); break;
                    case 0x15: instruction_Fx15(chip8, (instruction >> 8) & 0x0F); break;
                    case 0x18: instruction_Fx18(chip8, (instruction >> 8) & 0x0F); break;
                    case 0x1E: instruction_Fx1E(chip8, (instruction >> 8) & 0x0F); break;
                    case 0x29: instruction_Fx29(chip8, (instruction >> 8) & 0x0F); break;
                    case 0x33: instruction_Fx33(chip8, (instruction >> 8) & 0x0F); break;
                    case 0x55: instruction_Fx55(chip8, (instruction >> 8) & 0x0F); break;
                    case 0x65: instruction_Fx65(chip8, (instruction >> 8) & 0x0F); break;
                }
                break;
        }
    }
}

static void instruction_00E0(Chip8* chip8) {
    printf("CLS\n");
    memset(chip8->display, 0, (64 * 32) * sizeof(uint8_t));
}

static void instruction_00EE(Chip8* chip8) {
    printf("RTE\n");
    chip8->stack_pointer -= 1;
    chip8->program_counter = chip8->stack[chip8->stack_pointer];
}

static void instruction_1nnn(Chip8* chip8, uint16_t location) {
    printf("JP %04x\n", location);
    chip8->program_counter = location;
}

static void instruction_2nnn(Chip8* chip8, uint16_t location) {
    printf("CALL %04x\n", location);
    chip8->stack[chip8->stack_pointer++] = chip8->program_counter;
    chip8->program_counter = location;
}

static void instruction_3xkk(Chip8* chip8, uint8_t Vx, uint8_t value) {
    printf("SE V%02x, %02x\n", Vx, value);
    if (chip8->registers[Vx] == value) {
        chip8->program_counter += 2;
    }
}

static void instruction_4xkk(Chip8* chip8, uint8_t Vx, uint8_t value) {
    printf("SNE V%02x, %02x\n", Vx, value);
    if (chip8->registers[Vx] != value) {
        chip8->program_counter += 2;
    }
}

static void instruction_5xy0(Chip8* chip8, uint8_t Vx, uint8_t Vy) {
    printf("SNE V%02x, V%02x\n", Vx, Vy);
    if (chip8->registers[Vx] == chip8->registers[Vy]) {
        chip8->program_counter += 2;
    }
}

static void instruction_6xkk(Chip8* chip8, uint8_t Vx, uint8_t value) {
    printf("LD V%02x, %02x\n", Vx, value);
    chip8->registers[Vx] = value;
}

static void instruction_7xkk(Chip8* chip8, uint8_t Vx, uint8_t value) {
    printf("ADD V%02x, %02x\n", Vx, value);
    chip8->registers[Vx] += value;
}

static void instruction_8xy0(Chip8* chip8, uint8_t Vx, uint8_t Vy) {
    printf("LD V%02x, V%02x\n", Vx, Vy);
    chip8->registers[Vx] = chip8->registers[Vy];
}

static void instruction_8xy1(Chip8* chip8, uint8_t Vx, uint8_t Vy) { // good
    printf("OR V%02x, V%02x\n", Vx, Vy);
    chip8->registers[Vx] |= chip8->registers[Vy];
}

static void instruction_8xy2(Chip8* chip8, uint8_t Vx, uint8_t Vy) { // good
    printf("AND V%02x, V%02x\n", Vx, Vy);
    chip8->registers[Vx] &= chip8->registers[Vy];
}

static void instruction_8xy3(Chip8* chip8, uint8_t Vx, uint8_t Vy) { // good
    printf("XOR V%02x, V%02x\n", Vx, Vy);
    chip8->registers[Vx] ^= chip8->registers[Vy];
}

static void instruction_8xy4(Chip8* chip8, uint8_t Vx, uint8_t Vy) { // bad
    printf("ADD V%02x, V%02x\n", Vx, Vy);

    chip8->registers[0xF] = 0;
    uint16_t sum = chip8->registers[Vx] + chip8->registers[Vy];
    if (sum > 0xFF) {
        chip8->registers[0xF] = 1;
    }
    chip8->registers[Vx] = (sum >> 8) & 0x0F;
}

static void instruction_8xy5(Chip8* chip8, uint8_t Vx, uint8_t Vy) { // bad
    printf("SUB V%02x, V%02x\n", Vx, Vy);

    chip8->registers[0xF] = 0;
    if (chip8->registers[Vx] >= chip8->registers[Vy]) {
        chip8->registers[0xF] = 1;
    }
    chip8->registers[Vx] -= chip8->registers[Vy];
}

static void instruction_8xy6(Chip8* chip8, uint8_t Vx, uint8_t Vy) { // bad
    printf("SHR V%02x, V%02x\n", Vx, Vy);

    chip8->registers[0xF] = chip8->registers[Vx] & 0x1;
    chip8->registers[Vx] >>= 1;
}

static void instruction_8xy7(Chip8* chip8, uint8_t Vx, uint8_t Vy) { // bad
    printf("SUBN V%02x, V%02x\n", Vx, Vy);

    chip8->registers[0xF] = 0;
    if (chip8->registers[Vy] >= chip8->registers[Vx]) {
        chip8->registers[0xF] = 1;
    }
    chip8->registers[Vx] = chip8->registers[Vy] - chip8->registers[Vx];
}

static void instruction_8xyE(Chip8* chip8, uint8_t Vx, uint8_t Vy) { // bad
    printf("SHL V%02x, V%02x\n", Vx, Vy);

    chip8->registers[0xF] = chip8->registers[Vx] & 0x7;

    chip8->registers[Vx] <<= 1;
}

static void instruction_9xy0(Chip8* chip8, uint8_t Vx, uint8_t Vy) {
    printf("SNE V%02x, V%02x\n", Vx, Vy);
    if (chip8->registers[Vx] != chip8->registers[Vy]) {
        chip8->program_counter += 2;
    }
}

static void instruction_Annn(Chip8* chip8, uint16_t location) {
    printf("LD I, %04x\n", location);
    chip8->address_register = location;
}

static void instruction_Bnnn(Chip8* chip8, uint16_t location) {
    printf("JP V0, %04x\n", location);
    chip8->program_counter = location + chip8->registers[0];
}

static void instruction_Cxkk(Chip8* chip8, uint8_t Vx, uint8_t value) {
    printf("RND V%02x, %02x\n", Vx, value);
    chip8->registers[Vx] = (rand() % (255 - 1)) & value;
}

static void instruction_Dxyn(Chip8* chip8, uint8_t Vx, uint8_t Vy, uint8_t size) {
    printf("DRW V%02x, V%02x, %02x\n", Vx, Vy, size);

    uint8_t x_position = chip8->registers[Vx] % 64;
    uint8_t y_position = chip8->registers[Vy] % 32;
    chip8->registers[0xF] = 0;

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < 8; x++) {
            int index = (y_position + y) * 64 + (x_position + x);

            if ((chip8->memory[chip8->address_register + y] >> (7 - x)) & 1) {
                if (chip8->display[index] > 0x0) {
                    chip8->registers[0xF] = 1;
                }

                chip8->display[index] ^= 0xFF;
            }
        }
    }

    chip8->update_display = true;
}

static uint8_t get_keypad_value(int index) {
    uint8_t value;

    switch (index) {
        case 0:  value = 0x1; break;
        case 1:  value = 0x2; break;
        case 2:  value = 0x3; break;
        case 3:  value = 0xC; break;

        case 4:  value = 0x4; break;
        case 5:  value = 0x5; break;
        case 6:  value = 0x6; break;
        case 7:  value = 0xD; break;

        case 8:  value = 0x7; break;
        case 9:  value = 0x8; break;
        case 10: value = 0x9; break;
        case 11: value = 0xE; break;

        case 12: value = 0xA; break;
        case 13: value = 0x0; break;
        case 14: value = 0xB; break;
        case 15: value = 0xF; break;
    }

    return value;
}

static int get_keypad_index(uint8_t value) {
    int index;

    switch (value) {
        case 0x1: index = 0;  break;
        case 0x2: index = 1;  break;
        case 0x3: index = 2;  break;
        case 0xC: index = 3;  break;

        case 0x4: index = 4;  break;
        case 0x5: index = 5;  break;
        case 0x6: index = 6;  break;
        case 0xD: index = 7;  break;

        case 0x7: index = 8;  break;
        case 0x8: index = 9;  break;
        case 0x9: index = 10; break;
        case 0xE: index = 11; break;

        case 0xA: index = 12; break;
        case 0x0: index = 13; break;
        case 0xB: index = 14; break;
        case 0xF: index = 15; break;
    }

    return index;
}

static void instruction_Ex9E(Chip8* chip8, uint8_t Vx) {
    printf("SKP %02x\n", Vx);
    if (chip8->keypad[get_keypad_index(chip8->registers[Vx])]) {
        chip8->program_counter += 2;
    }
}

static void instruction_ExA1(Chip8* chip8, uint8_t Vx) {
    printf("SKNP %02x\n", Vx);
    if (!chip8->keypad[get_keypad_index(chip8->registers[Vx])]) {
        chip8->program_counter += 2;
    }
}

static void instruction_Fx07(Chip8* chip8, uint8_t Vx) {
    printf("LD V%02x, DT\n", Vx);
    chip8->registers[Vx] = chip8->delay_timer;
}

static void instruction_Fx0A(Chip8* chip8, uint8_t Vx) {
    printf("LD V%02x\n", Vx);
    for (int i = 0; i < 16; i++) {
        if (chip8->keypad[i] == 1) {
            chip8->registers[Vx] = get_keypad_value(i);
            return;
        }
    }
    chip8->program_counter -= 2;
}

static void instruction_Fx15(Chip8* chip8, uint8_t Vx) {
    printf("LD DT, V%02x\n", Vx);
    chip8->delay_timer = chip8->registers[Vx];
}

static void instruction_Fx18(Chip8* chip8, uint8_t Vx) {
    printf("LD ST, V%02x\n", Vx);
    chip8->sound_timer = chip8->registers[Vx];
}

static void instruction_Fx1E(Chip8* chip8, uint8_t Vx) {
    printf("ADD I, V%02x\n", Vx);
    chip8->address_register += chip8->registers[Vx];
}

static void instruction_Fx29(Chip8* chip8, uint8_t Vx) {
    printf("LD F, V%02x\n", Vx);
    chip8->address_register = get_keypad_index(chip8->registers[Vx]);
}

static void instruction_Fx33(Chip8* chip8, uint8_t Vx) {
    printf("LD B, V%02x\n", Vx);
    chip8->memory[chip8->address_register] = chip8->registers[Vx] / 100;
    chip8->memory[chip8->address_register + 1] = (chip8->registers[Vx] / 10) % 10;
    chip8->memory[chip8->address_register + 2] = chip8->registers[Vx] % 10;
}

static void instruction_Fx55(Chip8* chip8, uint8_t Vx) {
    printf("LD I, V%02x\n", Vx);
    for (int i = 0; i <= Vx; i++) {
        chip8->memory[chip8->address_register + i] = chip8->registers[i];
    }
}

static void instruction_Fx65(Chip8* chip8, uint8_t Vx) {
    printf("LD V%02x, I\n", Vx);
    for (int i = 0; i <= Vx; i++) {
        chip8->registers[i] = chip8->memory[chip8->address_register + i];
    }
}
