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
    cpu->f = 0;
    if (cpu->a + n > UINT8_MAX)
		set_flag(cpu, CARRY_FLAG);
    //Get first nibbles, then AND with 0x10 to check for overflow over 15
    if ((((cpu->a & 0xF) + (n & 0xF)) & 0x10) == 0x10)
        set_flag(cpu, HALFCARRY_FLAG);
    cpu->a += n;
    if (cpu->a == 0)
        set_flag(cpu, ZERO_FLAG);
}

void add_to_accumulator_with_carry(Cpu* cpu, uint8_t n) {
    if (cpu->f & CARRY_FLAG)
        add_to_accumulator(cpu, n + 1); //Add with carry
    else
        add_to_accumulator(cpu, n);
}

void subtract_from_accumulator(Cpu* cpu, uint8_t n) {
    cpu->f = 0;
    set_flag(cpu, SUBTRACTION_FLAG);
    if (cpu->a - n < 0)
        set_flag(cpu, CARRY_FLAG);
    if (((cpu->a & 0xF) - (n & 0xF)) < 0)
        set_flag(cpu, HALFCARRY_FLAG);

    cpu->a -= n;
    if (cpu->a == 0)
        set_flag(cpu, ZERO_FLAG);
    
}

void subtract_from_accumulator_with_carry(Cpu* cpu, uint8_t n) {
    if (cpu->f & CARRY_FLAG)
        subtract_from_accumulator(cpu, n + 1); //Subract with carry
    else
        subtract_from_accumulator(cpu, n);
}

void and_with_accumulator(Cpu* cpu, uint8_t n) {
    cpu->f = 0;
    set_flag(cpu, HALFCARRY_FLAG);
    cpu->a = cpu->a & n;
    if (cpu->a == 0)
        set_flag(cpu, ZERO_FLAG);
}

void xor_with_accumulator(Cpu* cpu, uint8_t n) {
    cpu->f = 0;
    cpu->a = cpu->a ^ n;
    if (cpu->a == 0)
        set_flag(cpu, ZERO_FLAG);
}
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
void add_a_b(Cpu* cpu) {
    add_to_accumulator(cpu, cpu->b);
    cpu->m = 1;
    cpu->t = 4;
}

//0x81
void add_a_c(Cpu* cpu) {
    add_to_accumulator(cpu, cpu->c);
    cpu->m = 1;
    cpu->t = 4;
}

//0x82
void add_a_d(Cpu* cpu) {
    add_to_accumulator(cpu, cpu->d);
    cpu->m = 1;
    cpu->t = 4;
}

//0x83
void add_a_e(Cpu* cpu) {
    add_to_accumulator(cpu, cpu->e);
    cpu->m = 1;
    cpu->t = 4;
}

//0x84
void add_a_h(Cpu* cpu) {
    add_to_accumulator(cpu, cpu->h);
    cpu->m = 1;
    cpu->t = 4;
}

//0x85
void add_a_l(Cpu* cpu) {
    add_to_accumulator(cpu, cpu->l);
    cpu->m = 1;
    cpu->t = 4;
}

//0x86
void add_a_hl(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t value = read_byte(cpu->memory, address);
    add_to_accumulator(cpu, value);
    cpu->m = 2;
    cpu->t = 8;
}
//0x87
void add_a_a(Cpu* cpu) {
    add_to_accumulator(cpu, cpu->a);
    cpu->m = 1;
    cpu->t = 4;
}

//0x88
void adc_a_b(Cpu* cpu) {
    add_to_accumulator_with_carry(cpu, cpu->b);
    cpu->m = 1;
    cpu->t = 4;
}

//0x89
void adc_a_c(Cpu* cpu) {
    add_to_accumulator_with_carry(cpu, cpu->c);
    cpu->m = 1;
    cpu->t = 4;
}

//0x8A
void adc_a_d(Cpu* cpu) {
    add_to_accumulator_with_carry(cpu, cpu->d);
    cpu->m = 1;
    cpu->t = 4;
}

//0x8B
void adc_a_e(Cpu* cpu) {
    add_to_accumulator_with_carry(cpu, cpu->e);
    cpu->m = 1;
    cpu->t = 4;
}

//0x8C
void adc_a_h(Cpu* cpu) {
    add_to_accumulator_with_carry(cpu, cpu->h);
    cpu->m = 1;
    cpu->t = 4;
}

//0x8D
void adc_a_l(Cpu* cpu) {
    add_to_accumulator_with_carry(cpu, cpu->l);
    cpu->m = 1;
    cpu->t = 4;
}

//0x8E
void adc_a_hl(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t value = read_byte(cpu->memory, address);
    add_to_accumulator_with_carry(cpu, value);
    cpu->m = 2;
    cpu->t = 8;
}

//0x8F
void adc_a_a(Cpu* cpu) {
    add_to_accumulator_with_carry(cpu, cpu->a);
    cpu->m = 1;
    cpu->t = 4;
}

