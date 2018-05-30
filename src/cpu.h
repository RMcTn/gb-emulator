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

void and_with_accumulator(Cpu* cpu, uint8_t n);
void xor_with_accumulator(Cpu* cpu, uint8_t n);

void or_with_accumulator(Cpu* cpu, uint8_t n);
void compare_with_accumulator(Cpu* cpu, uint8_t n);
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
//0x40
void ld_b_b(Cpu* cpu);
//0x41
void ld_b_c(Cpu* cpu);
//0x42
void ld_b_d(Cpu* cpu);
//0x43
void ld_b_e(Cpu* cpu);
//0x44
void ld_b_h(Cpu* cpu);
//0x45
void ld_b_l(Cpu* cpu);
//0x46
void ld_b_hl(Cpu* cpu);
//0x47
void ld_b_a(Cpu* cpu);
//0x48
void ld_c_b(Cpu* cpu);
//0x49
void ld_c_c(Cpu* cpu);
//0x4A
void ld_c_d(Cpu* cpu);
//0x4B
void ld_c_e(Cpu* cpu);
//0x4C
void ld_c_h(Cpu* cpu);
//0x4D
void ld_c_l(Cpu* cpu);
//0x4E
void ld_c_hl(Cpu* cpu);
//0x4F
void ld_c_a(Cpu* cpu);
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

//0x90
void sub_b(Cpu* cpu);
//0x91
void sub_c(Cpu* cpu);
//0x92
void sub_d(Cpu* cpu);
//0x93
void sub_e(Cpu* cpu);
//0x94
void sub_h(Cpu* cpu);
//0x95
void sub_l(Cpu* cpu);
//0x96
void sub_hl(Cpu* cpu);
//0x97
void sub_a(Cpu* cpu);
//0x98
void sbc_a_b(Cpu* cpu);
//0x99
void sbc_a_c(Cpu* cpu);
//0x9A
void sbc_a_d(Cpu* cpu);
//0x9B
void sbc_a_e(Cpu* cpu);
//0x9C
void sbc_a_h(Cpu* cpu);
//0x9D
void sbc_a_l(Cpu* cpu);
//0x9E
void sbc_a_hl(Cpu* cpu);
//0x9F
void sbc_a_a(Cpu* cpu);
//Ax
//0xA0
void and_b(Cpu* cpu);
//0xA1
void and_c(Cpu* cpu);
//0xA2
void and_d(Cpu* cpu);
//0xA3
void and_e(Cpu* cpu);
//0xA4
void and_h(Cpu* cpu);
//0xA5
void and_l(Cpu* cpu);
//0xA6
void and_hl(Cpu* cpu);
//0xA7
void and_a(Cpu* cpu);
//0xA8
void xor_b(Cpu* cpu);
//0xA9
void xor_c(Cpu* cpu);
//0xAA
void xor_d(Cpu* cpu);
//0xAB
void xor_e(Cpu* cpu);
//0xAC
void xor_h(Cpu* cpu);
//0xAD
void xor_l(Cpu* cpu);
//0xAE
void xor_hl(Cpu* cpu);
//0xAF
void xor_a(Cpu* cpu);
//Bx
//0xB0
void or_b(Cpu* cpu);
//0xB1
void or_c(Cpu* cpu);
//0xB2
void or_d(Cpu* cpu);
//0xB3
void or_e(Cpu* cpu);
//0xB4
void or_h(Cpu* cpu);
//0xB5
void or_l(Cpu* cpu);
//0xB6
void or_hl(Cpu* cpu);
//0xB7
void or_a(Cpu* cpu);
//0xB8
void cp_b(Cpu* cpu);
//0xB9
void cp_c(Cpu* cpu);
//0xBA
void cp_d(Cpu* cpu);
//0xBB
void cp_e(Cpu* cpu);
//0xBC
void cp_h(Cpu* cpu);
//0xBD
void cp_l(Cpu* cpu);
//0xBE
void cp_hl(Cpu* cpu);
//0xBF
void cp_a(Cpu* cpu);
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