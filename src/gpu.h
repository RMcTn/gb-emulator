#pragma once
#include <stdint.h>

#define MAX_DISPLAY_LINES 143

typedef struct Gpu {
    uint8_t mode;
    int mode_clock;
    int line; 
} Gpu;

enum GpuModes {
    SCANLINE_OAM    = 2,        //80 clocks
    SCANLINE_VRAM   = 3,        //172 clocks
    HBLANK          = 0,        //204 clocks
    VBLANK          = 1         //4560 clocks

    //One line takes 456 clocks (hblank + scanline_vram + scanline_oam)
    //Vblank is 10 of those lines
    //Full frame is 70224 with scans and vblank
};

enum GpuClocks {
    SCANLINE_OAM_CLOCKS     = 80,
    SCANLINE_VRAM_CLOCKS    = 172,
    HBLANK_CLOCKS           = 204,
    SINGLE_LINE_CLOCKS      = 456,
    VBLANK_CLOCKS           = 4560,
    FULL_FRAME_CLOCKS       = 70224
};

void gpu_step(Gpu* gpu, uint8_t last_t_clock);