//9x
//0x90
void sub_b(Cpu* cpu)  {
    subtract_from_accumulator(cpu, cpu->b);
    cpu->m = 1;
    cpu->t = 4;
}
//0x91
void sub_c(Cpu* cpu)  {
    subtract_from_accumulator(cpu, cpu->c);
    cpu->m = 1;
    cpu->t = 4;
}
//0x92
void sub_d(Cpu* cpu)  {
    subtract_from_accumulator(cpu, cpu->d);
    cpu->m = 1;
    cpu->t = 4;
}
//0x93
void sub_e(Cpu* cpu)  {
    subtract_from_accumulator(cpu, cpu->e);
    cpu->m = 1;
    cpu->t = 4;
}
//0x94
void sub_h(Cpu* cpu)  {
    subtract_from_accumulator(cpu, cpu->h);
    cpu->m = 1;
    cpu->t = 4;
}
//0x95
void sub_l(Cpu* cpu)  {
    subtract_from_accumulator(cpu, cpu->l);
    cpu->m = 1;
    cpu->t = 4;
}
//0x96
void sub_hl(Cpu* cpu)  {
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t value = read_byte(cpu->memory, address);
    subtract_from_accumulator(cpu, value);
    cpu->m = 2;
    cpu->t = 8;
}
//0x97
void sub_a(Cpu* cpu)  {
    subtract_from_accumulator(cpu, cpu->a);
    cpu->m = 1;
    cpu->t = 4;
}
//0x98
void sbc_a_b(Cpu* cpu)  {
    subtract_from_accumulator_with_carry(cpu, cpu->b);
    cpu->m = 1;
    cpu->t = 4;
}
//0x99
void sbc_a_c(Cpu* cpu)  {
    subtract_from_accumulator_with_carry(cpu, cpu->c);
    cpu->m = 1;
    cpu->t = 4;
    
}
//0x9A
void sbc_a_d(Cpu* cpu)  {
    subtract_from_accumulator_with_carry(cpu, cpu->d);
    cpu->m = 1;
    cpu->t = 4;
    
}
//0x9B
void sbc_a_e(Cpu* cpu)  {
    subtract_from_accumulator_with_carry(cpu, cpu->e);
    cpu->m = 1;
    cpu->t = 4;
    
}
//0x9C
void sbc_a_h(Cpu* cpu)  {
    subtract_from_accumulator_with_carry(cpu, cpu->h);
    cpu->m = 1;
    cpu->t = 4;
}
//0x9D
void sbc_a_l(Cpu* cpu)  {
    subtract_from_accumulator_with_carry(cpu, cpu->l);
    cpu->m = 1;
    cpu->t = 4;
}
//0x9E
void sbc_a_hl(Cpu* cpu)  {
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t value = read_byte(cpu->memory, address);
    subtract_from_accumulator_with_carry(cpu, value);
    cpu->m = 2;
    cpu->t = 8;
}
//0x9F
void sbc_a_a(Cpu* cpu)  {
    subtract_from_accumulator_with_carry(cpu, cpu->a);
    cpu->m = 1;
    cpu->t = 4;
}
//Ax
//0xA0
void and_b(Cpu* cpu) {
    and_with_accumulator(cpu, cpu->b);
    cpu->m = 1;
    cpu->t = 4;
}
//0xA1
void and_c(Cpu* cpu) {
    and_with_accumulator(cpu, cpu->c);
    cpu->m = 1;
    cpu->t = 4;
}
//0xA2
void and_d(Cpu* cpu) {
    and_with_accumulator(cpu, cpu->d);
    cpu->m = 1;
    cpu->t = 4;
}
//0xA3
void and_e(Cpu* cpu) {
    and_with_accumulator(cpu, cpu->e);
    cpu->m = 1;
    cpu->t = 4;
}
//0xA4
void and_h(Cpu* cpu) {
    and_with_accumulator(cpu, cpu->h);
    cpu->m = 1;
    cpu->t = 4;
}
//0xA5
void and_l(Cpu* cpu) {
    and_with_accumulator(cpu, cpu->l);
    cpu->m = 1;
    cpu->t = 4;
}
//0xA6
void and_hl(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t value = read_byte(cpu->memory, address);
    and_with_accumulator(cpu, value);
    cpu->m = 2;
    cpu->t = 8;
}
//0xA7
void and_a(Cpu* cpu) {
    and_with_accumulator(cpu, cpu->a);
    cpu->m = 1;
    cpu->t = 4;
}
//0xA8
void xor_b(Cpu* cpu) {
    xor_with_accumulator(cpu, cpu->b);
    cpu->m = 1;
    cpu->t = 4;
}
//0xA9
void xor_c(Cpu* cpu) {
    xor_with_accumulator(cpu, cpu->c);
    cpu->m = 1;
    cpu->t = 4;
}
//0xAA
void xor_d(Cpu* cpu) {
    xor_with_accumulator(cpu, cpu->d);
    cpu->m = 1;
    cpu->t = 4;
}
//0xAB
void xor_e(Cpu* cpu) {
    xor_with_accumulator(cpu, cpu->e);
    cpu->m = 1;
    cpu->t = 4;
}
//0xAC
void xor_h(Cpu* cpu) {
    xor_with_accumulator(cpu, cpu->h);
    cpu->m = 1;
    cpu->t = 4;
}
//0xAD
void xor_l(Cpu* cpu) {
    xor_with_accumulator(cpu, cpu->l);
    cpu->m = 1;
    cpu->t = 4;
}
//0xAE
void xor_hl(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t value = read_byte(cpu->memory, address);
    xor_with_accumulator(cpu, value);
    cpu->m = 2;
    cpu->t = 8;
}
//0xAF
void xor_a(Cpu* cpu) {
    xor_with_accumulator(cpu, cpu->a);
    cpu->m = 1;
    cpu->t = 4;
}
//TODO:
//Bx
//TODO:
//Cx
//TODO:
//0xC6
void add_a_8bit_constant(Cpu* cpu, uint8_t n) {
    add_to_accumulator(cpu, n);
    cpu->m = 2;
    cpu->t = 8;
}
//Dx
//TODO:
//Ex
//TODO:
//Fx
//TODO: