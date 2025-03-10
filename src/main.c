#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include <SDL3/SDL.h>

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_scancode.h"
#include "chip8.h"

// what number to scale the chip8's resolution (64x32) by
#define SCALE 15

int main() {
    // set random seed
    srand(time(0));

    // init sdl
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("ERROR: Failed to initialize SDL!\n");
        return -1;
    }

    // create window
    SDL_Window* window = SDL_CreateWindow("Chip 8 Emulator", 64 * SCALE, 32 * SCALE, 0);
    if (!window) {
        printf("ERROR: Failed to create SDL window!\n");
        return -2;
    }

    // create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        printf("ERROR: Failed to create SDL renderer!\n");
        return -3;
    }

    // keyboard keys
    int keyboard_scancodes[16] = {SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
                                  SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R,
                                  SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F,
                                  SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_C, SDL_SCANCODE_V};

    // create texture which will be used to display the chip8's framebuffer
    SDL_Texture* chip8_display_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    SDL_SetTextureScaleMode(chip8_display_texture, SDL_SCALEMODE_NEAREST);

    Chip8 chip8 = chip8_create();

    SDL_Event event;
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                // window events
                case SDL_EVENT_QUIT: running = false; break;
                case SDL_EVENT_DROP_FILE: chip8_load_rom(&chip8, (char*) event.drop.data); break;

                // keyboard events
                case SDL_EVENT_KEY_DOWN:
                    for (int i = 0; i < 16; i++) {
                        if (event.key.scancode == keyboard_scancodes[i]) {
                            chip8.keypad[i] = 1;
                        }
                    }
                    break;
                case SDL_EVENT_KEY_UP:
                    for (int i = 0; i < 16; i++) {
                        if (event.key.scancode == keyboard_scancodes[i]) {
                            chip8.keypad[i] = 0;
                        }
                    }
                    break;
            }
        }

        // update
        for (int i = 0; i < 1; i++) { // temp for loop
            chip8_update(&chip8);
        }

        // update the display texture
        if (chip8.update_display) {
            SDL_UpdateTexture(chip8_display_texture, NULL, chip8.display, 64 * sizeof(uint8_t));
            chip8.update_display = false;
        }

        // render
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, chip8_display_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
