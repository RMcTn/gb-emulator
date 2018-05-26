#include "cpu.h"
#include "memory.h"

uint8_t read_byte(uint8_t memory[], uint16_t address) {
    return memory[address];
}

uint16_t read_word(uint8_t memory[], uint16_t address) {
    return read_byte(memory, address) + (read_byte(memory, address + 1) << 8);
}

void write_byte(uint8_t memory[], uint16_t address, uint8_t value) {
    memory[address] = value;
    if (address >= WORKING_RAM && address <= WORKING_RAM_END) {
        //write to shadow working ram as well
        address = (address - WORKING_RAM) + WORKING_RAM_SHADOW;
        if (address < WORKING_RAM_SHADOW_END)
            memory[address] = value;

    }
}

void write_word(uint8_t memory[], uint16_t address, uint16_t value) {
    uint8_t first_byte = (uint8_t) value;
    write_byte(memory, address, first_byte);
    uint8_t second_byte = (uint8_t) (value >> 8);
    write_byte(memory, address + 1, second_byte);
}