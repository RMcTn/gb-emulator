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

//Functions underneath here could probably be static
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

void loop(Cpu* cpu);
