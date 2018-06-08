#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "gpu.h"
#include "memory.h"

struct Gpu;
typedef struct Cpu {
	uint8_t a;                      //Accumulator register
	uint8_t f;                      //Flag register (lower 4 bits are always 0)
	uint8_t b, c, d, e, h, l;       //8 bit registers 
	uint16_t sp, pc;               	//16 bit registers
	uint8_t m, t;					//clocks for last instruction
									//t increments with each clock step, m being a quarter of t
	int total_m, total_t;
	uint8_t memory[UINT16_MAX];		//16 bit address bus

	bool interrupt_master_enable;
	uint8_t interrupt_enable;		//Bits 1-4
	uint8_t interrupt_flags;		//Bits 1-4

	Gpu gpu;
} Cpu;

enum CpuFlags {
	CARRY_FLAG = 0x10,				/* Last operation result was over 255 (addition)
									or under 0 (subtraction) */
	HALFCARRY_FLAG = 0x20,			/* Last operation's result's lower half of
									byte overflowed past 15 */
	SUBTRACTION_FLAG = 0x40,		//Last operation was a subtraction
	ZERO_FLAG = 0x80,				//Last operation had result of 0
	ALL_FLAGS = CARRY_FLAG + HALFCARRY_FLAG + SUBTRACTION_FLAG + ZERO_FLAG
};

void print_cpu_contents();

void reset_cpu(Cpu* cpu);

uint16_t join_registers(uint8_t a, uint8_t b);
void write_to_16bit_registers(uint8_t* r1, uint8_t* r2, uint16_t value);

void set_flag(Cpu* cpu, int flag); 
void clear_flag(Cpu* cpu, int flag);
bool is_flag_set(Cpu* cpu, int flag);

void add_to_accumulator(Cpu* cpu, uint8_t n);
void add_to_accumulator_with_carry(Cpu* cpu, uint8_t n);

void add_to_16bit_register(Cpu* cpu, uint8_t* reg1, uint8_t* reg2, uint16_t n);

void subtract_from_accumulator(Cpu* cpu, uint8_t n);
void subtract_from_accumulator_with_carry(Cpu* cpu, uint8_t n);

void and_with_accumulator(Cpu* cpu, uint8_t n);
void xor_with_accumulator(Cpu* cpu, uint8_t n);

void or_with_accumulator(Cpu* cpu, uint8_t n);
void compare_with_accumulator(Cpu* cpu, uint8_t n);

void increment_8bit_register(Cpu* cpu, uint8_t* reg);
void decrement_8bit_register(Cpu* cpu, uint8_t* reg);

void increment_16bit_register(uint8_t* reg1, uint8_t* reg2);
void decrement_16bit_register(uint8_t* reg1, uint8_t* reg2);

void push_16bit_register(Cpu* cpu, uint8_t reg1, uint8_t reg2);
//Opcode groupings

//TODO: Put program counter increments into opcode functions(?)

void unimplemented_opcode(uint8_t opcode);
/*
	Two registers together lowercase means value at the 
	address held in those registers.
	ld_a_bc, load the value at address in bc, into a

	Two registers together UPPERCASE means the value held in 
	those registers.
	ld_HL_BC, load the value in bc, into registers hl 
*/

