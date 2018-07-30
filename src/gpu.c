#include <string.h>

#include "gpu.h"

void reset_gpu(Gpu* gpu) {
    memset(gpu, 0, sizeof(Gpu));
}

uint8_t gpu_step(Gpu* gpu, uint8_t last_t_clock) {
	uint8_t interrupts = 0;
	//Check if LCD display bit is enabled
	if (gpu->lcdc & 0x80) {
		gpu->mode_clock += last_t_clock;
		switch(gpu->mode) {
			case SCANLINE_OAM:
				if (gpu->mode_clock >= SCANLINE_OAM_CLOCKS) {
					gpu->mode_clock = 0;
					gpu->mode = SCANLINE_VRAM;
					//Clear the mode bits
					gpu->lcd_status_register &= ~LCD_MODE_BITS;
					//Set the mode bits
					gpu->lcd_status_register |= SCANLINE_VRAM;
				}
				break;
			case SCANLINE_VRAM:
				//end of this mode is end of scanline
				if  (gpu->mode_clock >= SCANLINE_VRAM_CLOCKS) {
					gpu->mode_clock = 0;
					gpu->mode = HBLANK;
					//Clear the mode bits
					gpu->lcd_status_register &= ~LCD_MODE_BITS;
					//Set the mode bits
					gpu->lcd_status_register |= HBLANK;
					//hblank flag and check if STAT register has hblank interrupts enabled
					if (CHECK_BIT(gpu->lcd_status_register, 3)) {
						interrupts |= STAT_REG_RST48;
					}

					//TODO: write out scanline to the framebuffer
					render_background(gpu);
				}
				break;
			case HBLANK:
				if (gpu->mode_clock >= HBLANK_CLOCKS) {
					gpu->mode_clock = 0;
					gpu->line++;

					if (gpu->line == MAX_DISPLAY_LINES) {
						gpu->mode = VBLANK;
						//Clear the mode bits
						gpu->lcd_status_register &= ~LCD_MODE_BITS;
						//Set the mode bits
						gpu->lcd_status_register |= VBLANK;
						//vblank flag and check if STAT register has vblank interrupts enabled
						if (CHECK_BIT(gpu->lcd_status_register, 4)) {
							interrupts |= STAT_REG_RST48;
						}
						interrupts |= VBLANK_RST40;
					} else {
						gpu->mode = SCANLINE_OAM;
						//Check if OAM interrupt is enabled
						//Clear the mode bits
						gpu->lcd_status_register &= ~LCD_MODE_BITS;
						//Set the mode bits
						gpu->lcd_status_register |= SCANLINE_OAM;
						if (CHECK_BIT(gpu->lcd_status_register, 5)) {
							interrupts |= STAT_REG_RST48;
						}
					}
				}
				break;
			case VBLANK:
				if (gpu->mode_clock >= SINGLE_LINE_CLOCKS) {
					gpu->mode_clock = 0;
					gpu->line++;

					if (gpu->line > MAX_LINES) {
						//TODO: Double check this if this is correct
						gpu->mode = SCANLINE_OAM;
						//Clear the mode bits
						gpu->lcd_status_register &= ~LCD_MODE_BITS;
						//Set the mode bits
						gpu->lcd_status_register |= SCANLINE_OAM;
						//Check if OAM interrupt is enabled
						gpu->line = 0;
						if (CHECK_BIT(gpu->lcd_status_register, 5)) {
							interrupts |= STAT_REG_RST48;
						}
					}
				}
				break;
		}
	}

	if (gpu->line_y_compare == gpu->line) {
		//Set the register (2nd bit)
		gpu->lcd_status_register |= LCD_COINCIDENCE_BIT;
		if (CHECK_BIT(gpu->lcd_status_register, 6)) {
			interrupts |= STAT_REG_RST48;
		}
	} else {
		//Clear the register (2nd bit)
		gpu->lcd_status_register &= ~(LCD_COINCIDENCE_BIT);
	}
	return interrupts;
}

void render_background(Gpu* gpu) { 
	//Check if background is not enabled
	if (!CHECK_BIT(gpu->lcdc, 0)) {
		//Background is disabled, just render white
		memset(gpu->background_pixels, 0xFF, SCREEN_WIDTH * SCREEN_HEIGHT * 4);
		return;
	}
	
	//Set the correct address of the bg tilemap
	uint16_t bg_tilemap_address = CHECK_BIT(gpu->lcdc, 3) ? 0x9C00 : 0x9800;
	bg_tilemap_address -= 0x8000;

	uint16_t bg_tile_address = CHECK_BIT(gpu->lcdc, 4) ? 0x8000 : 0x8800;
	bg_tile_address -= 0x8000;

	uint8_t tile_y = (((gpu->line + gpu->scroll_y) / 8) % 32);
	uint8_t tile_y_offset = ((gpu->line + gpu->scroll_y) % 8);

	for (uint8_t i = 0; i < SCREEN_WIDTH; i++) {
		uint8_t tile_x = (((gpu->scroll_x + i) / 8) % 32);
		uint8_t tile_number = gpu->vram[bg_tilemap_address + (tile_y * 32) + tile_x];

		uint16_t tile_offset = 0;
		if (CHECK_BIT(gpu->lcdc, 4)) {
			//Unsigned tile_number
			tile_offset = bg_tile_address + tile_number * 0x10;
		} else {
			//Signed tile_number
			tile_offset = bg_tile_address + (int8_t)(tile_number) * 0x10;
		}
		tile_offset += tile_y_offset * 2;

		uint8_t first_byte = gpu->vram[tile_offset];
		uint8_t second_byte = gpu->vram[tile_offset + 1];

		uint8_t bit = (7 - ((gpu->scroll_x  + i) % 8));
		uint8_t palette_low_bit = CHECK_BIT(first_byte, bit) ? 0x01 : 0x00;
		uint8_t palette_high_bit = CHECK_BIT(second_byte, bit) ? 0x02 : 0x00;

		const uint8_t colours[] = {
			0xEB,
			0xC4,
			0x60,
			0x00
		};

		const uint8_t palette[] = {
			colours[gpu->background_palette & 0x03],
			colours[(gpu->background_palette >> 2) & 0x03],
			colours[(gpu->background_palette >> 4) & 0x03],
			colours[(gpu->background_palette >> 6) & 0x03]
		};

		uint8_t colour = palette[palette_low_bit + palette_high_bit];
		int offset = ((gpu->line * SCREEN_WIDTH) + i) * 4;
		gpu->background_pixels[offset + 0] = 0xFF;		//A
		//RGB
		gpu->background_pixels[offset + 1] = colour;		
		gpu->background_pixels[offset + 2] = colour;		
		gpu->background_pixels[offset + 3] = colour;		
	}

}

