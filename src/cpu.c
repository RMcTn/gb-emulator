#include <stdio.h>

#include "cpu.h" 

void set_flag(Cpu* cpu, int flag) {
	cpu->f |= flag;
}

void clear_flag(Cpu* cpu, int flag) {
	cpu->f &= ~flag;
}

void add_to_accumulator(Cpu* cpu, uint8_t n) {
    set_flag(cpu, SUBTRACTION_FLAG);
    if (cpu->a + n > UINT8_MAX)
		set_flag(cpu, CARRY_FLAG);
    cpu->a += n;
    if (cpu->a == 0)
        set_flag(cpu, ZERO_FLAG);
}

void add_a_8bit_constant(Cpu* cpu, uint8_t n) {
    add_to_accumulator(cpu, n);
    cpu->m = 2;
    cpu->t = 8;
}
void add_a_a(Cpu* cpu) {
    add_to_accumulator(cpu, cpu->a);
    cpu->m = 1;
    cpu->t = 4;
}

void add_a_b(Cpu* cpu) {
    add_to_accumulator(cpu, cpu->b);
    cpu->m = 1;
    cpu->t = 4;
}

void add_a_c(Cpu* cpu) {
    add_to_accumulator(cpu, cpu->c);
    cpu->m = 1;
    cpu->t = 4;
}

void add_a_d(Cpu* cpu) {
    add_to_accumulator(cpu, cpu->d);
    cpu->m = 1;
    cpu->t = 4;
}

void add_a_e(Cpu* cpu) {
    add_to_accumulator(cpu, cpu->e);
    cpu->m = 1;
    cpu->t = 4;
}

void add_a_h(Cpu* cpu) {
    add_to_accumulator(cpu, cpu->h);
    cpu->m = 1;
    cpu->t = 4;
}

void add_a_l(Cpu* cpu) {
    add_to_accumulator(cpu, cpu->l);
    cpu->m = 1;
    cpu->t = 4;
}