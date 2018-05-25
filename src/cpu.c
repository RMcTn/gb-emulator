#include <stdio.h>

#include "cpu.h" 

void set_flag(Cpu* cpu, int flag) {
	cpu->f |= flag;
}

void clear_flag(Cpu* cpu, int flag) {
	cpu->f &= ~flag;
}

void add_a_8bit_constant(Cpu* cpu, uint8_t n) {
    set_flag(cpu, ZERO_FLAG);
    if (cpu->a + n > UINT8_MAX)
		set_flag(cpu, CARRY_FLAG);
    cpu->a += n;
    if (cpu->a == 0)
        set_flag(cpu, ZERO_FLAG);
    cpu->m = 2;
    cpu->t = 8;
}
void add_a_a(Cpu* cpu) {
    set_flag(cpu, ZERO_FLAG);
    if (cpu->a + cpu->a > UINT8_MAX)
		set_flag(cpu, CARRY_FLAG);
    cpu->a += cpu->a;
    if (cpu->a == 0)
		set_flag(cpu, ZERO_FLAG);
    cpu->m = 1;
    cpu->t = 4;
}

void add_a_b(Cpu* cpu) {
    set_flag(cpu, ZERO_FLAG);
    if (cpu->a + cpu->b > UINT8_MAX)
		set_flag(cpu, CARRY_FLAG);
    cpu->a += cpu->b;
    if (cpu->a == 0)
		set_flag(cpu, ZERO_FLAG);
    cpu->m = 1;
    cpu->t = 4;
}

void add_a_c(Cpu* cpu) {
    set_flag(cpu, ZERO_FLAG);
    if (cpu->a + cpu->c > UINT8_MAX)
        set_flag(cpu, CARRY_FLAG);
    cpu->a += cpu->c;
    if (cpu->a == 0)
        set_flag(cpu, ZERO_FLAG);
    cpu->m = 1;
    cpu->t = 4;
}

void add_a_d(Cpu* cpu) {
    set_flag(cpu, ZERO_FLAG);
    if (cpu->a + cpu->d > UINT8_MAX)
        set_flag(cpu, CARRY_FLAG);
    cpu->a += cpu->d;
    if (cpu->a == 0)
        set_flag(cpu, ZERO_FLAG);
    cpu->m = 1;
    cpu->t = 4;
}

void add_a_e(Cpu* cpu) {
    set_flag(cpu, ZERO_FLAG);
    if (cpu->a + cpu->e > UINT8_MAX)
        set_flag(cpu, CARRY_FLAG);
    cpu->a += cpu->e;
    if (cpu->a == 0)
        set_flag(cpu, ZERO_FLAG);
    cpu->m = 1;
    cpu->t = 4;
}

void add_a_h(Cpu* cpu) {
    set_flag(cpu, ZERO_FLAG);
    if (cpu->a + cpu->h > UINT8_MAX)
        set_flag(cpu, CARRY_FLAG);
    cpu->a += cpu->h;
    if (cpu->a == 0)
        set_flag(cpu, ZERO_FLAG);
    cpu->m = 1;
    cpu->t = 4;
}

void add_a_l(Cpu* cpu) {
    set_flag(cpu, ZERO_FLAG);
    if (cpu->a + cpu->l > UINT8_MAX)
        set_flag(cpu, CARRY_FLAG);
    cpu->a += cpu->l;
    if (cpu->a == 0)
        set_flag(cpu, ZERO_FLAG);
    cpu->m = 1;
    cpu->t = 4;
}