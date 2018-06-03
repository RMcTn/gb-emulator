#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
    printf("Total T: %d, Total M: %d\n", cpu->total_t, cpu->total_m);
    

    printf("FLAGS\n");
    printf("Zero: %d ", cpu->f & ZERO_FLAG ? 1 : 0);
    printf("Subtraction: %d ", cpu->f & SUBTRACTION_FLAG ? 1 : 0);
    printf("Halfcarry: %d ", cpu->f & HALFCARRY_FLAG ? 1 : 0);
    printf("Carry: %d\n", cpu->f & CARRY_FLAG ? 1 : 0);
    
}

void reset_cpu(Cpu* cpu) {
    memset(cpu, 0, sizeof(Cpu));
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
    write_byte(cpu->memory, address, cpu->a);
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
    cpu->f = 0;
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
    write_byte(cpu->memory, address, cpu->sp);
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
    cpu->a = read_byte(cpu->memory, address);
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
    cpu->f = 0;
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
    write_byte(cpu->memory, address, cpu->a);
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
    cpu->a = read_byte(cpu->memory, address);
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
    write_byte(cpu->memory, address, cpu->a);
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
    cpu->a = read_byte(cpu->memory, address);
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
    uint16_t value = read_word(cpu->memory, n);
    cpu->sp = value;
    cpu->pc += 2;
    cpu->m = 3;
    cpu->t = 12;
}
//0x32
void ld_hldecrement_a(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    write_byte(cpu->memory, address, cpu->a);
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
    uint8_t value = read_byte(cpu->memory, address);
    increment_8bit_register(cpu, &value);
    cpu->m = 3;
    cpu->t = 12;
}
//0x35
void dec_hl(Cpu* cpu) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    uint8_t value = read_byte(cpu->memory, address);
    decrement_8bit_register(cpu, &value);
    cpu->m = 3;
    cpu->t = 12;
}
//0x36
void ld_hl_8bit_immediate(Cpu* cpu, uint8_t n) {
    uint16_t address = join_registers(cpu->h, cpu->l);
    write_byte(cpu->memory, address, n);
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
    if (!is_flag_set(cpu, CARRY_FLAG)) {
        //Don't jump
        cpu->pc++;
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
    uint8_t value = read_byte(cpu->memory, address);
    cpu->a = value;
    value++;
    write_byte(cpu->memory, address, value);
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
//0x70
void ld_hl_b(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    write_byte(cpu->memory, address, cpu->b);
    cpu->m = 2;
    cpu->t = 8;
}
//0x71
void ld_hl_c(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    write_byte(cpu->memory, address, cpu->c);
    cpu->m = 2;
    cpu->t = 8;
}
//0x72
void ld_hl_d(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    write_byte(cpu->memory, address, cpu->d);
    cpu->m = 2;
    cpu->t = 8;
}
//0x73
void ld_hl_e(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    write_byte(cpu->memory, address, cpu->e);
    cpu->m = 2;
    cpu->t = 8;
}
//0x74
void ld_hl_h(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    write_byte(cpu->memory, address, cpu->h);
    cpu->m = 2;
    cpu->t = 8;
}
//0x75
void ld_hl_l(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    write_byte(cpu->memory, address, cpu->l);
    cpu->m = 2;
    cpu->t = 8;
}
//0x76
//TODO
//0x77
void ld_hl_a(Cpu* cpu) {
    uint16_t address = (cpu->h << 8) | cpu->l;
    write_byte(cpu->memory, address, cpu->a);
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
    uint8_t value = read_byte(cpu->memory, address);
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
//0xC6
void add_a_8bit_immediate(Cpu* cpu, uint8_t n) {
    add_to_accumulator(cpu, n);
    cpu->m = 2;
    cpu->t = 8;
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
//Dx
//TODO:
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
//0xE9
void jp_hl(Cpu* cpu) {
    cpu->pc = join_registers(cpu->h, cpu->l);
    cpu->m = 1;
    cpu->t = 4;
}
//Fx
//TODO: