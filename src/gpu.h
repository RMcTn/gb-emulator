#pragma once
#include <stdint.h>

#define MAX_DISPLAY_LINES 144				//Amount of lines shown before vblank state is entered
#define MAX_LINES 153						//Max lines including vblank period
#define ROWS_IN_TILE 8                      //Each row of a tile is 8 pixels
#define NUM_OF_INDIVIDUAL_TILES 348         //2 tilesets of 256 tiles, but half of each tileset is shared

typedef struct Gpu {
    uint8_t mode;
    int mode_clock;
    uint8_t line; 
    uint16_t tileset[NUM_OF_INDIVIDUAL_TILES][ROWS_IN_TILE];
	uint8_t scroll_y;
	uint8_t scroll_x;

	/**
 	 * LCDC Bits
	 * 0 = Off,  1 = On, unless stated otherwise
 	 * Bit 0 - BG enable
	 * Bit 1 - OBJ enable
	 * Bit 2 - OBJ size					0 = 8x8, 1 = 8x16
	 * Bit 3 - BG map select			0 = 0x9800-9BFF, 1=0x9C00-0x9FFF
	 * Bit 4 - BG tile select			0 = 0x8800-0x97FF, 1 = 0x8000-0x8FFF
	 * Bit 5 - Window display enable
	 * Bit 6 - Window map select		0 = 0x9800-0x9BFF, 1 = 0x9C00-0x9FFF
	 * Bit 7 - LCD enable
	 */
	uint8_t lcdc;
	uint8_t background_palette;
} Gpu;

enum GpuModes {
    SCANLINE_OAM    = 2,        //80 clocks
    SCANLINE_VRAM   = 3,        //172 clocks
    HBLANK          = 0,        //204 clocks
    VBLANK          = 1         //4560 clocks
};

enum GpuClocks {
    SCANLINE_OAM_CLOCKS     = 80,
    SCANLINE_VRAM_CLOCKS    = 172,
    HBLANK_CLOCKS           = 204,
    SINGLE_LINE_CLOCKS      = 456,      //Hblank and scanline memory accesses summed
    VBLANK_CLOCKS           = 4560,     //10 single lines
    FULL_FRAME_CLOCKS       = 70224
};

void reset_gpu(Gpu* gpu);

//Processes timings and modes for gpu
//Returns a number which is used to set interrupts
uint8_t gpu_step(Gpu* gpu, uint8_t last_t_clock);
