#include <stdio.h>

#include "cpu.h" 

void print_cpu_contents(Cpu* cpu) {
    printf("REGISTERS\n");
    printf("A: %d, B: %d, C: %d, D: %d, E: %d, H: %d, L: %d, SP: %d, PC: %d\n", cpu->a, cpu->b, cpu->c,
                                                              cpu->d, cpu->e, cpu->h, cpu->l, cpu->sp, cpu->pc);
    printf("CLOCKS\n");
    printf("T: %d, M: %d\n", cpu->t, cpu->m);

    printf("FLAGS\n");
    printf("Zero: %d\n", cpu->f & ZERO_FLAG ? 1 : 0);
    printf("Subtraction: %d\n", cpu->f & SUBTRACTION_FLAG ? 1 : 0);
    printf("Halfcarry: %d\n", cpu->f & HALFCARRY_FLAG ? 1 : 0);
    printf("Carry: %d\n", cpu->f & CARRY_FLAG ? 1 : 0);
    
}

void set_flag(Cpu* cpu, int flag) {
	cpu->f |= flag;
}

void clear_flag(Cpu* cpu, int flag) {
	cpu->f &= ~flag;
}

void add_to_accumulator(Cpu* cpu, uint8_t n) {
    clear_flag(cpu, SUBTRACTION_FLAG);
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