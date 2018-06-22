#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cpu.h" 
#include "gpu.h"

void print_cpu_contents(Cpu* cpu) {
    printf("REGISTERS\n");
    printf("A: %d (0x%hhX), ", cpu->a,  cpu->a);
    printf("B: %d (0x%hhX), ", cpu->b,  cpu->b);
    printf("C: %d (0x%hhX), ", cpu->c,  cpu->c);
    printf("D: %d (0x%hhX), ", cpu->d,  cpu->d);
    printf("E: %d (0x%hhX), ", cpu->e,  cpu->e);
    printf("H: %d (0x%hhX), ", cpu->h,  cpu->h);
    printf("L: %d (0x%hhX), ", cpu->l,  cpu->l);
    printf("SP: %d (0x%hX), ", cpu->sp,  cpu->sp);
    printf("PC: %d (0x%hX)\n", cpu->pc,  cpu->pc);
    printf("CLOCKS\n");
    printf("T: %d, M: %d\n", cpu->t, cpu->m);
    printf("Total T: %d, Total M: %d\n", cpu->total_t, cpu->total_m);
    

    printf("FLAGS\n");
    printf("Zero: %d ", cpu->f & ZERO_FLAG ? 1 : 0);
    printf("Subtraction: %d ", cpu->f & SUBTRACTION_FLAG ? 1 : 0);
    printf("Halfcarry: %d ", cpu->f & HALFCARRY_FLAG ? 1 : 0);
    printf("Carry: %d\n", cpu->f & CARRY_FLAG ? 1 : 0);
    
}

void reset_cpu(Cpu* cpu) {
    cpu->a = 0;
    cpu->b = 0;
    cpu->c = 0;
    cpu->d = 0;
    cpu->e = 0;
    cpu->h = 0;
    cpu->l = 0;
    cpu->sp = 0;
    cpu->pc = 0;

    cpu->m = 0;
    cpu->t = 0;
    cpu->total_m = 0;
    cpu->total_t = 0;

    memset(cpu->memory, 0, MEMORY_SIZE);

    cpu->interrupt_master_enable = true;
    cpu->interrupt_enable = 0xF;
    cpu->interrupt_flags = 0;
    reset_gpu(&cpu->gpu);
    
}

uint16_t join_registers(uint8_t a, uint8_t b) {
    return (a << 8) | b;
};

void write_to_16bit_registers(uint8_t* r1, uint8_t* r2, uint16_t n) {
    *r1 = n >> 8;        //Top 8 bits
    *r2 = (uint8_t)n;    //Bottom 8 bits
}

void set_flag(Cpu* cpu, int flag) {
	cpu->f |= flag;
}

void clear_flag(Cpu* cpu, int flag) {
	cpu->f &= ~flag;
}

bool is_flag_set(Cpu* cpu, int flag) {
    return cpu->f & flag;
}

