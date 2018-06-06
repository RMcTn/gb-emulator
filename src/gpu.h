#pragma once
#include <stdint.h>

#define MAX_DISPLAY_LINES 143
#define ROWS_IN_TILE 8                      //Each row of a tile is 8 pixels
#define NUM_OF_INDIVIDUAL_TILES 348         //2 tilesets of 256 tiles, but half of each tileset is shared

typedef struct Gpu {
    uint8_t mode;
    int mode_clock;
    int line; 
    uint16_t tileset[NUM_OF_INDIVIDUAL_TILES][ROWS_IN_TILE];
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

void gpu_step(Gpu* gpu, uint8_t last_t_clock);