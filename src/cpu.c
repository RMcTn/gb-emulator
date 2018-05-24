#include <stdint.h>
#include <stdio.h>

#define CARRY_FLAG          0x10        /* Last operation result was over 255 (addition)
                                           or under 0 (subtraction) */
#define HALFCARRY_FLAG      0x20        /* Last operation's result's lower half of 
                                           byte overflowed past 15 */
#define SUBTRACTION_FLAG    0x40        //Last operation was a subtraction
#define ZERO_FLAG           0x80        //Last operation had result of 0

typedef struct Cpu {
    uint8_t a;                      //Accumulator register
    uint8_t f;                      //Flag register (lower 4 bits are always 0)
    uint8_t b, c, d, e, h, l;       //8 bit registers 
    uint16_t sp, pc;               //16 bit registers
    uint8_t m, t;                  //clocks for last instruction
    //t increments with each clock step, m being a quarter of t

} Cpu;

void set_carry_flag(Cpu* cpu) {
    cpu->f |= CARRY_FLAG;
}

void clear_carry_flag(Cpu* cpu) {
    cpu->f &= ~CARRY_FLAG;
}

void set_zero_flag(Cpu* cpu) {
    cpu->f |= ZERO_FLAG;
}

void clear_zero_flag(Cpu* cpu) {
    cpu->f &= ~ZERO_FLAG;
}

void set_half_carry_flag(Cpu* cpu) {
    cpu->f |= HALFCARRY_FLAG;
}

void clear_half_carry_flag(Cpu* cpu) {
    cpu->f &= ~HALFCARRY_FLAG;
}

void set_subtraction_flag(Cpu* cpu) {
    cpu->f |= SUBTRACTION_FLAG;
}

void clear_subtraction_flag(Cpu* cpu) {
    cpu->f &= ~SUBTRACTION_FLAG;
}

void add_a_8bit_constant(Cpu* cpu, uint8_t n) {
    clear_zero_flag(cpu);
    if (cpu->a + n > UINT8_MAX)
        set_carry_flag(cpu);
    cpu->a += n;
    if (cpu->a == 0)
        set_zero_flag(cpu);
    cpu->m = 2;
    cpu->t = 8;
}
void add_a_a(Cpu* cpu) {
    clear_zero_flag(cpu);
    if (cpu->a + cpu->a > UINT8_MAX)
        set_carry_flag(cpu);
    cpu->a += cpu->a;
    if (cpu->a == 0)
        set_zero_flag(cpu);
    cpu->m = 1;
    cpu->t = 4;
}

void add_a_b(Cpu* cpu) {
    clear_zero_flag(cpu);
    if (cpu->a + cpu->b > UINT8_MAX)
        set_carry_flag(cpu);
    cpu->a += cpu->b;
    if (cpu->a == 0)
        set_zero_flag(cpu);
    cpu->m = 1;
    cpu->t = 4;
}

void add_a_c(Cpu* cpu) {
    clear_zero_flag(cpu);
    if (cpu->a + cpu->c > UINT8_MAX)
        set_carry_flag(cpu);
    cpu->a += cpu->c;
    if (cpu->a == 0)
        set_zero_flag(cpu);
    cpu->m = 1;
    cpu->t = 4;
}

void add_a_d(Cpu* cpu) {
    clear_zero_flag(cpu);
    if (cpu->a + cpu->d > UINT8_MAX)
        set_carry_flag(cpu);
    cpu->a += cpu->d;
    if (cpu->a == 0)
        set_zero_flag(cpu);
    cpu->m = 1;
    cpu->t = 4;
}

void add_a_e(Cpu* cpu) {
    clear_zero_flag(cpu);
    if (cpu->a + cpu->e > UINT8_MAX)
        set_carry_flag(cpu);
    cpu->a += cpu->e;
    if (cpu->a == 0)
        set_zero_flag(cpu);
    cpu->m = 1;
    cpu->t = 4;
}

void add_a_h(Cpu* cpu) {
    clear_zero_flag(cpu);
    if (cpu->a + cpu->h > UINT8_MAX)
        set_carry_flag(cpu);
    cpu->a += cpu->h;
    if (cpu->a == 0)
        set_zero_flag(cpu);
    cpu->m = 1;
    cpu->t = 4;
}

void add_a_l(Cpu* cpu) {
    clear_zero_flag(cpu);
    if (cpu->a + cpu->l > UINT8_MAX)
        set_carry_flag(cpu);
    cpu->a += cpu->l;
    if (cpu->a == 0)
        set_zero_flag(cpu);
    cpu->m = 1;
    cpu->t = 4;
}