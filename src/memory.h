#include <stdint.h>

#define CARTRIDGE_ROM_BANK_0        0x0000  //0x0000 - 0x3FFF
#define BIOS                        0x0000  //0x0000 - 0x00FF
#define CARTRIDGE_HEADER            0x0100  //0x0100 - 0x014F
#define CARTRIDGE_ROM_OTHER_BANKS   0x4000  //0x4000 - 0x7FFF
#define GRAPHICS_RAM                0x8000  //0x8000 - 0x9FFF
#define EXTERNAL_CARTRIDGE_RAM      0xA000  //0xA000 - 0xBFFF
#define WORKING_RAM                 0xC000  //0xC000 - 0xDFFF
#define WORKING_RAM_END             0xDFFF
#define WORKING_RAM_SHADOW          0xE000  //0xE000 - 0xFDFF
#define WORKING_RAM_SHADOW_END      0xFDFF
#define SPRITE_INFO                 0xFE00  //0xFE00 - 0xFE9F
#define MEM_MAPPED_IO               0xFF00  //0xFF00 - 0xFF7F
#define ZERO_PAGE_RAM               0xFF80  //0xFF80 - 0xFFFF
#define MEMORY_SIZE                 0xFFFF

uint8_t read_byte(uint8_t memory[], uint16_t address);
uint16_t read_word(uint8_t memory[], uint16_t address);

void write_byte(uint8_t memory[], uint16_t address, uint8_t value);
void write_word(uint8_t memory[], uint16_t address, uint16_t value);