//0x
//0x00
void nop(Cpu* cpu);
//0x01
void ld_BC_16bit_immediate(Cpu* cpu, uint16_t n);
//0x02
void ld_bc_a(Cpu* cpu);
//0x03
void inc_BC(Cpu* cpu);
//0x04
void inc_b(Cpu* cpu);
//0x05
void dec_b(Cpu* cpu);
//0x06
void ld_b_8bit_immediate(Cpu* cpu, uint8_t n);
//0x07
void rlca(Cpu* cpu);
//0x08
void ld_16bit_address_sp(Cpu* cpu, uint16_t address);
//0x09
void add_HL_BC(Cpu* cpu);
//0x0A
void ld_a_bc(Cpu* cpu);
//0x0B
void dec_BC(Cpu* cpu);
//0x0C
void inc_c(Cpu* cpu);
//0x0D
void dec_c(Cpu* cpu);
//0x0E
void ld_c_8bit_immediate(Cpu* cpu, uint8_t n);
//0x0F
void rrca(Cpu* cpu);
//1x
//0x10
void stop(Cpu* cpu);
//0x11
void ld_DE_16bit_immediate(Cpu* cpu, uint16_t n);
//0x12
void ld_de_a(Cpu* cpu);
//0x13
void inc_DE(Cpu* cpu);
//0x14
void inc_d(Cpu* cpu);
//0x15
void dec_d(Cpu* cpu);
//0x16
void ld_d_8bit_immediate(Cpu* cpu, uint8_t n);
//0x17
void rla(Cpu* cpu);
//0x18
void jr_8bit_immediate(Cpu* cpu, int8_t n);
//0x19
void add_HL_DE(Cpu* cpu);
//0x1A
void ld_a_de(Cpu* cpu);
//0x1B
void dec_DE(Cpu* cpu);
//0x1C
void inc_e(Cpu* cpu);
//0x1D
void dec_e(Cpu* cpu);
//0x1E
void ld_e_8bit_immediate(Cpu* cpu, uint8_t n);
//0x1F
void rra(Cpu* cpu);
//2x
//0x20
void jr_nz_8bit_immediate(Cpu* cpu, int8_t n);
//0x21
void ld_HL_16bit_immediate(Cpu* cpu, uint16_t n);
//0x22
void ld_hlincrement_a(Cpu* cpu);
//0x23
void inc_HL(Cpu* cpu);
//0x24
void inc_h(Cpu* cpu);
//0x25
void dec_h(Cpu* cpu);
//0x26
void ld_h_8bit_immediate(Cpu* cpu, uint8_t n);
//0x27
void daa(Cpu* cpu);
//0x28
void jr_z_8bit_immediate(Cpu* cpu, int8_t n);
//0x29
void add_HL_HL(Cpu* cpu);
//0x2A
void ld_a_hlincrement(Cpu* cpu);
//0x2B
void dec_HL(Cpu* cpu);
//0x2C
void inc_l(Cpu* cpu);
//0x2D
void dec_l(Cpu* cpu);
//0x2E
void ld_l_8bit_immediate(Cpu* cpu, uint8_t n);
//0x2F
void cpl(Cpu* cpu);
//3x
//0x30
void jr_nc_8bit_immediate(Cpu* cpu, int8_t n);
//0x31
void ld_sp_16bit_immediate(Cpu* cpu, uint16_t n);
//0x32
void ld_hldecrement_a(Cpu* cpu);
//0x33
void inc_sp(Cpu* cpu);
//0x34
void inc_hl(Cpu* cpu);
//0x35
void dec_hl(Cpu* cpu);
//0x36
void ld_hl_8bit_immediate(Cpu* cpu, uint8_t n);
//0x37
void scf(Cpu* cpu);
//0x38
void jr_c_8bit_immediate(Cpu* cpu, int8_t n);
//0x39
void add_HL_sp(Cpu* cpu);
//0x3A
void ld_a_hldecrement(Cpu* cpu);
//0x3B
void dec_sp(Cpu* cpu);
//0x3C
void inc_a(Cpu* cpu);
//0x3D
void dec_a(Cpu* cpu);
//0x3E
void ld_a_8bit_immediate(Cpu* cpu, uint8_t n);
//0x3F
void ccf(Cpu* cpu);
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
//0x50
void ld_d_b(Cpu* cpu);
//0x51
void ld_d_c(Cpu* cpu);
//0x52
void ld_d_d(Cpu* cpu);
//0x53
void ld_d_e(Cpu* cpu);
//0x54
void ld_d_h(Cpu* cpu);
//0x55
void ld_d_l(Cpu* cpu);
//0x56
void ld_d_hl(Cpu* cpu);
//0x57
void ld_d_a(Cpu* cpu);
//0x58
void ld_e_b(Cpu* cpu);
//0x59
void ld_e_c(Cpu* cpu);
//0x5A
void ld_e_d(Cpu* cpu);
//0x5B
void ld_e_e(Cpu* cpu);
//0x5C
void ld_e_h(Cpu* cpu);
//0x5D
void ld_e_l(Cpu* cpu);
//0x5E
void ld_e_hl(Cpu* cpu);
//0x5F
void ld_e_a(Cpu* cpu);
//6x
//0x60
void ld_h_b(Cpu* cpu);
//0x61
void ld_h_c(Cpu* cpu);
//0x62
void ld_h_d(Cpu* cpu);
//0x63
void ld_h_e(Cpu* cpu);
//0x64
void ld_h_h(Cpu* cpu);
//0x65
void ld_h_l(Cpu* cpu);
//0x66
void ld_h_hl(Cpu* cpu);
//0x67
void ld_h_a(Cpu* cpu);
//0x68
void ld_l_b(Cpu* cpu);
//0x69
void ld_l_c(Cpu* cpu);
//0x6A
void ld_l_d(Cpu* cpu);
//0x6B
void ld_l_e(Cpu* cpu);
//0x6C
void ld_l_h(Cpu* cpu);
//0x6D
void ld_l_l(Cpu* cpu);
//0x6E
void ld_l_hl(Cpu* cpu);
//0x6F
void ld_l_a(Cpu* cpu);
//7x
//0x70
void ld_hl_b(Cpu* cpu);
//0x71
void ld_hl_c(Cpu* cpu);
//0x72
void ld_hl_d(Cpu* cpu);
//0x73
void ld_hl_e(Cpu* cpu);
//0x74
void ld_hl_h(Cpu* cpu);
//0x75
void ld_hl_l(Cpu* cpu);
//0x76
void halt(Cpu* cpu); //TODO
//0x77
void ld_hl_a(Cpu* cpu);
//0x78
void ld_a_b(Cpu* cpu);
//0x79
void ld_a_c(Cpu* cpu);
//0x7A
void ld_a_d(Cpu* cpu);
//0x7B
void ld_a_e(Cpu* cpu);
//0x7C
void ld_a_h(Cpu* cpu);
//0x7D
void ld_a_l(Cpu* cpu);
//0x7E
void ld_a_hl(Cpu* cpu);
//0x7F
void ld_a_a(Cpu* cpu);
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
//0xC0
void ret_nz(Cpu* cpu);
//0xC2
void jp_nz_16bit_immediate(Cpu* cpu, uint16_t n);
//0xC3
void jp_16bit_immediate(Cpu* cpu, uint16_t n);
//0xC5
void push_BC(Cpu* cpu);
//0xC6
void add_a_8bit_immediate(Cpu* cpu, uint8_t n);
//0xC8
void ret_z(Cpu* cpu);
//0xCA
void jp_z_16bit_immediate(Cpu* cpu, uint16_t n);
//0xCB
void prefix_cb(Cpu* cpu, uint8_t opcode);
//0xCD
void call_16bit_immediate(Cpu* cpu, uint16_t n);
//Dx
//TODO:
//0xD2
void jp_nc_16bit_immediate(Cpu* cpu, uint16_t n);
//0xD5
void push_DE(Cpu* cpu);
//0xD8
void ret_c(Cpu* cpu);
//0xD9
void reti(Cpu* cpu);
//0xDA
void jp_c_16bit_immediate(Cpu* cpu, uint16_t n);
//Ex
//TODO:
//0xE0
void ldh_8bit_immediate_a(Cpu* cpu, uint8_t n);
//0xE5
void push_HL(Cpu* cpu);
//0xE9
void jp_hl(Cpu* cpu);
//Fx
//TODO:
//0xF0
void ldh_a_8bit_immediate(Cpu* cpu, uint8_t n);
//0xF3
void di(Cpu* cpu);
//0xF5
void push_AF(Cpu* cpu);
//0xFA
void ld_a_16bit_address(Cpu* cpu, uint16_t address);
//0xFB
void ei(Cpu* cpu);
//0xFE
void cp_8bit_immediate(Cpu* cpu, uint8_t n);

//Vblank interrupt
void rst_40(Cpu* cpu);