void add_to_accumulator(Cpu* cpu, uint8_t n) {
    clear_flag(cpu, ALL_FLAGS);
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

void add_to_16bit_register(Cpu* cpu, uint8_t* reg1, uint8_t* reg2, uint16_t n) {
    clear_flag(cpu, SUBTRACTION_FLAG);
    clear_flag(cpu, CARRY_FLAG);
    clear_flag(cpu, HALFCARRY_FLAG);
    uint16_t reg_value = join_registers(*reg1, *reg2);
    if (n + reg_value > UINT16_MAX)
        set_flag(cpu, CARRY_FLAG);
    //Check bit 11 for overflow
    if ((((reg_value & 0xF00) + (n & 0xF00)) & 0x800) == 0x800)
        set_flag(cpu, HALFCARRY_FLAG);
    reg_value += n;
    write_to_16bit_registers(reg1, reg2, reg_value);
}

void subtract_from_accumulator(Cpu* cpu, uint8_t n) {
    clear_flag(cpu, ALL_FLAGS);
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
    clear_flag(cpu, ALL_FLAGS);
    set_flag(cpu, HALFCARRY_FLAG);
    cpu->a = cpu->a & n;
    if (cpu->a == 0)
        set_flag(cpu, ZERO_FLAG);
}

void xor_with_accumulator(Cpu* cpu, uint8_t n) {
    clear_flag(cpu, ALL_FLAGS);
    cpu->a = cpu->a ^ n;
    if (cpu->a == 0)
        set_flag(cpu, ZERO_FLAG);
}

void or_with_accumulator(Cpu* cpu, uint8_t n) {
    clear_flag(cpu, ALL_FLAGS);
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
    clear_flag(cpu, ALL_FLAGS);
    set_flag(cpu, SUBTRACTION_FLAG);
    if (cpu->a == n)
        set_flag(cpu, ZERO_FLAG);
    if (cpu->a > n)
        set_flag(cpu, HALFCARRY_FLAG);
    if (cpu->a < n)
        set_flag(cpu, CARRY_FLAG);
}

void increment_8bit_register(Cpu* cpu, uint8_t* reg) {
    clear_flag(cpu, ZERO_FLAG);
    clear_flag(cpu, SUBTRACTION_FLAG);
    clear_flag(cpu, HALFCARRY_FLAG);
    *reg = *reg + 1;
    if (*reg == 0)
        set_flag(cpu, ZERO_FLAG);
    //Check 4th bit of incremented value and non incremented value
    if ((((*reg & 0xF) + ((*reg - 1) & 0xF)) & 0x10) == 0x10)
        set_flag(cpu, HALFCARRY_FLAG);

}

void decrement_8bit_register(Cpu* cpu, uint8_t* reg) {
    clear_flag(cpu, ZERO_FLAG);
    set_flag(cpu, SUBTRACTION_FLAG);
    clear_flag(cpu, HALFCARRY_FLAG);
    *reg = *reg - 1;
    if (*reg == 0)
        set_flag(cpu, ZERO_FLAG);
    //Check 4th bit of incremented value and non incremented value
    if (((*reg & 0xF) - ((*reg - 1) & 0xF)) < 0)
        set_flag(cpu, HALFCARRY_FLAG);
}

void increment_16bit_register(uint8_t* reg1, uint8_t* reg2) {
    //No flags changed here
    uint16_t value = join_registers(*reg1, *reg2);
    value++;
    write_to_16bit_registers(reg1, reg2, value);
}

void decrement_16bit_register(uint8_t* reg1, uint8_t* reg2) {
    //No flags changed here
    uint16_t value = join_registers(*reg1, *reg2);
    value--;
    write_to_16bit_registers(reg1, reg2, value);
}

void push_16bit_register(Cpu* cpu, uint8_t reg1, uint8_t reg2) {
    cpu->sp -= 2;
    uint16_t value = join_registers(reg1, reg2);
    write_word(cpu, cpu->sp, value);
}

//Opcode groupings
void unimplemented_opcode(uint8_t opcode) {
    printf("Opcode not implemented: %hhX\n", opcode);
    exit(1);
}
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
void nop(Cpu* cpu) {
    //No operation
    cpu->m = 1;
    cpu->t = 4;
}
//0x01
void ld_BC_16bit_immediate(Cpu* cpu, uint16_t n) {
    write_to_16bit_registers(&cpu->b, &cpu->c, n);
    cpu->m = 3;
    cpu->t = 12;
}
//0x02
void ld_bc_a(Cpu* cpu) {
    uint16_t address = join_registers(cpu->b, cpu->c); 
    write_byte(cpu, address, cpu->a);
    cpu->m = 2;
    cpu->t = 8;
}
//0x03
void inc_BC(Cpu* cpu) {
    increment_16bit_register(&cpu->b, &cpu->c);
    cpu->m = 2;
    cpu->t = 8;
}
//0x04
void inc_b(Cpu* cpu) {
    increment_8bit_register(cpu, &cpu->b);
    cpu->m = 1;
    cpu->t = 4;
}
//0x05
void dec_b(Cpu* cpu) {
    decrement_8bit_register(cpu, &cpu->b);
    cpu->m = 1;
    cpu->t = 4;
}
//0x06
void ld_b_8bit_immediate(Cpu* cpu, uint8_t n) {
    cpu->b = n;
    cpu->m = 2;
    cpu->t = 8;
}
//0x07
void rlca(Cpu* cpu) {
    clear_flag(cpu, ALL_FLAGS);
    if ((cpu->a << 1) > UINT8_MAX)
        set_flag(cpu, CARRY_FLAG);
    cpu->a = cpu->a << 1;
    //Shift is meant to move 7th bit to 0th bit if overflowing
    if (is_flag_set(cpu, CARRY_FLAG))
        cpu->a = cpu->a | 0x1;     
    cpu->m = 1;
    cpu->t = 4;
}
//0x08
void ld_16bit_address_sp(Cpu* cpu, uint16_t address) {
    write_byte(cpu, address, cpu->sp);
    cpu->m = 5;
    cpu->t = 20;
}
//0x09
void add_HL_BC(Cpu* cpu) {
    uint16_t bc_value = join_registers(cpu->b, cpu->c);
    add_to_16bit_register(cpu, &cpu->h, &cpu->l, bc_value);
    cpu->m = 2;
    cpu->t = 8;
}
//0x0A
void ld_a_bc(Cpu* cpu) {
    uint16_t address = join_registers(cpu->b, cpu->c);
    cpu->a = read_byte(cpu, address);
    cpu->m = 2;
    cpu->t = 8;
}
//0x0B
void dec_BC(Cpu* cpu) {
    decrement_16bit_register(&cpu->b, &cpu->c);
    cpu->m = 2;
    cpu->t = 8;
}
//0x0C
void inc_c(Cpu* cpu) {
    increment_8bit_register(cpu, &cpu->c);
    cpu->m = 1;
    cpu->t = 4;
}
//0x0D
void dec_c(Cpu* cpu) {
    decrement_8bit_register(cpu, &cpu->c);
    cpu->m = 1;
    cpu->t = 4;
}
//0x0E
void ld_c_8bit_immediate(Cpu* cpu, uint8_t n) {
    cpu->c = n;
    cpu->m = 2;
    cpu->t = 8;
}
//0x0F
void rrca(Cpu* cpu) {
    clear_flag(cpu, ALL_FLAGS);
    //If the bottom bit is set, there's going to be a carry
    if (cpu->a & 0x1)
        set_flag(cpu, CARRY_FLAG);
    cpu->a = cpu->a >> 1;
    //Shift is meant to move 0th bit to 7th bit if overflowing
    if (is_flag_set(cpu, CARRY_FLAG))
        cpu->a = cpu->a | 0x80;
    cpu->m = 1;
    cpu->t = 4;
}
//1x
//0x10
//TODO
//0x11
void ld_DE_16bit_immediate(Cpu* cpu, uint16_t n) {
    write_to_16bit_registers(&cpu->d, &cpu->e, n);
    cpu->m = 3;
    cpu->t = 12;
}
//0x12
void ld_de_a(Cpu* cpu) {
    uint16_t address = join_registers(cpu->d, cpu->e); 
    write_byte(cpu, address, cpu->a);
    cpu->m = 2;
    cpu->t = 8;
}
//0x13
void inc_DE(Cpu* cpu) {
    increment_16bit_register(&cpu->d, &cpu->e);
    cpu->m = 2;
    cpu->t = 8;
}
//0x14
void inc_d(Cpu* cpu) {
    increment_8bit_register(cpu, &cpu->d);
    cpu->m = 1;
    cpu->t = 4;
}
//0x15
void dec_d(Cpu* cpu) {
    decrement_8bit_register(cpu, &cpu->d);
    cpu->m = 1;
    cpu->t = 4;
}
//0x16
void ld_d_8bit_immediate(Cpu* cpu, uint8_t n) {
    cpu->d = n;
    cpu->m = 2;
    cpu->t = 8;
}
//0x17
void rla(Cpu* cpu) {
    clear_flag(cpu, SUBTRACTION_FLAG);
    clear_flag(cpu, ZERO_FLAG);
    clear_flag(cpu, HALFCARRY_FLAG);
    bool set_carry = false;
    if ((cpu->a << 1) > UINT8_MAX)
        set_carry = true;

    cpu->a = cpu->a << 1;

    //0th bit gets rotated through from carry flag, so set it afterwards
    if (is_flag_set(cpu, CARRY_FLAG))
        cpu->a = cpu->a | 0x1;     
    else
        cpu->a = cpu->a & ~(0x1);
    if (set_carry)
        set_flag(cpu, CARRY_FLAG);
    else
        clear_flag(cpu, CARRY_FLAG);
    cpu->m = 1;
    cpu->t = 4;
}
//0x18
void jr_8bit_immediate(Cpu* cpu, int8_t n) {
    //Not sure if this handles correctly like the gameboy
    cpu->pc += n;
    cpu->m = 3;
    cpu->t = 12;
}
//0x19
void add_HL_DE(Cpu* cpu) {
    uint16_t bc_value = join_registers(cpu->d, cpu->e);
    add_to_16bit_register(cpu, &cpu->h, &cpu->l, bc_value);
    cpu->m = 2;
    cpu->t = 8;
}
//0x1A
void ld_a_de(Cpu* cpu) {
    uint16_t address = join_registers(cpu->d, cpu->e);
    cpu->a = read_byte(cpu, address);
    cpu->m = 2;
    cpu->t = 8;
}
//0x1B
void dec_DE(Cpu* cpu) {
    decrement_16bit_register(&cpu->d, &cpu->e);
    cpu->m = 2;
    cpu->t = 8;
}
//0x1C
void inc_e(Cpu* cpu) {
    increment_8bit_register(cpu, &cpu->e);
    cpu->m = 1;
    cpu->t = 4;
}
//0x1D
void dec_e(Cpu* cpu) {
    decrement_8bit_register(cpu, &cpu->e);
    cpu->m = 1;
    cpu->t = 4;
}
//0x1E
void ld_e_8bit_immediate(Cpu* cpu, uint8_t n) {
    cpu->e = n;
    cpu->m = 2;
    cpu->t = 8;
}
//0x1F
void rra(Cpu* cpu) {
    clear_flag(cpu, SUBTRACTION_FLAG);
    clear_flag(cpu, ZERO_FLAG);
    clear_flag(cpu, HALFCARRY_FLAG);
    bool set_carry = false;
    if (cpu->a & 0x1)
        set_carry = true;

    cpu->a = cpu->a >> 1;

    //7th bit gets rotated through from carry flag, so set it afterwards
    if (is_flag_set(cpu, CARRY_FLAG))
        cpu->a = cpu->a | 0x80;     
    else
        cpu->a = cpu->a & ~(0x80);
    if (set_carry)
        set_flag(cpu, CARRY_FLAG);
    else
        clear_flag(cpu, CARRY_FLAG);
    cpu->m = 1;
    cpu->t = 4;
}
//2x
//0x20
void jr_nz_8bit_immediate(Cpu* cpu, int8_t n) {
    if (is_flag_set(cpu, ZERO_FLAG)) {
        //Don't jump
        cpu->m = 2;
        cpu->t = 8;
        return;
    }
    cpu->pc += n;
    cpu->m = 3;
    cpu->t = 12;
}
//0x21
void ld_HL_16bit_immediate(Cpu* cpu, uint16_t n) {
    write_to_16bit_registers(&cpu->h, &cpu->l, n);
    cpu->m = 3;
    cpu->t = 12;
}
//0x22
void ld_hlincrement_a(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    write_byte(cpu, address, cpu->a);
    increment_16bit_register(&cpu->h, &cpu->l);
    cpu->m = 2;
    cpu->t = 8;
}
//0x23
void inc_HL(Cpu* cpu) {
    increment_16bit_register(&cpu->h, &cpu->l);
    cpu->m = 2;
    cpu->t = 8;
}
//0x24
void inc_h(Cpu* cpu) {
    increment_8bit_register(cpu, &cpu->h);
    cpu->m = 1;
    cpu->t = 4;
}
//0x25
void dec_h(Cpu* cpu) {
    decrement_8bit_register(cpu, &cpu->h);
    cpu->m = 1;
    cpu->t = 4;
}
//0x26
void ld_h_8bit_immediate(Cpu* cpu, uint8_t n) {
    cpu->h = n;
    cpu->m = 2;
    cpu->t = 8;
}
//0x27
//TODO
//0x28
void jr_z_8bit_immediate(Cpu* cpu, int8_t n) {
    if (!is_flag_set(cpu, ZERO_FLAG)) {
        //Don't jump
        cpu->m = 2;
        cpu->t = 8;
        return;
    }
    cpu->pc += n;
    cpu->m = 3;
    cpu->t = 12;
}
//0x29
void add_HL_HL(Cpu* cpu) {
    uint16_t value = join_registers(cpu->h, cpu->l);
    add_to_16bit_register(cpu, &cpu->h, &cpu->l, value);
    cpu->m = 2;
    cpu->t = 8;
}
//0x2A
void ld_a_hlincrement(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    cpu->a = read_byte(cpu, address);
    increment_16bit_register(&cpu->h, &cpu->l);
    cpu->m = 2;
    cpu->t = 8;
}
//0x2B
void dec_HL(Cpu* cpu) {
    decrement_16bit_register(&cpu->h, &cpu->l);
    cpu->m = 2;
    cpu->t = 8;
}
//0x2C
void inc_l(Cpu* cpu) {
    increment_8bit_register(cpu, &cpu->l);
    cpu->m = 1;
    cpu->t = 4;
}
//0x2D
void dec_l(Cpu* cpu) {
    decrement_8bit_register(cpu, &cpu->l);
    cpu->m = 1;
    cpu->t = 4;
}
//0x2E
void ld_l_8bit_immediate(Cpu* cpu, uint8_t n) {
    cpu->l = n;
    cpu->m = 2;
    cpu->t = 8;
}
//0x2F
void cpl(Cpu* cpu) {
    set_flag(cpu, SUBTRACTION_FLAG);
    set_flag(cpu, HALFCARRY_FLAG);
    cpu->a = ~cpu->a;
}
//3x
//0x30
void jr_nc_8bit_immediate(Cpu* cpu, int8_t n) {
    if (is_flag_set(cpu, CARRY_FLAG)) {
        //Don't jump
        cpu->pc++;
        cpu->m = 2;
        cpu->t = 8;
    }
    cpu->pc += n;
    cpu->m = 3;
    cpu->t = 12;
}
//0x31
void ld_sp_16bit_immediate(Cpu* cpu, uint16_t n) {
    uint16_t value = read_word(cpu, n);
    cpu->sp = value;
    cpu->pc += 2;
    cpu->m = 3;
    cpu->t = 12;
}
//0x32
void ld_hldecrement_a(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    write_byte(cpu, address, cpu->a);
	decrement_16bit_register(&cpu->h, &cpu->l);
    cpu->m = 2;
    cpu->t = 8;
}
//0x33
void inc_sp(Cpu* cpu) {
    cpu->sp++;
    cpu->m = 2;
    cpu->t = 8;
}
//0x34
void inc_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    increment_8bit_register(cpu, &value);
    cpu->m = 3;
    cpu->t = 12;
}
//0x35
void dec_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    decrement_8bit_register(cpu, &value);
    cpu->m = 3;
    cpu->t = 12;
}
//0x36
void ld_hl_8bit_immediate(Cpu* cpu, uint8_t n) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    write_byte(cpu, address, n);
    cpu->pc++;
    cpu->m = 3;
    cpu->t = 12;
}
//0x37
void scf(Cpu* cpu) {
    set_flag(cpu, CARRY_FLAG);
    clear_flag(cpu, SUBTRACTION_FLAG);
    clear_flag(cpu, HALFCARRY_FLAG);
    cpu->m = 1;
    cpu->t = 4;
}
//0x38
void jr_c_8bit_immediate(Cpu* cpu, int8_t n) {
        cpu->pc++;
    if (!is_flag_set(cpu, CARRY_FLAG)) {
        //Don't jump
        cpu->m = 2;
        cpu->t = 8;
    }
    cpu->pc += n;
    cpu->m = 3;
    cpu->t = 12;
}
//0x39
void add_HL_sp(Cpu* cpu) {
    add_to_16bit_register(cpu, &cpu->h, &cpu->l, cpu->sp);
    cpu->m = 2;
    cpu->t = 8;
}
//0x3A
void ld_a_hldecrement(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    cpu->a = value;
    value++;
    write_byte(cpu, address, value);
    cpu->m = 2;
    cpu->t = 8;
}
//0x3B
void dec_sp(Cpu* cpu) {
    cpu->sp--;
    cpu->m = 2;
    cpu->t = 8;
}
//0x3C
void inc_a(Cpu* cpu) {
    increment_8bit_register(cpu, &cpu->a);
    cpu->m = 1;
    cpu->t = 4;
}
//0x3D
void dec_a(Cpu* cpu) {
    decrement_8bit_register(cpu, &cpu->a);
    cpu->m = 1;
    cpu->t = 4;
}
//0x3E
void ld_a_8bit_immediate(Cpu* cpu, uint8_t n) {
    cpu->a = n;
    cpu->pc++;
    cpu->m = 2;
    cpu->t = 8;
}
//0x3F
void ccf(Cpu* cpu) {
    clear_flag(cpu, SUBTRACTION_FLAG);
    clear_flag(cpu, HALFCARRY_FLAG);
    if (is_flag_set(cpu, CARRY_FLAG))
        clear_flag(cpu, CARRY_FLAG);
    else
        set_flag(cpu, CARRY_FLAG);
    cpu->m = 1;
    cpu->t = 4;
}
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
    uint8_t value = read_byte(cpu, address);
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
    uint8_t value = read_byte(cpu, address);
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
    uint8_t value = read_byte(cpu, address);
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
    uint8_t value = read_byte(cpu, address);
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
    uint8_t value = read_byte(cpu, address);
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
    uint8_t value = read_byte(cpu, address);
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
//0x70
void ld_hl_b(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    write_byte(cpu, address, cpu->b);
    cpu->m = 2;
    cpu->t = 8;
}
//0x71
void ld_hl_c(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    write_byte(cpu, address, cpu->c);
    cpu->m = 2;
    cpu->t = 8;
}
//0x72
void ld_hl_d(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    write_byte(cpu, address, cpu->d);
    cpu->m = 2;
    cpu->t = 8;
}
//0x73
void ld_hl_e(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    write_byte(cpu, address, cpu->e);
    cpu->m = 2;
    cpu->t = 8;
}
//0x74
void ld_hl_h(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    write_byte(cpu, address, cpu->h);
    cpu->m = 2;
    cpu->t = 8;
}
//0x75
void ld_hl_l(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    write_byte(cpu, address, cpu->l);
    cpu->m = 2;
    cpu->t = 8;
}
//0x76
//TODO
//0x77
void ld_hl_a(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    write_byte(cpu, address, cpu->a);
    cpu->m = 2;
    cpu->t = 8;
}
//0x78
void ld_a_b(Cpu* cpu) {
    cpu->a = cpu->b;
    cpu->m = 1;
    cpu->t = 4;
}
//0x79
void ld_a_c(Cpu* cpu) {
    cpu->a = cpu->c;
    cpu->m = 1;
    cpu->t = 4;
}
//0x7A
void ld_a_d(Cpu* cpu) {
    cpu->a = cpu->d;
    cpu->m = 1;
    cpu->t = 4;
}
//0x7B
void ld_a_e(Cpu* cpu) {
    cpu->a = cpu->e;
    cpu->m = 1;
    cpu->t = 4;
}
//0x7C
void ld_a_h(Cpu* cpu) {
    cpu->a = cpu->h;
    cpu->m = 1;
    cpu->t = 4;
}
//0x7D
void ld_a_l(Cpu* cpu) {
    cpu->a = cpu->l;
    cpu->m = 1;
    cpu->t = 4;
}
//0x7E
void ld_a_hl(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    uint8_t value = read_byte(cpu, address);
    cpu->a = value;
    cpu->m = 2;
    cpu->t = 8;
}
//0x7F
void ld_a_a(Cpu* cpu) {
    cpu->a = cpu->a;
    cpu->m = 1;
    cpu->t = 4;
}
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
    uint8_t value = read_byte(cpu, address);
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
    uint8_t value = read_byte(cpu, address);
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
    uint8_t value = read_byte(cpu, address);
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
    uint8_t value = read_byte(cpu, address);
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
    uint8_t value = read_byte(cpu, address);
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
    uint8_t value = read_byte(cpu, address);
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
    uint8_t value = read_byte(cpu, address);
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
    uint8_t value = read_byte(cpu, address);
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
//0xC0
void ret_nz(Cpu* cpu) {
    if (is_flag_set(cpu, ZERO_FLAG)) {
        //Don't return from call
        cpu->m = 2;
        cpu->t = 8;
        return;
    }

    //Pop pc off stack
    cpu->pc = read_word(cpu, cpu->sp);
    cpu->sp += 2;
    cpu->m = 5;
    cpu->t = 20;
}
//0xC1
void pop_BC(Cpu* cpu) {
    cpu->c = read_byte(cpu, cpu->sp);
    cpu->sp++;
    cpu->b = read_byte(cpu, cpu->sp);
    cpu->sp++;
    cpu->m = 3;
    cpu->t = 12;
}
//0xC2
void jp_nz_16bit_immediate(Cpu* cpu, uint16_t n) {
    if (is_flag_set(cpu, ZERO_FLAG)) {
        //Don't jump
        cpu->pc += 2;
        cpu->m = 3;
        cpu->t = 12;
        return;
    }

    cpu->pc = n;
    cpu->m = 4;
    cpu->t = 16;
}
//0xC3
void jp_16bit_immediate(Cpu*cpu, uint16_t n) {
    cpu->pc = n;
    cpu->m = 4;
    cpu->t = 16;
}
//0xC5
void push_BC(Cpu* cpu) {
    push_16bit_register(cpu, cpu->b, cpu->c);
    cpu->m = 4;
    cpu->t = 16;
}
//0xC6
void add_a_8bit_immediate(Cpu* cpu, uint8_t n) {
    add_to_accumulator(cpu, n);
    cpu->m = 2;
    cpu->t = 8;
}
//0xC8
void ret_z(Cpu* cpu) {
    if (!is_flag_set(cpu, ZERO_FLAG)) {
        //Don't return from call
        cpu->m = 2;
        cpu->t = 8;
        return;
    }

    //Pop pc off stack
    cpu->pc = read_word(cpu, cpu->sp);
    cpu->sp += 2;
    cpu->m = 5;
    cpu->t = 20;
}
//0xC9
void ret(Cpu* cpu) {
    //Pop pc off stack
    cpu->pc = read_word(cpu, cpu->sp);
    cpu->sp += 2;
    cpu->m = 4;
    cpu->t = 16;
}
//0xCA
void jp_z_16bit_immediate(Cpu* cpu, uint16_t n) {
    if (!is_flag_set(cpu, ZERO_FLAG)) {
        //Don't jump
        cpu->m = 3;
        cpu->t = 12;
        cpu->pc += 2;
        return;
    }

    cpu->pc = n;
    cpu->m = 4;
    cpu->t = 16;
}
//0xCB
void prefix_cb(Cpu* cpu, uint8_t opcode);       //Function at end of file
//0xCD
void call_16bit_immediate(Cpu* cpu, uint16_t n) {
    //Push the resulting program counter after the call
    cpu->sp -= 2;
    write_word(cpu, cpu->sp, cpu->pc + 2);

    cpu->pc = n;
    cpu->m = 6;
    cpu->t = 24;
}
//Dx
//TODO:
//0xD1
void pop_DE(Cpu* cpu) {
    cpu->e = read_byte(cpu, cpu->sp);
    cpu->sp++;
    cpu->d = read_byte(cpu, cpu->sp);
    cpu->sp++;
    cpu->m = 3;
    cpu->t = 12;
}
//0xD2
void jp_nc_16bit_immediate(Cpu* cpu, uint16_t n) {
    if (is_flag_set(cpu, CARRY_FLAG)) {
        //Don't jump
        cpu->pc += 2;
        cpu->m = 3;
        cpu->t = 12;
        return;
    }

    cpu->pc = n;
    cpu->m = 4;
    cpu->t = 16;
}
//0xD5
void push_DE(Cpu* cpu) {
    push_16bit_register(cpu, cpu->d, cpu->e);
    cpu->m = 4;
    cpu->t = 16;
}
//0xD8
void ret_c(Cpu* cpu) {
    if (!is_flag_set(cpu, CARRY_FLAG)) {
        //Don't return from call
        cpu->m = 2;
        cpu->t = 8;
        return;
    }

    //Pop pc off stack
    cpu->pc = read_word(cpu, cpu->sp);
    cpu->sp += 2;
    cpu->m = 5;
    cpu->t = 20;
}
//0xD9
void reti(Cpu* cpu) {
    //Enable interrupts
    cpu->interrupt_master_enable = true;

    //Jump to pc on stack
    cpu->pc = read_word(cpu, cpu->sp);
    cpu->sp += 2;

    cpu->m = 4;
    cpu->t = 16;
}
//0xDA
void jp_c_16bit_immediate(Cpu* cpu, uint16_t n) {
    if (!is_flag_set(cpu, CARRY_FLAG)) {
        //Don't jump
        cpu->pc += 2;
        cpu->m = 3;
        cpu->t = 12;
        return;
    }

    cpu->pc = n;
    cpu->m = 4;
    cpu->t = 16;
}
//Ex
//TODO:
//0xE0
void ldh_8bit_immediate_a(Cpu* cpu, uint8_t n) {
    write_byte(cpu, MEM_MAPPED_IO + n, cpu->a);
    cpu->pc++;
    cpu->m = 3;
    cpu->t = 12;
}
//0xE1
void pop_HL(Cpu* cpu) {
    cpu->l = read_byte(cpu, cpu->sp);
    cpu->sp++;
    cpu->h = read_byte(cpu, cpu->sp);
    cpu->sp++;
    cpu->m = 3;
    cpu->t = 12;
}
//0xE5
void push_HL(Cpu* cpu) {
    push_16bit_register(cpu, cpu->h, cpu->l);
    cpu->m = 4;
    cpu->t = 16;
}
//0xE9
void jp_hl(Cpu* cpu) {
    cpu->pc = join_registers(cpu->h, cpu->l);
    cpu->m = 1;
    cpu->t = 4;
}
//Fx
//TODO:
//0xF0
void ldh_a_8bit_immediate(Cpu* cpu, uint8_t n) {
    cpu->a = read_byte(cpu, MEM_MAPPED_IO + n);
    cpu->pc++;
    cpu->m = 3;
    cpu->t = 12;
}
//0xF1
void pop_AF(Cpu* cpu) {
    //TODO: check if flags are affected here at all
    cpu->f = read_byte(cpu, cpu->sp);
    cpu->sp++;
    cpu->a = read_byte(cpu, cpu->sp);
    cpu->sp++;
    cpu->m = 3;
    cpu->t = 12;
}
//0xF3
void di(Cpu* cpu) {
    cpu->interrupt_master_enable = false;
    cpu->m = 1;
    cpu->t = 4;
}
//0xF5
void push_AF(Cpu* cpu) {
    push_16bit_register(cpu, cpu->a, cpu->f);
    cpu->m = 4;
    cpu->t = 16;
}
//0xFA
void ld_a_16bit_address(Cpu* cpu, uint16_t address) {
    cpu->a = read_byte(cpu, address);
    cpu->pc += 2;
    cpu->m = 4;
    cpu->t = 16;
}
//0xFB
void ei(Cpu* cpu) {
    cpu->interrupt_master_enable = true;
    cpu->m = 1;
    cpu->t = 4;
}
//0xFE
void cp_8bit_immediate(Cpu* cpu, uint8_t n) {
    compare_with_accumulator(cpu, n);
    cpu->pc++;
    cpu->m = 2;
    cpu->t = 8;
}

