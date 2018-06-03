#include "gpu.h"

void gpu_step(Gpu* gpu, uint8_t last_t_clock) {
    gpu->mode_clock += last_t_clock;

    switch(gpu->mode) {
        case SCANLINE_OAM:
            if (gpu->mode_clock >= SCANLINE_OAM_CLOCKS) {
                gpu->mode_clock = 0;
                gpu->mode = SCANLINE_VRAM;
            }
            break;
        case SCANLINE_VRAM:
            //end of this mode is end of scanline
            if  (gpu->mode_clock >= SCANLINE_VRAM_CLOCKS) {
                gpu->mode_clock = 0;
                gpu->mode = HBLANK;

                //TODO: write out scanline to the framebuffer
            }
            break;
        case HBLANK:
            if (gpu->mode_clock >= HBLANK_CLOCKS) {
                gpu->mode_clock = 0;
                gpu->line++;

                if (gpu->line == MAX_DISPLAY_LINES) {
                    gpu->mode = VBLANK;
                    //TODO: draw frame
                } else {
                    gpu->mode = SCANLINE_OAM;
                }
            }
            break;
        case VBLANK:
            if (gpu->mode_clock >= SINGLE_LINE_CLOCKS) {
                gpu->mode_clock = 0;
                gpu->line++;

                if (gpu->line > MAX_DISPLAY_LINES) {
                    //TODO: Double check this if this is correct
                    gpu->mode = SCANLINE_OAM;
                    gpu->line = 0;
                }
            }
            break;
    }
}