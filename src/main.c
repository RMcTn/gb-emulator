#include <stdio.h>
#include "cpu.h"
#include "memory.h"
#include "gpu.h"

void loop(Cpu* cpu) {
    //TODO: Move this to a function in cpu, get rid of function headers for opcodes
	
	//Pandocs says these values are used at program execution
	//Commented out for now as it does not seem to have made an impact yet
	//cpu->a = 0x01;
	//cpu->f = 0xB0;
	//cpu->b = 0x00;
	//cpu->c = 0x13;
	//cpu->d = 0x00;
	//cpu->e = 0xD8;
	//cpu->h = 0x01;
	//cpu->l = 0x4D;
	//cpu->sp = 0xFFFE;
    int i = 0;
    while (i < 500000) {
        printf("PC:0x%02hX\t", cpu->pc);
        uint8_t opcode = read_byte(cpu, cpu->pc++);
        printf("Op:0x%02hhX", opcode);
        uint16_t operand = read_word(cpu, cpu->pc);
        printf("\tOp 1st:0x%02hhX", (uint8_t)(operand >> 8));
        printf("\tOp 2nd:0x%02hhX", (uint8_t)operand);
        printf("\tOp both:0x%02X\n", operand);
        switch (opcode) {
            case 0x00:
                nop(cpu);
                break;
            case 0x01:
            {
                uint16_t immediate = read_word(cpu, cpu->pc);
                ld_BC_16bit_immediate(cpu, immediate);
                cpu->pc += 2;
                break;
            }
            case 0x02:
                ld_bc_a(cpu);
                break;
            case 0x03:
                inc_BC(cpu);
                break;
            case 0x04:
                inc_b(cpu);
                break;
            case 0x05:
                dec_b(cpu);
                break;
            case 0x06:
            {
                uint8_t immediate = read_word(cpu, cpu->pc);
                ld_b_8bit_immediate(cpu, immediate);
                cpu->pc++;
                break;
            }
            case 0x07:
                rlca(cpu);
                break;
            case 0x08:
            {
                uint16_t address = read_word(cpu, cpu->pc);
                ld_16bit_address_sp(cpu, address);
                cpu->pc += 2;
                break;
            }
            case 0x09:
                add_HL_BC(cpu);
                break;
            case 0x0A:
                ld_a_bc(cpu);
                break;
            case 0x0B:
                dec_BC(cpu);
                break;
            case 0x0C:
                inc_c(cpu);
                break;
            case 0x0D:
                dec_c(cpu);
                break;
            case 0x0E:
            {
                uint8_t immediate = read_byte(cpu, cpu->pc);
                ld_c_8bit_immediate(cpu, immediate);
                cpu->pc++;
                break;
            }
            case 0x0F:
                rrca(cpu);
                break;
            case 0x10:
                //TODO:
                //STOP opcode
                unimplemented_opcode(opcode);
                break;
            case 0x11:
            {
                uint16_t immediate = read_word(cpu, cpu->pc);
                ld_DE_16bit_immediate(cpu, immediate);
                cpu->pc += 2;
                break;
            }
            case 0x12:
                ld_de_a(cpu);
                break;
            case 0x13:
                inc_DE(cpu);
                break;
            case 0x14:
                inc_d(cpu);
                break;
            case 0x15:
                dec_d(cpu);
                break;
            case 0x16:
            {
                uint8_t immediate = read_byte(cpu, cpu->pc);
                ld_d_8bit_immediate(cpu, immediate);
                cpu->pc++;
                break;
            }
            case 0x17:
                rla(cpu);
                break;
            case 0x18:
            {
                int8_t immediate = read_byte(cpu, cpu->pc);
                jr_8bit_immediate(cpu, immediate);
                break;
            }
            case 0x19:
                add_HL_DE(cpu);
                break;
            case 0x1A:
                ld_a_de(cpu);
                break;
            case 0x1B:
                dec_DE(cpu);
                break;
            case 0x1C:
                inc_e(cpu);
                break;
            case 0x1D:
                dec_e(cpu);
                break;
            case 0x1E:
            {
                uint8_t immediate = read_byte(cpu, cpu->pc);
                ld_e_8bit_immediate(cpu, immediate);
                cpu->pc++;
                break;
            }
            case 0x1F:
                rra(cpu);
                break;
            case 0x20:
            {
                int8_t immediate = read_byte(cpu, cpu->pc);
                jr_nz_8bit_immediate(cpu, immediate);
                break;
            }
            case 0x21:
            {
                uint16_t immediate = read_word(cpu, cpu->pc);
                ld_HL_16bit_immediate(cpu, immediate);
                cpu->pc += 2;
                break;
            }
            case 0x22:
                ld_hlincrement_a(cpu);
                break;
            case 0x23:
                inc_HL(cpu);
                break;
            case 0x24:
                inc_h(cpu);
                break;
            case 0x25:
                dec_h(cpu);
                break;
            case 0x26:
            {
                uint8_t immediate = read_byte(cpu, cpu->pc);
                ld_h_8bit_immediate(cpu, immediate);
                cpu->pc++;
                break;
            }
            case 0x27:
                //TODO:
                //DAA
                unimplemented_opcode(opcode);
                break;
            case 0x28:
            {
                int8_t immediate = read_byte(cpu, cpu->pc);
                jr_z_8bit_immediate(cpu, immediate);
                break;
            }
            case 0x29:
                add_HL_HL(cpu);
                break;
            case 0x2A:
                ld_a_hlincrement(cpu);
                break;
            case 0x2B:
                dec_HL(cpu);
                break;
            case 0x2C:
                inc_l(cpu);
                break;
            case 0x2D:
                dec_l(cpu);
                break;
            case 0x2E:
            {
                uint8_t immediate = read_byte(cpu, cpu->pc);
                ld_l_8bit_immediate(cpu, immediate);
                cpu->pc++;
                break;
            }
            case 0x2F:
                cpl(cpu);
                break;
            case 0x30:
            {
                int8_t immediate = read_byte(cpu, cpu->pc);
                jr_nc_8bit_immediate(cpu, immediate);
                break;
            }
            case 0x31:
            {
                uint16_t immediate = read_word(cpu, cpu->pc);
                ld_sp_16bit_immediate(cpu, immediate);
                break;
            }
            case 0x32:
                ld_hldecrement_a(cpu);
                break;
            case 0x33:
                inc_sp(cpu);
                break;
            case 0x34:
                inc_hl(cpu);
                break;
            case 0x35:
                dec_hl(cpu);
                break;
            case 0x36:
            {
                uint8_t immediate = read_byte(cpu, cpu->pc);
                ld_hl_8bit_immediate(cpu, immediate);
                break;
            }
            case 0x37:
                scf(cpu);
                break;
            case 0x38:
            {
                int8_t immediate = read_byte(cpu, cpu->pc);
                jr_c_8bit_immediate(cpu, immediate);
                break;
            }
            case 0x39:
                add_HL_sp(cpu);
                break;
            case 0x3A:
                ld_a_hldecrement(cpu);
                break;
            case 0x3B:
                dec_sp(cpu);
                break;
            case 0x3C:
                inc_a(cpu);
                break;
            case 0x3D:
                dec_a(cpu);
                break;
            case 0x3E:
            {
                uint8_t immediate = read_byte(cpu, cpu->pc);
                ld_a_8bit_immediate(cpu, immediate);
                break;
            }
            case 0x3F:
                ccf(cpu);
                break;
            case 0x40:
                ld_b_b(cpu);
                break;
            case 0x41:
                ld_b_c(cpu);
                break;
            case 0x42:
                ld_b_d(cpu);
                break;
            case 0x43:
                ld_b_e(cpu);
                break;
            case 0x44:
                ld_b_h(cpu);
                break;
            case 0x45:
                ld_b_l(cpu);
                break;
            case 0x46:
                ld_b_hl(cpu);
                break;
            case 0x47:
                ld_b_a(cpu);
                break;
            case 0x48:
                ld_c_b(cpu);
                break;
            case 0x49:
                ld_c_c(cpu);
                break;
            case 0x4A:
                ld_c_d(cpu);
                break;
            case 0x4B:
                ld_c_e(cpu);
                break;
            case 0x4C:
                ld_c_h(cpu);
                break;
            case 0x4D:
                ld_c_l(cpu);
                break;
            case 0x4E:
                ld_c_hl(cpu);
                break;
            case 0x4F:
                ld_c_a(cpu);
                break;
            case 0x50:
                ld_d_b(cpu);
                break;
            case 0x51:
                ld_d_c(cpu);
                break;
            case 0x52:
                ld_d_d(cpu);
                break;
            case 0x53:
                ld_d_e(cpu);
                break;
            case 0x54:
                ld_d_h(cpu);
                break;
            case 0x55:
                ld_d_l(cpu);
                break;
            case 0x56:
                ld_d_hl(cpu);
                break;
            case 0x57:
                ld_d_a(cpu);
                break;
            case 0x58:
                ld_e_b(cpu);
                break;
            case 0x59:
                ld_e_c(cpu);
                break;
            case 0x5A:
                ld_e_d(cpu);
                break;
            case 0x5B:
                ld_e_e(cpu);
                break;
            case 0x5C:
                ld_e_h(cpu);
                break;
            case 0x5D:
                ld_e_l(cpu);
                break;
            case 0x5E:
                ld_e_hl(cpu);
                break;
            case 0x5F:
                ld_e_a(cpu);
                break;
            case 0x60:
                ld_h_b(cpu);
                break;
            case 0x61:
                ld_h_c(cpu);
                break;
            case 0x62:
                ld_h_d(cpu);
                break;
            case 0x63:
                ld_h_e(cpu);
                break;
            case 0x64:
                ld_h_h(cpu);
                break;
            case 0x65:
                ld_h_l(cpu);
                break;
            case 0x66:
                ld_h_hl(cpu);
                break;
            case 0x67:
                ld_h_a(cpu);
                break;
            case 0x68:
                ld_l_b(cpu);
                break;
            case 0x69:
                ld_l_c(cpu);
                break;
            case 0x6A:
                ld_l_d(cpu);
                break;
            case 0x6B:
                ld_l_e(cpu);
                break;
            case 0x6C:
                ld_l_h(cpu);
                break;
            case 0x6D:
                ld_l_l(cpu);
                break;
            case 0x6E:
                ld_l_hl(cpu);
                break;
            case 0x6F:
                ld_l_a(cpu);
                break;
            case 0x70:
                ld_hl_b(cpu);
                break;
            case 0x71:
                ld_hl_c(cpu);
                break;
            case 0x72:
                ld_hl_d(cpu);
                break;
            case 0x73:
                ld_hl_e(cpu);
                break;
            case 0x74:
                ld_hl_h(cpu);
                break;
            case 0x75:
                ld_hl_l(cpu);
                break;
            case 0x76:
                //TODO:
                //HALT
                unimplemented_opcode(opcode);
                break;
            case 0x77:
                ld_hl_a(cpu);
                break;
            case 0x78:
                ld_a_b(cpu);
                break;
            case 0x79:
                ld_a_c(cpu);
                break;
            case 0x7A:
                ld_a_d(cpu);
                break;
            case 0x7B:
                ld_a_e(cpu);
                break;
            case 0x7C:
                ld_a_h(cpu);
                break;
            case 0x7D:
                ld_a_l(cpu);
                break;
            case 0x7E:
                ld_a_hl(cpu);
                break;
            case 0x7F:
                ld_a_a(cpu);
                break;
            case 0x80:
                add_a_b(cpu);
                break;
            case 0x81:
                add_a_c(cpu);
                break;
            case 0x82:
                add_a_d(cpu);
                break;
            case 0x83:
                add_a_e(cpu);
                break;
            case 0x84:
                add_a_h(cpu);
                break;
            case 0x85:
                add_a_l(cpu);
                break;
            case 0x86:
                add_a_hl(cpu);
                break;
            case 0x87:
                add_a_a(cpu);
                break;
            case 0x88:
                adc_a_b(cpu);
                break;
            case 0x89:
                adc_a_c(cpu);
                break;
            case 0x8A:
                adc_a_d(cpu);
                break;
            case 0x8B:
                adc_a_e(cpu);
                break;
            case 0x8C:
                adc_a_h(cpu);
                break;
            case 0x8D:
                adc_a_l(cpu);
                break;
            case 0x8E:
                adc_a_hl(cpu);
                break;
            case 0x8F:
                adc_a_a(cpu);
                break;
            case 0x90:
                sub_b(cpu);
                break;
            case 0x91:
                sub_c(cpu);
                break;
            case 0x92:
                sub_d(cpu);
                break;
            case 0x93:
                sub_e(cpu);
                break;
            case 0x94:
                sub_h(cpu);
                break;
            case 0x95:
                sub_l(cpu);
                break;
            case 0x96:
                sub_hl(cpu);
                break;
            case 0x97:
                sub_a(cpu);
                break;
            case 0x98:
                sbc_a_b(cpu);
                break;
            case 0x99:
                sbc_a_c(cpu);
                break;
            case 0x9A:
                sbc_a_d(cpu);
                break;
            case 0x9B:
                sbc_a_e(cpu);
                break;
            case 0x9C:
                sbc_a_h(cpu);
                break;
            case 0x9D:
                sbc_a_l(cpu);
                break;
            case 0x9E:
                sbc_a_hl(cpu);
                break;
            case 0x9F:
                sbc_a_a(cpu);
                break;
            case 0xA0:
                and_b(cpu);
                break;
            case 0xA1:
                and_c(cpu);
                break;
            case 0xA2:
                and_d(cpu);
                break;
            case 0xA3:
                and_e(cpu);
                break;
            case 0xA4:
                and_h(cpu);
                break;
            case 0xA5:
                and_l(cpu);
                break;
            case 0xA6:
                and_hl(cpu);
                break;
            case 0xA7:
                and_a(cpu);
                break;
            case 0xA8:
                xor_b(cpu);
                break;
            case 0xA9:
                xor_c(cpu);
                break;
            case 0xAA:
                xor_d(cpu);
                break;
            case 0xAB:
                xor_e(cpu);
                break;
            case 0xAC:
                xor_h(cpu);
                break;
            case 0xAD:
                xor_l(cpu);
                break;
            case 0xAE:
                xor_hl(cpu);
                break;
            case 0xAF:
                xor_a(cpu);
                break;
            case 0xB0:
                or_b(cpu);
                break;
            case 0xB1:
                or_c(cpu);
                break;
            case 0xB2:
                or_d(cpu);
                break;
            case 0xB3:
                or_e(cpu);
                break;
            case 0xB4:
                or_h(cpu);
                break;
            case 0xB5:
                or_l(cpu);
                break;
            case 0xB6:
                or_hl(cpu);
                break;
            case 0xB7:
                or_a(cpu);
                break;
            case 0xB8:
                cp_b(cpu);
                break;
            case 0xB9:
                cp_c(cpu);
                break;
            case 0xBA:
                cp_d(cpu);
                break;
            case 0xBB:
                cp_e(cpu);
                break;
            case 0xBC:
                cp_h(cpu);
                break;
            case 0xBD:
                cp_l(cpu);
                break;
            case 0xBE:
                cp_hl(cpu);
                break;
            case 0xBF:
                cp_a(cpu);
                break;
            case 0xC0:
                ret_nz(cpu);
                break;
            case 0xC1:
                pop_BC(cpu);
                break;
            case 0xC2:
                unimplemented_opcode(opcode);
                break;
            case 0xC3:
                {
                    uint16_t address = read_word(cpu, cpu->pc);
                    jp_16bit_immediate(cpu, address);
                    break;
                }
            case 0xC4:
                unimplemented_opcode(opcode);
                break;
            case 0xC5:
                push_BC(cpu);
                break;
            case 0xC6:
                {
                    uint8_t immediate = read_byte(cpu, cpu->pc);
                    add_a_8bit_immediate(cpu, immediate);
                    cpu->pc++;
                    break;
                }
            case 0xC7:
                unimplemented_opcode(opcode);
                break;
            case 0xC8:
                ret_z(cpu);
                break;
            case 0xC9:
                ret(cpu);
                break;
            case 0xCA:
                {
                    uint16_t immediate = read_word(cpu, cpu->pc);
					jp_z_16bit_immediate(cpu, immediate);
                    break;
                }
            case 0xCB:
                {
                    uint8_t opcode = read_byte(cpu, cpu->pc);
                    prefix_cb(cpu, opcode);
                    break;
                }
            case 0xCC:
                unimplemented_opcode(opcode);
                break;
            case 0xCD:
            {
                uint16_t immediate = read_word(cpu, cpu->pc);
                call_16bit_immediate(cpu, immediate);
                break;
            }
            case 0xCE:
                unimplemented_opcode(opcode);
                break;
            case 0xCF:
                unimplemented_opcode(opcode);
                break;
            case 0xD0:
				ret_nc(cpu);
                break;
            case 0xD1:
                pop_DE(cpu);
                break;
            case 0xD2:
                unimplemented_opcode(opcode);
                break;
            case 0xD3:
                unimplemented_opcode(opcode);
                break;
            case 0xD4:
                unimplemented_opcode(opcode);
                break;
            case 0xD5:
                push_DE(cpu);
                break;
            case 0xD6:
                unimplemented_opcode(opcode);
                break;
            case 0xD7:
                unimplemented_opcode(opcode);
                break;
            case 0xD8:
                ret_c(cpu);
                break;
            case 0xD9:
                reti(cpu);
                break;
            case 0xDA:
                unimplemented_opcode(opcode);
                break;
            case 0xDB:
                unimplemented_opcode(opcode);
                break;
            case 0xDC:
                unimplemented_opcode(opcode);
                break;
            case 0xDD:
                unimplemented_opcode(opcode);
                break;
            case 0xDE:
                unimplemented_opcode(opcode);
                break;
            case 0xDF:
                unimplemented_opcode(opcode);
                break;
            case 0xE0:
            {
                uint8_t immediate = read_byte(cpu, cpu->pc);
                ldh_8bit_immediate_a(cpu, immediate);
                break;
            }
            case 0xE1:
                pop_HL(cpu);
                break;
            case 0xE2:
				ld_C_a(cpu);
                break;
            case 0xE3:
                unimplemented_opcode(opcode);
                break;
            case 0xE4:
                //No opcode here
                break;
            case 0xE5:
                push_HL(cpu);
                break;
            case 0xE6:
            {
                uint8_t immediate = read_byte(cpu, cpu->pc);
				and_8bit_immediate(cpu, immediate);
                break;
            }
            case 0xE7:
                unimplemented_opcode(opcode);
                break;
            case 0xE8:
                unimplemented_opcode(opcode);
                break;
            case 0xE9:
				jp_hl(cpu);
                break;
            case 0xEA:
            {
                uint16_t immediate = read_word(cpu, cpu->pc);
				ld_16_bit_immediate_a(cpu, immediate);
                break;
            }

                break;
            case 0xEB:
                unimplemented_opcode(opcode);
                break;
            case 0xEC:
                unimplemented_opcode(opcode);
                break;
            case 0xED:
                unimplemented_opcode(opcode);
                break;
            case 0xEE:
                unimplemented_opcode(opcode);
                break;
            case 0xEF:
				rst_28(cpu);
                break;
            case 0xF0:
            {
                uint8_t immediate = read_byte(cpu, cpu->pc);
                ldh_a_8bit_immediate(cpu, immediate);
                break;
            }
            case 0xF1:
                pop_AF(cpu);
                break;
            case 0xF2:
                unimplemented_opcode(opcode);
                break;
            case 0xF3:
                di(cpu);
                break;
            case 0xF4:
                unimplemented_opcode(opcode);
                break;
            case 0xF5:
                push_AF(cpu);
                break;
            case 0xF6:
                unimplemented_opcode(opcode);
                break;
            case 0xF7:
                unimplemented_opcode(opcode);
                break;
            case 0xF8:
                unimplemented_opcode(opcode);
                break;
            case 0xF9:
                unimplemented_opcode(opcode);
                break;
            case 0xFA:
            {
                uint16_t address = read_word(cpu, cpu->pc);
                ld_a_16bit_address(cpu, address);
                break;
            }
            case 0xFB:
                ei(cpu);
                break;
            case 0xFC:
                unimplemented_opcode(opcode);
                break;
            case 0xFD:
                unimplemented_opcode(opcode);
                break;
            case 0xFE:
            {
                uint8_t immediate = read_byte(cpu, cpu->pc);
                cp_8bit_immediate(cpu, immediate);
                break;
            }
            case 0xFF:
                unimplemented_opcode(opcode);
                break;
        }
        cpu->total_m += cpu->m;
        cpu->total_t += cpu->t;

        uint8_t interrupts_to_set = gpu_step(&cpu->gpu, cpu->t);
        cpu->interrupt_flags |= interrupts_to_set;

        if (cpu->interrupt_master_enable && cpu->interrupt_enable && cpu->interrupt_flags) {
            //Get the interrupts which are actually enabled, and have been set
            uint8_t interrupts = cpu->interrupt_enable & cpu->interrupt_flags;
            if (interrupts & 0x01) {
                cpu->interrupt_flags &= ~(0x01);
                rst_40(cpu);
            }
        }

        i++;
        
    }
    
}

int main(int argc, char** argv) {
    Cpu cpu;
    reset_cpu(&cpu);

    if (argc == 2) {
        //Load rom
        FILE* rom = fopen(argv[1], "r");
        if (rom == NULL) {
            printf("Could not open %s\n", argv[1]);
            return 1;
        } 
        fseek(rom, 0, SEEK_END);
        long filelength = ftell(rom);
        rewind(rom);
        printf("filelength %ld\n", filelength);
        fread(cpu.memory, filelength, 1, rom);

        fclose(rom);
    }
    
    //execution stats at 0x100
    cpu.pc = 0x100;
    loop(&cpu);
    return 0;
}