//Vblank interrupt
void rst_40(Cpu* cpu) {
    //Disable interrupts
    cpu->interrupt_master_enable = false;

    //Store pc on stack
    cpu->sp -= 2;
    write_word(cpu, cpu->sp, cpu->pc);

    //Jump to interrupt handler
    cpu->pc = 0x40;
    cpu->m = 3;
    cpu->t = 12;
}

void rotate_8bit_left(Cpu* cpu, uint8_t* n) {
    clear_flag(cpu, ALL_FLAGS);
    bool last_bit_set = (*n & 0x80) == 1;
    *n = *n << 1;
    //Last bit gets shifted to carry flag
    //Last bit rotates around
    if (last_bit_set) {
        set_flag(cpu, CARRY_FLAG);
        *n |= 0x1;
    } else {
        clear_flag(cpu, CARRY_FLAG);
        *n &= 0x1;
    }
    if (*n == 0)
        set_flag(cpu, ZERO_FLAG);

}

void rotate_8bit_right(Cpu* cpu, uint8_t* n) {
    clear_flag(cpu, ALL_FLAGS);
    bool first_bit_set = (*n & 0x1) == 1;
    *n = *n >> 1;
    //First bit gets shifted to carry flag
    //First bit rotates around
    if (first_bit_set) {
        set_flag(cpu, CARRY_FLAG);
        *n |= 0x80;
    } else {
        clear_flag(cpu, CARRY_FLAG);
        *n &= 0x1;
    }
    if (*n == 0)
        set_flag(cpu, ZERO_FLAG);

}

