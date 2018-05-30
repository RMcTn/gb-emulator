#include <stdio.h>

#include "cpu.h" 

void print_cpu_contents(Cpu* cpu) {
    printf("REGISTERS\n");
    printf("A: %d (0x%hhX), ", cpu->a,  cpu->a);
    printf("B: %d (0x%hhX), ", cpu->b,  cpu->b);
    printf("C: %d (0x%hhX), ", cpu->c,  cpu->c);
    printf("D: %d (0x%hhX), ", cpu->d,  cpu->d);
    printf("E: %d (0x%hhX), ", cpu->e,  cpu->e);
    printf("H: %d (0x%hhX), ", cpu->h,  cpu->h);
    printf("L: %d (0x%hhX), ", cpu->l,  cpu->l);
    printf("SP: %d (0x%hhX), ", cpu->sp,  cpu->sp);
    printf("PC: %d (0x%hhX)\n", cpu->pc,  cpu->pc);
    printf("CLOCKS\n");
    printf("T: %d, M: %d\n", cpu->t, cpu->m);

    printf("FLAGS\n");
    printf("Zero: %d ", cpu->f & ZERO_FLAG ? 1 : 0);
    printf("Subtraction: %d ", cpu->f & SUBTRACTION_FLAG ? 1 : 0);
    printf("Halfcarry: %d ", cpu->f & HALFCARRY_FLAG ? 1 : 0);
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

void or_with_accumulator(Cpu* cpu, uint8_t n) {
    cpu->f = 0;
    cpu->a = cpu->a | n;
    if (cpu->a == 0)
        set_flag(cpu, ZERO_FLAG);
}

/*
    Zero flag set if equal
    Halfcarry flag set if accumulator > n
    Carry flag set if accumulator < n
*/
void compare_with_accumulator(Cpu* cpu, uint8_t n) {
    cpu->f = 0;
    set_flag(cpu, SUBTRACTION_FLAG);
    if (cpu->a == n)
        set_flag(cpu, ZERO_FLAG);
    if (cpu->a > n)
        set_flag(cpu, HALFCARRY_FLAG);
    if (cpu->a < n)
        set_flag(cpu, CARRY_FLAG);
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
//0x40
void ld_b_b(Cpu* cpu) {
    cpu->b = cpu->b;
    cpu->m = 1;
    cpu->t = 4;
}
//0x41
void ld_b_c(Cpu* cpu) {
    cpu->b = cpu->c;
    cpu->m = 1;
    cpu->t = 4;
}
//0x42
void ld_b_d(Cpu* cpu) {
    cpu->b = cpu->d;
    cpu->m = 1;
    cpu->t = 4;
}
//0x43
void ld_b_e(Cpu* cpu) {
    cpu->b = cpu->e;
    cpu->m = 1;
    cpu->t = 4;
}
//0x44
void ld_b_h(Cpu* cpu) {
    cpu->b = cpu->h;
    cpu->m = 1;
    cpu->t = 4;
}
//0x45
void ld_b_l(Cpu* cpu) {
    cpu->b = cpu->l;
    cpu->m = 1;
    cpu->t = 4;
}
//0x46
void ld_b_hl(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t value = read_byte(cpu->memory, address);
    cpu->b = value;
    cpu->m = 2;
    cpu->t = 8;
}
//0x47
void ld_b_a(Cpu* cpu) {
    cpu->b = cpu->a;
    cpu->m = 1;
    cpu->t = 4;
}
//0x48
void ld_c_b(Cpu* cpu) {
    cpu->c = cpu->b;
    cpu->m = 1;
    cpu->t = 4;
}
//0x49
void ld_c_c(Cpu* cpu) {
    cpu->c = cpu->c;
    cpu->m = 1;
    cpu->t = 4;
}
//0x4A
void ld_c_d(Cpu* cpu) {
    cpu->c = cpu->d;
    cpu->m = 1;
    cpu->t = 4;
}
//0x4B
void ld_c_e(Cpu* cpu) {
    cpu->c = cpu->e;
    cpu->m = 1;
    cpu->t = 4;
}
//0x4C
void ld_c_h(Cpu* cpu) {
    cpu->c = cpu->h;
    cpu->m = 1;
    cpu->t = 4;
}
//0x4D
void ld_c_l(Cpu* cpu) {
    cpu->c = cpu->l;
    cpu->m = 1;
    cpu->t = 4;
}
//0x4E
void ld_c_hl(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t value = read_byte(cpu->memory, address);
    cpu->c = value;
    cpu->m = 2;
    cpu->t = 8;
}
//0x4F
void ld_c_a(Cpu* cpu) {
    cpu->c = cpu->a;
    cpu->m = 1;
    cpu->t = 4;
}
//5x
//0x50
void ld_d_b(Cpu* cpu) {
    cpu->d = cpu->b;
    cpu->m = 1;
    cpu->t = 4;
}
//0x51
void ld_d_c(Cpu* cpu) {
    cpu->d = cpu->c;
    cpu->m = 1;
    cpu->t = 4;
}
//0x52
void ld_d_d(Cpu* cpu) {
    cpu->d = cpu->d;
    cpu->m = 1;
    cpu->t = 4;
}
//0x53
void ld_d_e(Cpu* cpu) {
    cpu->d = cpu->e;
    cpu->m = 1;
    cpu->t = 4;
}
//0x54
void ld_d_h(Cpu* cpu) {
    cpu->d = cpu->h;
    cpu->m = 1;
    cpu->t = 4;
}
//0x55
void ld_d_l(Cpu* cpu) {
    cpu->d = cpu->l;
    cpu->m = 1;
    cpu->t = 4;
}
//0x56
void ld_d_hl(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t value = read_byte(cpu->memory, address);
    cpu->d = value;
    cpu->m = 2;
    cpu->t = 8;
}
//0x57
void ld_d_a(Cpu* cpu) {
    cpu->d = cpu->a;
    cpu->m = 1;
    cpu->t = 4;
}
//0x58
void ld_e_b(Cpu* cpu) {
    cpu->e = cpu->b;
    cpu->m = 1;
    cpu->t = 4;
}
//0x59
void ld_e_c(Cpu* cpu) {
    cpu->e = cpu->c;
    cpu->m = 1;
    cpu->t = 4;
}
//0x5A
void ld_e_d(Cpu* cpu) {
    cpu->e = cpu->d;
    cpu->m = 1;
    cpu->t = 4;
}
//0x5B
void ld_e_e(Cpu* cpu) {
    cpu->e = cpu->e;
    cpu->m = 1;
    cpu->t = 4;
}
//0x5C
void ld_e_h(Cpu* cpu) {
    cpu->e = cpu->h;
    cpu->m = 1;
    cpu->t = 4;
}
//0x5D
void ld_e_l(Cpu* cpu) {
    cpu->e = cpu->l;
    cpu->m = 1;
    cpu->t = 4;
}
//0x5E
void ld_e_hl(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t value = read_byte(cpu->memory, address);
    cpu->e = value;
    cpu->m = 2;
    cpu->t = 8;
}
//0x5F
void ld_e_a(Cpu* cpu) {
    cpu->e = cpu->a;
    cpu->m = 1;
    cpu->t = 4;
}
//6x
//0x60
void ld_h_b(Cpu* cpu) {
    cpu->h = cpu->b;
    cpu->m = 1;
    cpu->t = 4;
}
//0x61
void ld_h_c(Cpu* cpu) {
    cpu->h = cpu->c;
    cpu->m = 1;
    cpu->t = 4;
}
//0x62
void ld_h_d(Cpu* cpu) {
    cpu->h = cpu->d;
    cpu->m = 1;
    cpu->t = 4;
}
//0x63
void ld_h_e(Cpu* cpu) {
    cpu->h = cpu->e;
    cpu->m = 1;
    cpu->t = 4;
}
//0x64
void ld_h_h(Cpu* cpu) {
    cpu->h = cpu->h;
    cpu->m = 1;
    cpu->t = 4;
}
//0x65
void ld_h_l(Cpu* cpu) {
    cpu->h = cpu->l;
    cpu->m = 1;
    cpu->t = 4;
}
//0x66
void ld_h_hl(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t value = read_byte(cpu->memory, address);
    cpu->h = value;
    cpu->m = 2;
    cpu->t = 8;
}
//0x67
void ld_h_a(Cpu* cpu) {
    cpu->h = cpu->a;
    cpu->m = 1;
    cpu->t = 4;
}
//0x68
void ld_l_b(Cpu* cpu) {
    cpu->l = cpu->b;
    cpu->m = 1;
    cpu->t = 4;
}
//0x69
void ld_l_c(Cpu* cpu) {
    cpu->l = cpu->c;
    cpu->m = 1;
    cpu->t = 4;
}
//0x6A
void ld_l_d(Cpu* cpu) {
    cpu->l = cpu->d;
    cpu->m = 1;
    cpu->t = 4;
}
//0x6B
void ld_l_e(Cpu* cpu) {
    cpu->l = cpu->e;
    cpu->m = 1;
    cpu->t = 4;
}
//0x6C
void ld_l_h(Cpu* cpu) {
    cpu->l = cpu->h;
    cpu->m = 1;
    cpu->t = 4;
}
//0x6D
void ld_l_l(Cpu* cpu) {
    cpu->l = cpu->l;
    cpu->m = 1;
    cpu->t = 4;
}
//0x6E
void ld_l_hl(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t value = read_byte(cpu->memory, address);
    cpu->l = value;
    cpu->m = 2;
    cpu->t = 8;
}
//0x6F
void ld_l_a(Cpu* cpu) {
    cpu->l = cpu->a;
    cpu->m = 1;
    cpu->t = 4;
}
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
//0xB0
void or_b(Cpu* cpu) {
    or_with_accumulator(cpu, cpu->b);
    cpu->m = 1;
    cpu->t = 4;
}
//0xB1
void or_c(Cpu* cpu) {
    or_with_accumulator(cpu, cpu->c);
    cpu->m = 1;
    cpu->t = 4;
}
//0xB2
void or_d(Cpu* cpu) {
    or_with_accumulator(cpu, cpu->d);
    cpu->m = 1;
    cpu->t = 4;
}
//0xB3
void or_e(Cpu* cpu) {
    or_with_accumulator(cpu, cpu->e);
    cpu->m = 1;
    cpu->t = 4;
}
//0xB4
void or_h(Cpu* cpu) {
    or_with_accumulator(cpu, cpu->h);
    cpu->m = 1;
    cpu->t = 4;
}
//0xB5
void or_l(Cpu* cpu) {
    or_with_accumulator(cpu, cpu->l);
    cpu->m = 1;
    cpu->t = 4;
}
//0xB6
void or_hl(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t value = read_byte(cpu->memory, address);
    or_with_accumulator(cpu, value);
    cpu->m = 2;
    cpu->t = 8;
}
//0xB7
void or_a(Cpu* cpu) {
    or_with_accumulator(cpu, cpu->a);
    cpu->m = 1;
    cpu->t = 4;
}
//0xB8
void cp_b(Cpu* cpu) {
    compare_with_accumulator(cpu, cpu->b);
    cpu->m = 1;
    cpu->t = 4;
}
//0xB9
void cp_c(Cpu* cpu) {
    compare_with_accumulator(cpu, cpu->c);
    cpu->m = 1;
    cpu->t = 4;
}
//0xBA
void cp_d(Cpu* cpu) {
    compare_with_accumulator(cpu, cpu->d);
    cpu->m = 1;
    cpu->t = 4;
}
//0xBB
void cp_e(Cpu* cpu) {
    compare_with_accumulator(cpu, cpu->e);
    cpu->m = 1;
    cpu->t = 4;
}
//0xBC
void cp_h(Cpu* cpu) {
    compare_with_accumulator(cpu, cpu->h);
    cpu->m = 1;
    cpu->t = 4;
}
//0xBD
void cp_l(Cpu* cpu) {
    compare_with_accumulator(cpu, cpu->l);
    cpu->m = 1;
    cpu->t = 4;
}
//0xBE
void cp_hl(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t value = read_byte(cpu->memory, address);
    compare_with_accumulator(cpu, value);
    cpu->m = 2;
    cpu->t = 8;
}
//0xBF
void cp_a(Cpu* cpu) {
    compare_with_accumulator(cpu, cpu->a);
    cpu->m = 1;
    cpu->t = 4;
}
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