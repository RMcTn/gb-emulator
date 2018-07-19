#include <stdio.h>
#include <SDL2/SDL.h>
#include "cpu.h"
#include "memory.h"
#include "gpu.h"

void render(SDL_Renderer* renderer, SDL_Texture* texture, Cpu* cpu) {
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(renderer);
	uint8_t* pixels;
	int pitch = 0;
	SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch);
	
	memcpy(pixels, cpu->gpu.background_pixels, 160 * 144 * 4);

	SDL_UnlockTexture(texture);

	SDL_RenderCopy(renderer, texture, NULL, NULL);

	SDL_RenderPresent(renderer);
}

int main(int argc, char** argv) {
    Cpu cpu;
    reset_cpu(&cpu);

    if (argc == 2) {
        //Load rom
        FILE* rom = fopen(argv[1], "r");
        if (rom == NULL) {
            printf("Could not open %s\n", argv[1]);
            return 1;
        } 
        fseek(rom, 0, SEEK_END);
        long filelength = ftell(rom);
        rewind(rom);
        printf("filelength %ld\n", filelength);
        fread(cpu.memory, filelength, 1, rom);
        fclose(rom);
    }
    
    //execution stats at 0x100
    cpu.pc = 0x100;
	int window_scale = 5;
	SDL_Window* window = NULL;
	SDL_Texture* texture = NULL;
	SDL_Renderer* renderer = NULL;
	
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initalize. SDL Error: %s\n", SDL_GetError());
		//return 1;
	}
	window = SDL_CreateWindow("SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160 * window_scale, 144 * window_scale , SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("Window could not be created, SDL_Error %s\n", SDL_GetError());
		//return 1;
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
	SDL_RendererInfo info;
	SDL_GetRendererInfo(renderer, &info);
	printf("Render name: %s\n", info.name);
	printf("Texture formats: ");
	for (uint32_t i = 0; i < info.num_texture_formats; i++) {
		printf("%d\n", info.texture_formats[i]);
	}
	int i = 0;
	while (i < 500000) {
		int val = step(&cpu);
		if (val == 1) {
			printf("vblank render now\n");
			render(renderer, texture, &cpu);
		}
		//i++;	
	}
	SDL_Delay(2000);

    return 0;
}