void rotate_8bit_left_through_carry(Cpu* cpu, uint8_t* n) {
    clear_flag(cpu, ALL_FLAGS);
    bool last_bit_set = (*n & 0x80) == 1;
    bool carry_set = is_flag_set(cpu, CARRY_FLAG);
    *n = *n << 1;

    //Carry becomes 0th bit, last bit becomes carry
    if (last_bit_set)
        set_flag(cpu, CARRY_FLAG);
    else
        clear_flag(cpu, CARRY_FLAG);

    if (carry_set)
        *n |= 0x1;
    else
        *n &= 0x1;
    if (*n == 0)
        set_flag(cpu, ZERO_FLAG);
}

void rotate_8bit_right_through_carry(Cpu* cpu, uint8_t* n) {
    clear_flag(cpu, ALL_FLAGS);
    bool first_bit_set = (*n & 0x1) == 1;
    bool carry_set = is_flag_set(cpu, CARRY_FLAG);
    *n = *n >> 1;

    //Carry becomes 7th bit, first bit becomes carry
    if (first_bit_set)
        set_flag(cpu, CARRY_FLAG);
    else
        clear_flag(cpu, CARRY_FLAG);

    if (carry_set)
        *n |= 0x80;
    else
        *n &= 0x80;
    if (*n == 0)
        set_flag(cpu, ZERO_FLAG);
}

void rotate_8bit_left_arithmetic(Cpu* cpu, uint8_t* n) {
    clear_flag(cpu, ALL_FLAGS);
    bool last_bit_set = (*n & 0x80) == 1;
    *n = *n << 1;
    //Last bit gets shifted to carry flag
    //First bit gets reset in the shift
    if (last_bit_set) {
        set_flag(cpu, CARRY_FLAG);
    } else {
        clear_flag(cpu, CARRY_FLAG);
    }
    if (*n == 0)
        set_flag(cpu, ZERO_FLAG);

}

void rotate_8bit_right_arithmetic(Cpu* cpu, uint8_t* n) {
    clear_flag(cpu, ALL_FLAGS);
    bool first_bit_set = (*n & 0x1) == 1;
    *n = (int8_t)*n >> 1;
    //0th bit gets shifted to carry flag
    if (first_bit_set) {
        set_flag(cpu, CARRY_FLAG);
    } else {
        clear_flag(cpu, CARRY_FLAG);
    }
    if (*n == 0)
        set_flag(cpu, ZERO_FLAG);

}
//Prefix CB opcodes
//0x
void rlc_b(Cpu* cpu) {
    rotate_8bit_left(cpu, &cpu->b);
    cpu->m = 2;
    cpu->t = 8;
}

void rlc_c(Cpu* cpu) {
    rotate_8bit_left(cpu, &cpu->c);
    cpu->m = 2;
    cpu->t = 8;
}

void rlc_d(Cpu* cpu) {
    rotate_8bit_left(cpu, &cpu->d);
    cpu->m = 2;
    cpu->t = 8;
}

void rlc_e(Cpu* cpu) {
    rotate_8bit_left(cpu, &cpu->e);
    cpu->m = 2;
    cpu->t = 8;
}

void rlc_h(Cpu* cpu) {
    rotate_8bit_left(cpu, &cpu->h);
    cpu->m = 2;
    cpu->t = 8;
}

void rlc_l(Cpu* cpu) {
    rotate_8bit_left(cpu, &cpu->l);
    cpu->m = 2;
    cpu->t = 8;
}

void rlc_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    rotate_8bit_left(cpu, &value);
    write_byte(cpu, address, value);
    cpu->m = 4;
    cpu->t = 16;
}

void rlc_a(Cpu* cpu) {
    rotate_8bit_left(cpu, &cpu->a);
    cpu->m = 2;
    cpu->t = 8;
}

void rrc_b(Cpu* cpu) {
    rotate_8bit_right(cpu, &cpu->b);
    cpu->m = 2;
    cpu->t = 8;
}

void rrc_c(Cpu* cpu) {
    rotate_8bit_right(cpu, &cpu->c);
    cpu->m = 2;
    cpu->t = 8;
}

void rrc_d(Cpu* cpu) {
    rotate_8bit_right(cpu, &cpu->d);
    cpu->m = 2;
    cpu->t = 8;
}

void rrc_e(Cpu* cpu) {
    rotate_8bit_right(cpu, &cpu->e);
    cpu->m = 2;
    cpu->t = 8;
}

void rrc_h(Cpu* cpu) {
    rotate_8bit_right(cpu, &cpu->h);
    cpu->m = 2;
    cpu->t = 8;
}

void rrc_l(Cpu* cpu) {
    rotate_8bit_right(cpu, &cpu->l);
    cpu->m = 2;
    cpu->t = 8;
}

void rrc_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    rotate_8bit_right(cpu, &value);
    write_byte(cpu, address, value);
    cpu->m = 4;
    cpu->t = 16;
}

void rrc_a(Cpu* cpu) {
    rotate_8bit_right(cpu, &cpu->a);
    cpu->m = 2;
    cpu->t = 8;
}

//1x
void rl_b(Cpu* cpu) {
    rotate_8bit_left_through_carry(cpu, &cpu->b);
    cpu->m = 2;
    cpu->t = 8;
}

void rl_c(Cpu* cpu) {
    rotate_8bit_left_through_carry(cpu, &cpu->c);
    cpu->m = 2;
    cpu->t = 8;
}

void rl_d(Cpu* cpu) {
    rotate_8bit_left_through_carry(cpu, &cpu->d);
    cpu->m = 2;
    cpu->t = 8;
}

void rl_e(Cpu* cpu) {
    rotate_8bit_left_through_carry(cpu, &cpu->e);
    cpu->m = 2;
    cpu->t = 8;
}

void rl_h(Cpu* cpu) {
    rotate_8bit_left_through_carry(cpu, &cpu->h);
    cpu->m = 2;
    cpu->t = 8;
}

void rl_l(Cpu* cpu) {
    rotate_8bit_left_through_carry(cpu, &cpu->l);
    cpu->m = 2;
    cpu->t = 8;
}

void rl_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    rotate_8bit_left_through_carry(cpu, &value);
    write_byte(cpu, address, value);
    cpu->m = 4;
    cpu->t = 16;
}

void rl_a(Cpu* cpu) {
    rotate_8bit_left_through_carry(cpu, &cpu->a);
    cpu->m = 2;
    cpu->t = 8;
}

void rr_b(Cpu* cpu) {
    rotate_8bit_right_through_carry(cpu, &cpu->b);
    cpu->m = 2;
    cpu->t = 8;
}

void rr_c(Cpu* cpu) {
    rotate_8bit_right_through_carry(cpu, &cpu->c);
    cpu->m = 2;
    cpu->t = 8;
}

void rr_d(Cpu* cpu) {
    rotate_8bit_right_through_carry(cpu, &cpu->d);
    cpu->m = 2;
    cpu->t = 8;
}

void rr_e(Cpu* cpu) {
    rotate_8bit_right_through_carry(cpu, &cpu->e);
    cpu->m = 2;
    cpu->t = 8;
}

void rr_h(Cpu* cpu) {
    rotate_8bit_right_through_carry(cpu, &cpu->h);
    cpu->m = 2;
    cpu->t = 8;
}

void rr_l(Cpu* cpu) {
    rotate_8bit_right_through_carry(cpu, &cpu->l);
    cpu->m = 2;
    cpu->t = 8;
}

void rr_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    rotate_8bit_right_through_carry(cpu, &value);
    write_byte(cpu, address, value);
    cpu->m = 4;
    cpu->t = 16;
}

void rr_a(Cpu* cpu) {
    rotate_8bit_right_through_carry(cpu, &cpu->a);
    cpu->m = 2;
    cpu->t = 8;
}

//2x
void sla_b(Cpu* cpu) {
    rotate_8bit_left_arithmetic(cpu, &cpu->b);
    cpu->m = 2;
    cpu->t = 8;
}

