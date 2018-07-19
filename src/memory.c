#include "cpu.h"
#include "memory.h"

uint8_t read_byte(Cpu* cpu, uint16_t address) {
	if (address == INTERRUPT_FLAGS_ADDRESS)
		return cpu->interrupt_flags;

	if (address == INTERRUPT_ENABLE_ADDRESS)
		return cpu->interrupt_enable;

	if (address >= GRAPHICS_RAM && address <= GRAPHICS_RAM_END) {
		return cpu->gpu.vram[address - 0x8000];
	}	

	//Hardware i/o registers
	//Check pandocs for rest
	
	//0xFF00 - Joypad register
	if (address == 0xFF00) {
		return cpu->joypad_register;
		//return cpu->memory[address];
	}
	
	//0xFF40 - LCD and GPU control
	if (address == 0xFF40) {
		return cpu->gpu.lcdc;	
	}	
	//0xFF41 - LCD Status Register
	if (address == 0xFF41) {
		return cpu->gpu.lcd_status_register;
	}
	//0xFF42 - Scroll Y
	if (address == 0xFF42) {
		return cpu->gpu.scroll_y;
	}
	//0xFF43 - Scroll X
	if (address == 0xFF43) {
		return cpu->gpu.scroll_x;
	}
	//0xFF44 - LCD LY
	if (address == 0xFF44) {
		return cpu->gpu.line;
	}
	//0xFF45 - LY compare
	return cpu->memory[address];
}

uint16_t read_word(Cpu* cpu, uint16_t address) {
    return read_byte(cpu, address) + (read_byte(cpu, address + 1) << 8);
}

void write_byte(Cpu* cpu, uint16_t address, uint8_t value) {
    if (address >= WORKING_RAM && address <= WORKING_RAM_END) {
		cpu->memory[address] = value;
        //write to shadow working ram as well
        address = (address - WORKING_RAM) + WORKING_RAM_SHADOW;
        if (address < WORKING_RAM_SHADOW_END)
            cpu->memory[address] = value;
		return;

    }

	if (address >= GRAPHICS_RAM && address <= GRAPHICS_RAM_END) {
		cpu->gpu.vram[address - 0x8000] = value;
		return;
	}	

	//0xFF00 - Joypad register
	if (address == 0xFF00) {
		cpu->joypad_register = value;
		//Saet bottom bits for now since we have no joypad
		cpu->joypad_register |= 0xF;
		//set top 2 bits cause they're set anyway
		cpu->joypad_register |= 0xC0;
		return;
	}

	//0xFF0F - Interrupt flags
	if (address == 0xFF0F) {
		cpu->interrupt_flags = value;
		return;
	}
	//0xFF40 - LCDC
	if (address == 0xFF40) {
		cpu->gpu.lcdc = value;
	}
	//0xFF41 - LCD Status Register
	if (address == 0xFF41) {
		cpu->gpu.lcd_status_register = value;
		return;
	}
	//0xFF42 - Scroll Y
	if (address == 0xFF42) {
		cpu->gpu.scroll_y = value;
		return;
	}
	//0xFF43 - Scroll X
	if (address == 0xFF43) {
		cpu->gpu.scroll_x = value;
		return;
	}
	//0xFF47 - BG Palette
	if (address == 0xFF47)  {
		cpu->gpu.background_palette = value;	
		return;
	}
	if (address == 0xFFFF) {
		cpu->interrupt_enable = value;
		return;
	}
    cpu->memory[address] = value;
}

void write_word(Cpu* cpu, uint16_t address, uint16_t value) {
    uint8_t first_byte = (uint8_t) value;
    write_byte(cpu, address, first_byte);
    uint8_t second_byte = (uint8_t) (value >> 8);
    write_byte(cpu, address + 1, second_byte);
}
