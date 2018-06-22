#include "cpu.h"
#include "memory.h"

uint8_t read_byte(Cpu* cpu, uint16_t address) {
	if (address == INTERRUPT_FLAGS_ADDRESS)
		return cpu->interrupt_flags;

	if (address == INTERRUPT_ENABLE_ADDRESS)
		return cpu->interrupt_enable;

	//Hardware i/o registers
	//Check pandocs for rest

	//0xFF42 - Scroll y
	//0xFF43 - Scroll x
	//0xFF44 - LCD LY
	if (address == 0xFF44) {
		//LCD LY
		return cpu->gpu.line;

		//0xFF45 - LY compare

	}

	return cpu->memory[address];
}

uint16_t read_word(Cpu* cpu, uint16_t address) {
    return read_byte(cpu, address) + (read_byte(cpu, address + 1) << 8);
}

void write_byte(Cpu* cpu, uint16_t address, uint8_t value) {
    cpu->memory[address] = value;
    if (address >= WORKING_RAM && address <= WORKING_RAM_END) {
        //write to shadow working ram as well
        address = (address - WORKING_RAM) + WORKING_RAM_SHADOW;
        if (address < WORKING_RAM_SHADOW_END)
            cpu->memory[address] = value;

    }
}

void write_word(Cpu* cpu, uint16_t address, uint16_t value) {
    uint8_t first_byte = (uint8_t) value;
    write_byte(cpu, address, first_byte);
    uint8_t second_byte = (uint8_t) (value >> 8);
    write_byte(cpu, address + 1, second_byte);
}