void sla_c(Cpu* cpu) {
    rotate_8bit_left_arithmetic(cpu, &cpu->c);
    cpu->m = 2;
    cpu->t = 8;
}

void sla_d(Cpu* cpu) {
    rotate_8bit_left_arithmetic(cpu, &cpu->d);
    cpu->m = 2;
    cpu->t = 8;
}

void sla_e(Cpu* cpu) {
    rotate_8bit_left_arithmetic(cpu, &cpu->e);
    cpu->m = 2;
    cpu->t = 8;
}

void sla_h(Cpu* cpu) {
    rotate_8bit_left_arithmetic(cpu, &cpu->h);
    cpu->m = 2;
    cpu->t = 8;
}

void sla_l(Cpu* cpu) {
    rotate_8bit_left_arithmetic(cpu, &cpu->l);
    cpu->m = 2;
    cpu->t = 8;
}

void sla_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    rotate_8bit_left_arithmetic(cpu, &value);
    write_byte(cpu, address, value);
    cpu->m = 4;
    cpu->t = 16;
}

void sla_a(Cpu* cpu) {
    rotate_8bit_left_arithmetic(cpu, &cpu->a);
    cpu->m = 2;
    cpu->t = 8;
}

void sra_b(Cpu* cpu) {
    rotate_8bit_right_arithmetic(cpu, &cpu->b);
    cpu->m = 2;
    cpu->t = 8;
}

void sra_c(Cpu* cpu) {
    rotate_8bit_right_arithmetic(cpu, &cpu->c);
    cpu->m = 2;
    cpu->t = 8;
}

void sra_d(Cpu* cpu) {
    rotate_8bit_right_arithmetic(cpu, &cpu->d);
    cpu->m = 2;
    cpu->t = 8;
}

void sra_e(Cpu* cpu) {
    rotate_8bit_right_arithmetic(cpu, &cpu->e);
    cpu->m = 2;
    cpu->t = 8;
}

void sra_h(Cpu* cpu) {
    rotate_8bit_right_arithmetic(cpu, &cpu->h);
    cpu->m = 2;
    cpu->t = 8;
}

void sra_l(Cpu* cpu) {
    rotate_8bit_right_arithmetic(cpu, &cpu->l);
    cpu->m = 2;
    cpu->t = 8;
}

void sra_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    rotate_8bit_right_arithmetic(cpu, &value);
    write_byte(cpu, address, value);
    cpu->m = 4;
    cpu->t = 16;
}

void sra_a(Cpu* cpu) {
    rotate_8bit_right_arithmetic(cpu, &cpu->a);
    cpu->m = 2;
    cpu->t = 8;
}

//Swaps the upper 4 bits with lower 4 bits
//Zero set if result is 0, other flags are cleared
void swap_8bit(Cpu* cpu, uint8_t* n) {
    clear_flag(cpu, ALL_FLAGS);
    uint8_t lower = *n & 0xF;
    uint8_t higher = *n & 0xF0;
    *n = lower | higher;        //Swap lower and higher
    if (*n == 0)
        set_flag(cpu, ZERO_FLAG);
}
//3x
void swap_b(Cpu* cpu) {
    swap_8bit(cpu, &cpu->b);
    cpu->m = 2;
    cpu->t = 8;
}

void swap_c(Cpu* cpu) {
    swap_8bit(cpu, &cpu->c);
    cpu->m = 2;
    cpu->t = 8;
}

void swap_d(Cpu* cpu) {
    swap_8bit(cpu, &cpu->d);
    cpu->m = 2;
    cpu->t = 8;
}

void swap_e(Cpu* cpu) {
    swap_8bit(cpu, &cpu->e);
    cpu->m = 2;
    cpu->t = 8;
}

void swap_h(Cpu* cpu) {
    swap_8bit(cpu, &cpu->h);
    cpu->m = 2;
    cpu->t = 8;
}

void swap_l(Cpu* cpu) {
    swap_8bit(cpu, &cpu->l);
    cpu->m = 2;
    cpu->t = 8;
}

void swap_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    swap_8bit(cpu, &value);
    write_byte(cpu, address, value);
    cpu->m = 4;
    cpu->t = 16;
}

void swap_a(Cpu* cpu) {
    swap_8bit(cpu, &cpu->a);
    cpu->m = 2;
    cpu->t = 8;
}

void rotate_8bit_right_logical(Cpu* cpu, uint8_t* n) {
    clear_flag(cpu, ALL_FLAGS);
    bool last_bit_set = (*n & 0x1) == 1;
    *n = *n >> 1;
    if (last_bit_set)
        set_flag(cpu, CARRY_FLAG);
    if (*n == 0)
        set_flag(cpu, ZERO_FLAG);
}

void srl_b(Cpu* cpu) {
    rotate_8bit_right_logical(cpu, &cpu->b);
    cpu->m = 2;
    cpu->t = 8;
}

void srl_c(Cpu* cpu) {
    rotate_8bit_right_logical(cpu, &cpu->c);
    cpu->m = 2;
    cpu->t = 8;
}

void srl_d(Cpu* cpu) {
    rotate_8bit_right_logical(cpu, &cpu->d);
    cpu->m = 2;
    cpu->t = 8;
}

void srl_e(Cpu* cpu) {
    rotate_8bit_right_logical(cpu, &cpu->e);
    cpu->m = 2;
    cpu->t = 8;
}

void srl_h(Cpu* cpu) {
    rotate_8bit_right_logical(cpu, &cpu->h);
    cpu->m = 2;
    cpu->t = 8;
}

void srl_l(Cpu* cpu) {
    rotate_8bit_right_logical(cpu, &cpu->l);
    cpu->m = 2;
    cpu->t = 8;
}

void srl_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    rotate_8bit_right_logical(cpu, &value);
    write_byte(cpu, address, value);
    cpu->m = 4;
    cpu->t = 16;
}

void srl_a(Cpu* cpu) {
    rotate_8bit_right_logical(cpu, &cpu->a);
    cpu->m = 2;
    cpu->t = 8;
}

//4x

//Tests the given bit (starting at 0th bit) of n. Only bits 0th-7th
//Sets zero flag if the bit is not set.
//Subtraction flag reset, halfcarry flag set, carry flag unchanged
void test_bit_8bit(Cpu* cpu, uint8_t n, uint8_t bit_to_test) {
    if (bit_to_test > 7) {
        printf("Cannot test bit %d in an 8 bit number\n", bit_to_test);
    }
    clear_flag(cpu, SUBTRACTION_FLAG);
    clear_flag(cpu, HALFCARRY_FLAG);
    uint8_t mask;
    mask = 1 << bit_to_test; 

    if ((n & mask) == 0)
        set_flag(cpu, ZERO_FLAG);
    else
        clear_flag(cpu, ZERO_FLAG);
}

void bit_0_b(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->b, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_0_c(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->c, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_0_d(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->d, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_0_e(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->e, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_0_h(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->h, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_0_l(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->l, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_0_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    test_bit_8bit(cpu, value, 0);
    cpu->m = 4;
    cpu->t = 16;
}

void bit_0_a(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->a, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_1_b(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->b, 1);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_1_c(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->c, 1);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_1_d(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->d, 1);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_1_e(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->e, 1);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_1_h(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->h, 1);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_1_l(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->l, 1);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_1_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    test_bit_8bit(cpu, value, 1);
    cpu->m = 4;
    cpu->t = 16;
}

void bit_1_a(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->a, 1);
    cpu->m = 2;
    cpu->t = 8;
}

//5x
void bit_2_b(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->b, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_2_c(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->c, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_2_d(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->d, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_2_e(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->e, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_2_h(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->h, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_2_l(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->l, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_2_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    test_bit_8bit(cpu, value, 2);
    cpu->m = 4;
    cpu->t = 16;
}

void bit_2_a(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->a, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_3_b(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->b, 3);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_3_c(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->c, 3);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_3_d(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->d, 3);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_3_e(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->e, 3);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_3_h(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->h, 3);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_3_l(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->l, 3);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_3_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    test_bit_8bit(cpu, value, 3);
    cpu->m = 4;
    cpu->t = 16;
}

void bit_3_a(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->a, 3);
    cpu->m = 2;
    cpu->t = 8;
}

//6x
void bit_4_b(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->b, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_4_c(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->c, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_4_d(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->d, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_4_e(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->e, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_4_h(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->h, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_4_l(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->l, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_4_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    test_bit_8bit(cpu, value, 4);
    cpu->m = 4;
    cpu->t = 16;
}

void bit_4_a(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->a, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_5_b(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->b, 5);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_5_c(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->c, 5);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_5_d(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->d, 5);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_5_e(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->e, 5);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_5_h(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->h, 5);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_5_l(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->l, 5);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_5_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    test_bit_8bit(cpu, value, 5);
    cpu->m = 4;
    cpu->t = 16;
}

void bit_5_a(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->a, 5);
    cpu->m = 2;
    cpu->t = 8;
}

//7x
void bit_6_b(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->b, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_6_c(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->c, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_6_d(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->d, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_6_e(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->e, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_6_h(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->h, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_6_l(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->l, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_6_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    test_bit_8bit(cpu, value, 6);
    cpu->m = 4;
    cpu->t = 16;
}

void bit_6_a(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->a, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_7_b(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->b, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_7_c(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->c, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_7_d(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->d, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_7_e(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->e, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_7_h(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->h, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_7_l(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->l, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void bit_7_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    test_bit_8bit(cpu, value, 7);
    cpu->m = 4;
    cpu->t = 16;
}

void bit_7_a(Cpu* cpu) {
    test_bit_8bit(cpu, cpu->a, 7);
    cpu->m = 2;
    cpu->t = 8;
}
//8x
void reset_bit_8bit(uint8_t* n, uint8_t bit_to_reset) {
    if (bit_to_reset > 7) {
        printf("Cannot reset bit %d in an 8 bit number\n", bit_to_reset);
    }
    uint8_t mask;
    mask = ~(1 << bit_to_reset); 
    *n &= mask;
}

void res_0_b(Cpu* cpu) {
    reset_bit_8bit(&cpu->b, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void res_0_c(Cpu* cpu) {
    reset_bit_8bit(&cpu->c, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void res_0_d(Cpu* cpu) {
    reset_bit_8bit(&cpu->d, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void res_0_e(Cpu* cpu) {
    reset_bit_8bit(&cpu->e, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void res_0_h(Cpu* cpu) {
    reset_bit_8bit(&cpu->h, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void res_0_l(Cpu* cpu) {
    reset_bit_8bit(&cpu->l, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void res_0_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    reset_bit_8bit(&value, 0);
    cpu->m = 4;
    cpu->t = 16;
}

void res_0_a(Cpu* cpu) {
    reset_bit_8bit(&cpu->a, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void res_1_b(Cpu* cpu) {
    reset_bit_8bit(&cpu->b, 1);
    cpu->m = 2;
    cpu->t = 8;
}

void res_1_c(Cpu* cpu) {
    reset_bit_8bit(&cpu->c, 1);
    cpu->m = 2;
    cpu->t = 8;
}

void res_1_d(Cpu* cpu) {
    reset_bit_8bit(&cpu->d, 1);
    cpu->m = 2;
    cpu->t = 8;
}

void res_1_e(Cpu* cpu) {
    reset_bit_8bit(&cpu->e, 1);
    cpu->m = 2;
    cpu->t = 8;
}

void res_1_h(Cpu* cpu) {
    reset_bit_8bit(&cpu->h, 1);
    cpu->m = 2;
    cpu->t = 8;
}

void res_1_l(Cpu* cpu) {
    reset_bit_8bit(&cpu->l, 1);
    cpu->m = 2;
    cpu->t = 8;
}

void res_1_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    reset_bit_8bit(&value, 1);
    cpu->m = 4;
    cpu->t = 16;
}

void res_1_a(Cpu* cpu) {
    reset_bit_8bit(&cpu->a, 1);
    cpu->m = 2;
    cpu->t = 8;
}

//9x
void res_2_b(Cpu* cpu) {
    reset_bit_8bit(&cpu->b, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void res_2_c(Cpu* cpu) {
    reset_bit_8bit(&cpu->c, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void res_2_d(Cpu* cpu) {
    reset_bit_8bit(&cpu->d, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void res_2_e(Cpu* cpu) {
    reset_bit_8bit(&cpu->e, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void res_2_h(Cpu* cpu) {
    reset_bit_8bit(&cpu->h, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void res_2_l(Cpu* cpu) {
    reset_bit_8bit(&cpu->l, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void res_2_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    reset_bit_8bit(&value, 2);
    cpu->m = 4;
    cpu->t = 16;
}

void res_2_a(Cpu* cpu) {
    reset_bit_8bit(&cpu->a, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void res_3_b(Cpu* cpu) {
    reset_bit_8bit(&cpu->b, 3);
    cpu->m = 2;
    cpu->t = 8;
}

void res_3_c(Cpu* cpu) {
    reset_bit_8bit(&cpu->c, 3);
    cpu->m = 2;
    cpu->t = 8;
}

void res_3_d(Cpu* cpu) {
    reset_bit_8bit(&cpu->d, 3);
    cpu->m = 2;
    cpu->t = 8;
}

void res_3_e(Cpu* cpu) {
    reset_bit_8bit(&cpu->e, 3);
    cpu->m = 2;
    cpu->t = 8;
}

void res_3_h(Cpu* cpu) {
    reset_bit_8bit(&cpu->h, 3);
    cpu->m = 2;
    cpu->t = 8;
}

void res_3_l(Cpu* cpu) {
    reset_bit_8bit(&cpu->l, 3);
    cpu->m = 2;
    cpu->t = 8;
}

void res_3_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    reset_bit_8bit(&value, 3);
    cpu->m = 4;
    cpu->t = 16;
}

void res_3_a(Cpu* cpu) {
    reset_bit_8bit(&cpu->a, 3);
    cpu->m = 2;
    cpu->t = 8;
}

//Ax
void res_4_b(Cpu* cpu) {
    reset_bit_8bit(&cpu->b, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void res_4_c(Cpu* cpu) {
    reset_bit_8bit(&cpu->c, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void res_4_d(Cpu* cpu) {
    reset_bit_8bit(&cpu->d, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void res_4_e(Cpu* cpu) {
    reset_bit_8bit(&cpu->e, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void res_4_h(Cpu* cpu) {
    reset_bit_8bit(&cpu->h, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void res_4_l(Cpu* cpu) {
    reset_bit_8bit(&cpu->l, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void res_4_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    reset_bit_8bit(&value, 4);
    cpu->m = 4;
    cpu->t = 16;
}

void res_4_a(Cpu* cpu) {
    reset_bit_8bit(&cpu->a, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void res_5_b(Cpu* cpu) {
    reset_bit_8bit(&cpu->b, 5);
    cpu->m = 2;
    cpu->t = 8;
}

void res_5_c(Cpu* cpu) {
    reset_bit_8bit(&cpu->c, 5);
    cpu->m = 2;
    cpu->t = 8;
}

void res_5_d(Cpu* cpu) {
    reset_bit_8bit(&cpu->d, 5);
    cpu->m = 2;
    cpu->t = 8;
}

void res_5_e(Cpu* cpu) {
    reset_bit_8bit(&cpu->e, 5);
    cpu->m = 2;
    cpu->t = 8;
}

void res_5_h(Cpu* cpu) {
    reset_bit_8bit(&cpu->h, 5);
    cpu->m = 2;
    cpu->t = 8;
}

void res_5_l(Cpu* cpu) {
    reset_bit_8bit(&cpu->l, 5);
    cpu->m = 2;
    cpu->t = 8;
}

void res_5_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    reset_bit_8bit(&value, 5);
    cpu->m = 4;
    cpu->t = 16;
}

void res_5_a(Cpu* cpu) {
    reset_bit_8bit(&cpu->a, 5);
    cpu->m = 2;
    cpu->t = 8;
}

//Bx
void res_6_b(Cpu* cpu) {
    reset_bit_8bit(&cpu->b, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void res_6_c(Cpu* cpu) {
    reset_bit_8bit(&cpu->c, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void res_6_d(Cpu* cpu) {
    reset_bit_8bit(&cpu->d, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void res_6_e(Cpu* cpu) {
    reset_bit_8bit(&cpu->e, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void res_6_h(Cpu* cpu) {
    reset_bit_8bit(&cpu->h, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void res_6_l(Cpu* cpu) {
    reset_bit_8bit(&cpu->l, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void res_6_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    reset_bit_8bit(&value, 6);
    cpu->m = 4;
    cpu->t = 16;
}

void res_6_a(Cpu* cpu) {
    reset_bit_8bit(&cpu->a, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void res_7_b(Cpu* cpu) {
    reset_bit_8bit(&cpu->b, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void res_7_c(Cpu* cpu) {
    reset_bit_8bit(&cpu->c, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void res_7_d(Cpu* cpu) {
    reset_bit_8bit(&cpu->d, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void res_7_e(Cpu* cpu) {
    reset_bit_8bit(&cpu->e, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void res_7_h(Cpu* cpu) {
    reset_bit_8bit(&cpu->h, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void res_7_l(Cpu* cpu) {
    reset_bit_8bit(&cpu->l, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void res_7_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    reset_bit_8bit(&value, 7);
    cpu->m = 4;
    cpu->t = 16;
}

void res_7_a(Cpu* cpu) {
    reset_bit_8bit(&cpu->a, 7);
    cpu->m = 2;
    cpu->t = 8;
}

//Cx
void set_bit_8bit(uint8_t* n, uint8_t bit_to_set) {
    if (bit_to_set > 7) {
        printf("Cannot set bit %d in an 8 bit number\n", bit_to_set);
    }
    uint8_t mask;
    mask = 1 << bit_to_set; 
    *n |= mask;
}

void set_0_b(Cpu* cpu) {
    set_bit_8bit(&cpu->b, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void set_0_c(Cpu* cpu) {
    set_bit_8bit(&cpu->c, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void set_0_d(Cpu* cpu) {
    set_bit_8bit(&cpu->d, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void set_0_e(Cpu* cpu) {
    set_bit_8bit(&cpu->e, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void set_0_h(Cpu* cpu) {
    set_bit_8bit(&cpu->h, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void set_0_l(Cpu* cpu) {
    set_bit_8bit(&cpu->l, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void set_0_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    set_bit_8bit(&value, 0);
    cpu->m = 4;
    cpu->t = 16;
}

void set_0_a(Cpu* cpu) {
    set_bit_8bit(&cpu->a, 0);
    cpu->m = 2;
    cpu->t = 8;
}

void set_1_b(Cpu* cpu) {
    set_bit_8bit(&cpu->b, 1);
    cpu->m = 2;
    cpu->t = 8;
}

void set_1_c(Cpu* cpu) {
    set_bit_8bit(&cpu->c, 1);
    cpu->m = 2;
    cpu->t = 8;
}

void set_1_d(Cpu* cpu) {
    set_bit_8bit(&cpu->d, 1);
    cpu->m = 2;
    cpu->t = 8;
}

void set_1_e(Cpu* cpu) {
    set_bit_8bit(&cpu->e, 1);
    cpu->m = 2;
    cpu->t = 8;
}

void set_1_h(Cpu* cpu) {
    set_bit_8bit(&cpu->h, 1);
    cpu->m = 2;
    cpu->t = 8;
}

void set_1_l(Cpu* cpu) {
    set_bit_8bit(&cpu->l, 1);
    cpu->m = 2;
    cpu->t = 8;
}

void set_1_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    set_bit_8bit(&value, 1);
    cpu->m = 4;
    cpu->t = 16;
}

void set_1_a(Cpu* cpu) {
    set_bit_8bit(&cpu->a, 1);
    cpu->m = 2;
    cpu->t = 8;
}

//Dx
void set_2_b(Cpu* cpu) {
    set_bit_8bit(&cpu->b, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void set_2_c(Cpu* cpu) {
    set_bit_8bit(&cpu->c, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void set_2_d(Cpu* cpu) {
    set_bit_8bit(&cpu->d, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void set_2_e(Cpu* cpu) {
    set_bit_8bit(&cpu->e, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void set_2_h(Cpu* cpu) {
    set_bit_8bit(&cpu->h, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void set_2_l(Cpu* cpu) {
    set_bit_8bit(&cpu->l, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void set_2_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    set_bit_8bit(&value, 2);
    cpu->m = 4;
    cpu->t = 16;
}

void set_2_a(Cpu* cpu) {
    set_bit_8bit(&cpu->a, 2);
    cpu->m = 2;
    cpu->t = 8;
}

void set_3_b(Cpu* cpu) {
    set_bit_8bit(&cpu->b, 3);
    cpu->m = 2;
    cpu->t = 8;
}

void set_3_c(Cpu* cpu) {
    set_bit_8bit(&cpu->c, 3);
    cpu->m = 2;
    cpu->t = 8;
}

void set_3_d(Cpu* cpu) {
    set_bit_8bit(&cpu->d, 3);
    cpu->m = 2;
    cpu->t = 8;
}

void set_3_e(Cpu* cpu) {
    set_bit_8bit(&cpu->e, 3);
    cpu->m = 2;
    cpu->t = 8;
}

void set_3_h(Cpu* cpu) {
    set_bit_8bit(&cpu->h, 3);
    cpu->m = 2;
    cpu->t = 8;
}

void set_3_l(Cpu* cpu) {
    set_bit_8bit(&cpu->l, 3);
    cpu->m = 2;
    cpu->t = 8;
}

void set_3_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    set_bit_8bit(&value, 3);
    cpu->m = 4;
    cpu->t = 16;
}

void set_3_a(Cpu* cpu) {
    set_bit_8bit(&cpu->a, 3);
    cpu->m = 2;
    cpu->t = 8;
}

//Ex
void set_4_b(Cpu* cpu) {
    set_bit_8bit(&cpu->b, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void set_4_c(Cpu* cpu) {
    set_bit_8bit(&cpu->c, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void set_4_d(Cpu* cpu) {
    set_bit_8bit(&cpu->d, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void set_4_e(Cpu* cpu) {
    set_bit_8bit(&cpu->e, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void set_4_h(Cpu* cpu) {
    set_bit_8bit(&cpu->h, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void set_4_l(Cpu* cpu) {
    set_bit_8bit(&cpu->l, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void set_4_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    set_bit_8bit(&value, 4);
    cpu->m = 4;
    cpu->t = 16;
}

void set_4_a(Cpu* cpu) {
    set_bit_8bit(&cpu->a, 4);
    cpu->m = 2;
    cpu->t = 8;
}

void set_5_b(Cpu* cpu) {
    set_bit_8bit(&cpu->b, 5);
    cpu->m = 2;
    cpu->t = 8;
}

void set_5_c(Cpu* cpu) {
    set_bit_8bit(&cpu->c, 5);
    cpu->m = 2;
    cpu->t = 8;
}

void set_5_d(Cpu* cpu) {
    set_bit_8bit(&cpu->d, 5);
    cpu->m = 2;
    cpu->t = 8;
}

void set_5_e(Cpu* cpu) {
    set_bit_8bit(&cpu->e, 5);
    cpu->m = 2;
    cpu->t = 8;
}

void set_5_h(Cpu* cpu) {
    set_bit_8bit(&cpu->h, 5);
    cpu->m = 2;
    cpu->t = 8;
}

void set_5_l(Cpu* cpu) {
    set_bit_8bit(&cpu->l, 5);
    cpu->m = 2;
    cpu->t = 8;
}

void set_5_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    set_bit_8bit(&value, 5);
    cpu->m = 4;
    cpu->t = 16;
}

void set_5_a(Cpu* cpu) {
    set_bit_8bit(&cpu->a, 5);
    cpu->m = 2;
    cpu->t = 8;
}

//Fx
void set_6_b(Cpu* cpu) {
    set_bit_8bit(&cpu->b, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void set_6_c(Cpu* cpu) {
    set_bit_8bit(&cpu->c, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void set_6_d(Cpu* cpu) {
    set_bit_8bit(&cpu->d, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void set_6_e(Cpu* cpu) {
    set_bit_8bit(&cpu->e, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void set_6_h(Cpu* cpu) {
    set_bit_8bit(&cpu->h, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void set_6_l(Cpu* cpu) {
    set_bit_8bit(&cpu->l, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void set_6_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    set_bit_8bit(&value, 6);
    cpu->m = 4;
    cpu->t = 16;
}

void set_6_a(Cpu* cpu) {
    set_bit_8bit(&cpu->a, 6);
    cpu->m = 2;
    cpu->t = 8;
}

void set_7_b(Cpu* cpu) {
    set_bit_8bit(&cpu->b, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void set_7_c(Cpu* cpu) {
    set_bit_8bit(&cpu->c, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void set_7_d(Cpu* cpu) {
    set_bit_8bit(&cpu->d, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void set_7_e(Cpu* cpu) {
    set_bit_8bit(&cpu->e, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void set_7_h(Cpu* cpu) {
    set_bit_8bit(&cpu->h, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void set_7_l(Cpu* cpu) {
    set_bit_8bit(&cpu->l, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void set_7_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu, address);
    set_bit_8bit(&value, 7);
    cpu->m = 4;
    cpu->t = 16;
}

void set_7_a(Cpu* cpu) {
    set_bit_8bit(&cpu->a, 7);
    cpu->m = 2;
    cpu->t = 8;
}

void prefix_cb(Cpu* cpu, uint8_t opcode) {
    //Move this to the end of file (near) if it's a big switch
    switch(opcode) {
        case 0x00:
            rlc_b(cpu);
            break;
        case 0x01:
            rlc_c(cpu);
            break;
        case 0x02:
            rlc_d(cpu);
            break;
        case 0x03:
            rlc_e(cpu);
            break;
        case 0x04:
            rlc_h(cpu);
            break;
        case 0x05:
            rlc_l(cpu);
            break;
        case 0x06:
            rlc_hl(cpu);
            break;
        case 0x07:
            rlc_a(cpu);
            break;
        case 0x08:
            rrc_b(cpu);
            break;
        case 0x09:
            rrc_c(cpu);
            break;
        case 0x0A:
            rrc_d(cpu);
            break;
        case 0x0B:
            rrc_e(cpu);
            break;
        case 0x0C:
            rrc_h(cpu);
            break;
        case 0x0D:
            rrc_l(cpu);
            break;
        case 0x0E:
            rrc_hl(cpu);
            break;
        case 0x0F:
            rrc_a(cpu);
            break;
        case 0x10:
            rl_b(cpu);
            break;
        case 0x11:
            rl_c(cpu);
            break;
        case 0x12:
            rl_d(cpu);
            break;
        case 0x13:
            rl_e(cpu);
            break;
        case 0x14:
            rl_h(cpu);
            break;
        case 0x15:
            rl_l(cpu);
            break;
        case 0x16:
            rl_hl(cpu);
            break;
        case 0x17:
            rl_a(cpu);
            break;
        case 0x18:
            rr_b(cpu);
            break;
        case 0x19:
            rr_c(cpu);
            break;
        case 0x1A:
            rr_d(cpu);
            break;
        case 0x1B:
            rr_e(cpu);
            break;
        case 0x1C:
            rr_h(cpu);
            break;
        case 0x1D:
            rr_l(cpu);
            break;
        case 0x1E:
            rr_hl(cpu);
            break;
        case 0x1F:
            rr_a(cpu);
            break;
        case 0x20:
            sla_b(cpu);
            break;
        case 0x21:
            sla_c(cpu);
            break;
        case 0x22:
            sla_d(cpu);
            break;
        case 0x23:
            sla_e(cpu);
            break;
        case 0x24:
            sla_h(cpu);
            break;
        case 0x25:
            sla_l(cpu);
            break;
        case 0x26:
            sla_hl(cpu);
            break;
        case 0x27:
            sla_a(cpu);
            break;
        case 0x28:
            sra_b(cpu);
            break;
        case 0x29:
            sra_c(cpu);
            break;
        case 0x2A:
            sra_d(cpu);
            break;
        case 0x2B:
            sra_e(cpu);
            break;
        case 0x2C:
            sra_h(cpu);
            break;
        case 0x2D:
            sra_l(cpu);
            break;
        case 0x2E:
            sra_hl(cpu);
            break;
        case 0x2F:
            sra_a(cpu);
            break;
        case 0x30:
            swap_b(cpu);
            break;
        case 0x31:
            swap_c(cpu);
            break;
        case 0x32:
            swap_d(cpu);
            break;
        case 0x33:
            swap_e(cpu);
            break;
        case 0x34:
            swap_h(cpu);
            break;
        case 0x35:
            swap_l(cpu);
            break;
        case 0x36:
            swap_hl(cpu);
            break;
        case 0x37:
            swap_a(cpu);
            break;
        case 0x38:
            srl_b(cpu);
            break;
        case 0x39:
            srl_c(cpu);
            break;
        case 0x3A:
            srl_d(cpu);
            break;
        case 0x3B:
            srl_e(cpu);
            break;
        case 0x3C:
            srl_h(cpu);
            break;
        case 0x3D:
            srl_l(cpu);
            break;
        case 0x3E:
            srl_hl(cpu);
            break;
        case 0x3F:
            srl_a(cpu);
            break;
        case 0x40:
            bit_0_b(cpu);
            break;
        case 0x41:
            bit_0_c(cpu);
            break;
        case 0x42:
            bit_0_d(cpu);
            break;
        case 0x43:
            bit_0_e(cpu);
            break;
        case 0x44:
            bit_0_h(cpu);
            break;
        case 0x45:
            bit_0_l(cpu);
            break;
        case 0x46:
            bit_0_hl(cpu);
            break;
        case 0x47:
            bit_0_a(cpu);
            break;
        case 0x48:
            bit_1_b(cpu);
            break;
        case 0x49:
            bit_1_c(cpu);
            break;
        case 0x4A:
            bit_1_d(cpu);
            break;
        case 0x4B:
            bit_1_e(cpu);
            break;
        case 0x4C:
            bit_1_h(cpu);
            break;
        case 0x4D:
            bit_1_l(cpu);
            break;
        case 0x4E:
            bit_1_hl(cpu);
            break;
        case 0x4F:
            bit_1_a(cpu);
            break;
        case 0x50:
            bit_2_b(cpu);
            break;
        case 0x51:
            bit_2_c(cpu);
            break;
        case 0x52:
            bit_2_d(cpu);
            break;
        case 0x53:
            bit_2_e(cpu);
            break;
        case 0x54:
            bit_2_h(cpu);
            break;
        case 0x55:
            bit_2_l(cpu);
            break;
        case 0x56:
            bit_2_hl(cpu);
            break;
        case 0x57:
            bit_2_a(cpu);
            break;
        case 0x58:
            bit_3_b(cpu);
            break;
        case 0x59:
            bit_3_c(cpu);
            break;
        case 0x5A:
            bit_3_d(cpu);
            break;
        case 0x5B:
            bit_3_e(cpu);
            break;
        case 0x5C:
            bit_3_h(cpu);
            break;
        case 0x5D:
            bit_3_l(cpu);
            break;
        case 0x5E:
            bit_3_hl(cpu);
            break;
        case 0x5F:
            bit_3_a(cpu);
            break;
        case 0x60:
            bit_4_b(cpu);
            break;
        case 0x61:
            bit_4_c(cpu);
            break;
        case 0x62:
            bit_4_d(cpu);
            break;
        case 0x63:
            bit_4_e(cpu);
            break;
        case 0x64:
            bit_4_h(cpu);
            break;
        case 0x65:
            bit_4_l(cpu);
            break;
        case 0x66:
            bit_4_hl(cpu);
            break;
        case 0x67:
            bit_4_a(cpu);
            break;
        case 0x68:
            bit_5_b(cpu);
            break;
        case 0x69:
            bit_5_c(cpu);
            break;
        case 0x6A:
            bit_5_d(cpu);
            break;
        case 0x6B:
            bit_5_e(cpu);
            break;
        case 0x6C:
            bit_5_h(cpu);
            break;
        case 0x6D:
            bit_5_l(cpu);
            break;
        case 0x6E:
            bit_5_hl(cpu);
            break;
        case 0x6F:
            bit_5_a(cpu);
            break;
        case 0x70:
            bit_6_b(cpu);
            break;
        case 0x71:
            bit_6_c(cpu);
            break;
        case 0x72:
            bit_6_d(cpu);
            break;
        case 0x73:
            bit_6_e(cpu);
            break;
        case 0x74:
            bit_6_h(cpu);
            break;
        case 0x75:
            bit_6_l(cpu);
            break;
        case 0x76:
            bit_6_hl(cpu);
            break;
        case 0x77:
            bit_6_a(cpu);
            break;
        case 0x78:
            bit_7_b(cpu);
            break;
        case 0x79:
            bit_7_c(cpu);
            break;
        case 0x7A:
            bit_7_d(cpu);
            break;
        case 0x7B:
            bit_7_e(cpu);
            break;
        case 0x7C:
            bit_7_h(cpu);
            break;
        case 0x7D:
            bit_7_l(cpu);
            break;
        case 0x7E:
            bit_7_hl(cpu);
            break;
        case 0x7F:
            bit_7_a(cpu);
            break;
        case 0x80:
            res_0_b(cpu);
            break;
        case 0x81:
            res_0_c(cpu);
            break;
        case 0x82:
            res_0_d(cpu);
            break;
        case 0x83:
            res_0_e(cpu);
            break;
        case 0x84:
            res_0_h(cpu);
            break;
        case 0x85:
            res_0_l(cpu);
            break;
        case 0x86:
            res_0_hl(cpu);
            break;
        case 0x87:
            res_0_a(cpu);
            break;
        case 0x88:
            res_1_b(cpu);
            break;
        case 0x89:
            res_1_c(cpu);
            break;
        case 0x8A:
            res_1_d(cpu);
            break;
        case 0x8B:
            res_1_e(cpu);
            break;
        case 0x8C:
            res_1_h(cpu);
            break;
        case 0x8D:
            res_1_l(cpu);
            break;
        case 0x8E:
            res_1_hl(cpu);
            break;
        case 0x8F:
            res_1_a(cpu);
            break;
        case 0x90:
            res_2_b(cpu);
            break;
        case 0x91:
            res_2_c(cpu);
            break;
        case 0x92:
            res_2_d(cpu);
            break;
        case 0x93:
            res_2_e(cpu);
            break;
        case 0x94:
            res_2_h(cpu);
            break;
        case 0x95:
            res_2_l(cpu);
            break;
        case 0x96:
            res_2_hl(cpu);
            break;
        case 0x97:
            res_2_a(cpu);
            break;
        case 0x98:
            res_3_b(cpu);
            break;
        case 0x99:
            res_3_c(cpu);
            break;
        case 0x9A:
            res_3_d(cpu);
            break;
        case 0x9B:
            res_3_e(cpu);
            break;
        case 0x9C:
            res_3_h(cpu);
            break;
        case 0x9D:
            res_3_l(cpu);
            break;
        case 0x9E:
            res_3_hl(cpu);
            break;
        case 0x9F:
            res_3_a(cpu);
            break;
        case 0xA0:
            res_4_b(cpu);
            break;
        case 0xA1:
            res_4_c(cpu);
            break;
        case 0xA2:
            res_4_d(cpu);
            break;
        case 0xA3:
            res_4_e(cpu);
            break;
        case 0xA4:
            res_4_h(cpu);
            break;
        case 0xA5:
            res_4_l(cpu);
            break;
        case 0xA6:
            res_4_hl(cpu);
            break;
        case 0xA7:
            res_4_a(cpu);
            break;
        case 0xA8:
            res_5_b(cpu);
            break;
        case 0xA9:
            res_5_c(cpu);
            break;
        case 0xAA:
            res_5_d(cpu);
            break;
        case 0xAB:
            res_5_e(cpu);
            break;
        case 0xAC:
            res_5_h(cpu);
            break;
        case 0xAD:
            res_5_l(cpu);
            break;
        case 0xAE:
            res_5_hl(cpu);
            break;
        case 0xAF:
            res_5_a(cpu);
            break;
        case 0xB0:
            res_6_b(cpu);
            break;
        case 0xB1:
            res_6_c(cpu);
            break;
        case 0xB2:
            res_6_d(cpu);
            break;
        case 0xB3:
            res_6_e(cpu);
            break;
        case 0xB4:
            res_6_h(cpu);
            break;
        case 0xB5:
            res_6_l(cpu);
            break;
        case 0xB6:
            res_6_hl(cpu);
            break;
        case 0xB7:
            res_6_a(cpu);
            break;
        case 0xB8:
            res_7_b(cpu);
            break;
        case 0xB9:
            res_7_c(cpu);
            break;
        case 0xBA:
            res_7_d(cpu);
            break;
        case 0xBB:
            res_7_e(cpu);
            break;
        case 0xBC:
            res_7_h(cpu);
            break;
        case 0xBD:
            res_7_l(cpu);
            break;
        case 0xBE:
            res_7_hl(cpu);
            break;
        case 0xBF:
            res_7_a(cpu);
            break;
        case 0xC0:
            set_0_b(cpu);
            break;
        case 0xC1:
            set_0_c(cpu);
            break;
        case 0xC2:
            set_0_d(cpu);
            break;
        case 0xC3:
            set_0_e(cpu);
            break;
        case 0xC4:
            set_0_h(cpu);
            break;
        case 0xC5:
            set_0_l(cpu);
            break;
        case 0xC6:
            set_0_hl(cpu);
            break;
        case 0xC7:
            set_0_a(cpu);
            break;
        case 0xC8:
            set_1_b(cpu);
            break;
        case 0xC9:
            set_1_c(cpu);
            break;
        case 0xCA:
            set_1_d(cpu);
            break;
        case 0xCB:
            set_1_e(cpu);
            break;
        case 0xCC:
            set_1_h(cpu);
            break;
        case 0xCD:
            set_1_l(cpu);
            break;
        case 0xCE:
            set_1_hl(cpu);
            break;
        case 0xCF:
            set_1_a(cpu);
            break;
        case 0xD0:
            set_2_b(cpu);
            break;
        case 0xD1:
            set_2_c(cpu);
            break;
        case 0xD2:
            set_2_d(cpu);
            break;
        case 0xD3:
            set_2_e(cpu);
            break;
        case 0xD4:
            set_2_h(cpu);
            break;
        case 0xD5:
            set_2_l(cpu);
            break;
        case 0xD6:
            set_2_hl(cpu);
            break;
        case 0xD7:
            set_2_a(cpu);
            break;
        case 0xD8:
            set_3_b(cpu);
            break;
        case 0xD9:
            set_3_c(cpu);
            break;
        case 0xDA:
            set_3_d(cpu);
            break;
        case 0xDB:
            set_3_e(cpu);
            break;
        case 0xDC:
            set_3_h(cpu);
            break;
        case 0xDD:
            set_3_l(cpu);
            break;
        case 0xDE:
            set_3_hl(cpu);
            break;
        case 0xDF:
            set_3_a(cpu);
            break;
        case 0xE0:
            set_4_b(cpu);
            break;
        case 0xE1:
            set_4_c(cpu);
            break;
        case 0xE2:
            set_4_d(cpu);
            break;
        case 0xE3:
            set_4_e(cpu);
            break;
        case 0xE4:
            set_4_h(cpu);
            break;
        case 0xE5:
            set_4_l(cpu);
            break;
        case 0xE6:
            set_4_hl(cpu);
            break;
        case 0xE7:
            set_4_a(cpu);
            break;
        case 0xE8:
            set_5_b(cpu);
            break;
        case 0xE9:
            set_5_c(cpu);
            break;
        case 0xEA:
            set_5_d(cpu);
            break;
        case 0xEB:
            set_5_e(cpu);
            break;
        case 0xEC:
            set_5_h(cpu);
            break;
        case 0xED:
            set_5_l(cpu);
            break;
        case 0xEE:
            set_5_hl(cpu);
            break;
        case 0xEF:
            set_5_a(cpu);
            break;
        case 0xF0:
            set_6_b(cpu);
            break;
        case 0xF1:
            set_6_c(cpu);
            break;
        case 0xF2:
            set_6_d(cpu);
            break;
        case 0xF3:
            set_6_e(cpu);
            break;
        case 0xF4:
            set_6_h(cpu);
            break;
        case 0xF5:
            set_6_l(cpu);
            break;
        case 0xF6:
            set_6_hl(cpu);
            break;
        case 0xF7:
            set_6_a(cpu);
            break;
        case 0xF8:
            set_7_b(cpu);
            break;
        case 0xF9:
            set_7_c(cpu);
            break;
        case 0xFA:
            set_7_d(cpu);
            break;
        case 0xFB:
            set_7_e(cpu);
            break;
        case 0xFC:
            set_7_h(cpu);
            break;
        case 0xFD:
            set_7_l(cpu);
            break;
        case 0xFE:
            set_7_hl(cpu);
            break;
        case 0xFF:
            set_7_a(cpu);
            break;
    }
    
    cpu->pc++;          //Every prefix cb call has the same length in bytes
}
