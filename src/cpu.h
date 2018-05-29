#pragma once
#include <stdint.h>

#include "memory.h"

typedef struct Cpu {
	uint8_t a;                      //Accumulator register
	uint8_t f;                      //Flag register (lower 4 bits are always 0)
	uint8_t b, c, d, e, h, l;       //8 bit registers 
	uint16_t sp, pc;               	//16 bit registers
	uint8_t m, t;				   	//clocks for last instruction
									//t increments with each clock step, m being a quarter of t
	uint8_t memory[UINT16_MAX];		//16 bit address bus
} Cpu;

enum CpuFlags {
	CARRY_FLAG = 0x10,				/* Last operation result was over 255 (addition)
									or under 0 (subtraction) */
	HALFCARRY_FLAG = 0x20,			/* Last operation's result's lower half of
									byte overflowed past 15 */
	SUBTRACTION_FLAG = 0x40,		//Last operation was a subtraction
	ZERO_FLAG = 0x80				//Last operation had result of 0
};

void print_cpu_contents();

void set_flag(Cpu* cpu, int flag); 
void clear_flag(Cpu* cpu, int flag);

void add_to_accumulator(Cpu* cpu, uint8_t n);
void add_to_accumulator_with_carry(Cpu* cpu, uint8_t n);

void subtract_from_accumulator(Cpu* cpu, uint8_t n);
void subtract_from_accumulator_with_carry(Cpu* cpu, uint8_t n);

//Opcode groupings

//0x
//TODO:
//1x
//TODO:
//2x
//TODO:
//3x
//TODO:
//4x
//TODO:
//5x
//TODO:
//6x
//TODO:
//7x
//TODO:
//8x

//0x80
void add_a_b(Cpu* cpu);
//0x81
void add_a_c(Cpu* cpu);
//0x82
void add_a_d(Cpu* cpu);
//0x83
void add_a_e(Cpu* cpu);
//0x84
void add_a_h(Cpu* cpu);
//0x85
void add_a_l(Cpu* cpu);
//0x86 (value in memory at location (h | l))
void add_a_hl(Cpu* cpu);
//0x87
void add_a_a(Cpu* cpu);
//0x88
void adc_a_b(Cpu* cpu);
//0x89
void adc_a_c(Cpu* cpu);
//0x8A
void adc_a_d(Cpu* cpu);
//0x8B
void adc_a_e(Cpu* cpu);
//0x8C
void adc_a_h(Cpu* cpu);
//0x8D
void adc_a_l(Cpu* cpu);
//0x8E
void adc_a_hl(Cpu* cpu);
//0x8F
void adc_a_a(Cpu* cpu);
//9x
//TODO:
//Ax
//TODO:
//Bx
//TODO:
//Cx
//TODO:
//0xC6
void add_a_8bit_constant(Cpu* cpu, uint8_t n);
//Dx
//TODO:
//Ex
//TODO:
//Fx
//TODO: