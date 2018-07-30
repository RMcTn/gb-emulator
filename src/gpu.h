#pragma once
#include <stdint.h>
#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))
#define CLEAR_BIT(var, pos) ((var) &= ~((1) << (pos)))

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

#define MAX_DISPLAY_LINES 144				//Amount of lines shown before vblank state is entered
#define MAX_LINES 153						//Max lines including vblank period
#define ROWS_IN_TILE 8                      //Each row of a tile is 8 pixels
#define NUM_OF_INDIVIDUAL_TILES 348         //2 tilesets of 256 tiles, but half of each tileset is shared
#define LCD_MODE_BITS		0x03
#define LCD_COINCIDENCE_BIT	0x04

typedef struct Gpu {
    uint8_t mode;
    int mode_clock;
    uint8_t line; 
	uint8_t line_y_compare;
    uint16_t tileset[NUM_OF_INDIVIDUAL_TILES][ROWS_IN_TILE];
	uint8_t scroll_y;
	uint8_t scroll_x;

	/*
 	 * 0xFF40 LCD Control Register (LCDC) Bits
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

	/*
 	 * 0xFF41 LCD Status Register (STAT) Bits
	 * Bit 0-1	- Mode flag
	 * Bit 2	- Coincidence flag					(Set if LYC = LY)
	 * Bit 3	- Mode 0 H-Blank interrupt			(1 = Enable)
	 * Bit 4	- Mode 1 V-Blank interrupt			(1 = Enable)
	 * Bit 5	- Mode 2 OAM interrupt				(1 = Enable)
	 * Bit 6	- LYC=LY Coincidence interrupt		(1 = Enable)
 	 */
	uint8_t lcd_status_register;
	uint8_t background_palette;

	uint8_t background_pixels[SCREEN_WIDTH * SCREEN_HEIGHT * 4];		//4 for ABGR

	uint8_t vram[0x1FFF];		//Temp for now (maybe permanent)
} Gpu;

enum GpuModes {
    HBLANK          = 0x0,			//204 clocks
    VBLANK          = 0x1,			//4560 clocks
    SCANLINE_OAM    = 0x2,			//80 clocks
    SCANLINE_VRAM   = 0x3			//172 clocks
};

enum GpuClocks {
    SCANLINE_OAM_CLOCKS     = 80,
    SCANLINE_VRAM_CLOCKS    = 172,
    HBLANK_CLOCKS           = 204,
    SINGLE_LINE_CLOCKS      = 456,      //Hblank and scanline memory accesses summed
    VBLANK_CLOCKS           = 4560,     //10 single lines
    FULL_FRAME_CLOCKS       = 70224
};

enum GpuInterrupts {
	VBLANK_RST40			= 0x01,
	STAT_REG_RST48			= 0x02
};

void reset_gpu(Gpu* gpu);

//Processes timings and modes for gpu
//Returns a number which is used to set interrupts
uint8_t gpu_step(Gpu* gpu, uint8_t last_t_clock);

void render_background(Gpu* gpu);
