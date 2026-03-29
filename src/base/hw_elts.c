/**************************************************************************
 * C S 429 system emulator
 *
 * hw_elts.c - Module for emulating hardware elements.
 *
 * Copyright (c) 2022, 2023, 2024, 2025.
 * Authors: S. Chatterjee, Z. Leeper., P. Jamadagni, W. Borden.
 * All rights reserved.
 * May not be used, modified, or copied without permission.
 **************************************************************************/

#include "hw_elts.h"
#include "err_handler.h"
#include "instr.h"
#include "instr_pipeline.h"
#include "machine.h"
#include "mem.h"
#include <assert.h>
#include <stdint.h>

extern machine_t guest;

fa_t rca[64];

/*
 * Read from instruction memory. Students, do not modify.
 */
comb_logic_t imem(uint64_t imem_addr, uint32_t *imem_rval, bool *imem_err) {
    // imem_addr must be in "instruction memory" and a multiple of 4
    *imem_err = (!addr_in_imem(imem_addr) || (imem_addr & 0x3U));
    *imem_rval = (uint32_t) mem_read_I(imem_addr);
}

/*
 * Sets up the inputs to the ripple carry adder.
 * STUDENT TO-DO:
 */
comb_logic_t init_rca(uint64_t val_a, uint64_t val_b, bool c_in) {
    rca[0].c_in = c_in;
    for (int i = 0; i < 64; i++) {
        rca[i].bit_a = (val_a >> i) & 1;
        rca[i].bit_b = (val_b >> i) & 1;
    }

    return;
}

/*
 * Performs the ripple carry add.
 * STUDENT TO-DO:
 */
comb_logic_t ripple_carry_add(uint64_t *sum) {
    // compute first full adder
    rca[0].s = rca[0].bit_a ^ rca[0].bit_b ^ rca[0].c_in;
    rca[0].c_out = (rca[0].bit_a & rca[0].bit_b) | 
                       (rca[0].c_in & (rca[0].bit_a ^ rca[0].bit_b));

    // computes the rest
    for (int i = 1; i < 64; i++) {
        rca[i].c_in = rca[i - 1].c_out;
        rca[i].s = rca[i].bit_a ^ rca[i].bit_b ^ rca[i].c_in;
        rca[i].c_out = (rca[i].bit_a & rca[i].bit_b) | 
                       (rca[i].c_in & (rca[i].bit_a ^ rca[i].bit_b));
    }

    uint64_t build_sum = 0;
    for (int i = 0; i < 64; i++) {
        build_sum |= rca[i].s << i;
    }
    *sum = build_sum;

    return;
}

/*
 * Read from register file.
 * STUDENT TO-DO:
 * Read from src1 and src2 registers. Take extra care for SP/XZR.
 */
comb_logic_t regfile_read(uint8_t src1, uint8_t src2, uint64_t *val_a,
                          uint64_t *val_b) {
    if (src1 == 32) {
        *val_a = 0;
    } else if (src1 == 31) {
        *val_a = guest.proc->SP;
    } else {
        *val_a = guest.proc->GPR[src1];
    }

    if (src2 == 32) {
        *val_b = 0;
    } else if (src2 == 31) {
        *val_b = guest.proc->SP;
    } else {
        *val_b = guest.proc->GPR[src2];
    }

    return;
}

/*
 * Write to register file.
 * STUDENT TO-DO:
 * Write to dst register if enabled. Take extra care for SP/XZR.
 */
comb_logic_t regfile_write(uint8_t dst, uint64_t val_w, bool w_enable) {
    // TODO check with TA what w_enable means (writing to w-registers or write_enable)
    if (w_enable && dst != 32) {
        if (dst == 31) {
            guest.proc->SP = val_w;
        } else {
            guest.proc->GPR[dst] = val_w;
        }
    }
    
    return;
}

/*
 * Check whether a condition is satisfied given the NCZV status flags.
 * STUDENT TO-DO:
 */
static bool cond_holds(cond_t cond, uint8_t flags) {
    bool ret_boolean = false;
    bool nflag = GET_NF(flags);
    bool zflag = GET_ZF(flags);
    bool cflag = GET_CF(flags);
    bool vflag = GET_VF(flags);
    
    switch (cond) {
        case C_EQ:
            ret_boolean = zflag;
            break;
        case C_NE:
            ret_boolean = !zflag;
            break;
        case C_CS:
            ret_boolean = cflag;
            break;
        case C_CC:
            ret_boolean = !cflag;
            break;
        case C_MI:
            ret_boolean = nflag;
            break;
        case C_PL:
            ret_boolean = !nflag;
            break;
        case C_VS:
            ret_boolean = vflag;
            break;
        case C_VC:
            ret_boolean = !vflag;
            break;
        case C_HI:
            ret_boolean = cflag && !zflag;
            break;
        case C_LS:
            ret_boolean = !(cflag && !zflag); 
            break;
        case C_GE:
            ret_boolean = nflag == vflag;
            break;
        case C_LT:
            ret_boolean = !(nflag == vflag);
            break;
        case C_GT:
            ret_boolean = !zflag && (nflag == vflag);
            break;
        case C_LE:
            ret_boolean = !(!zflag && (nflag == vflag));
            break;
        case C_AL:
        case C_NV:
            ret_boolean = true;
            break;
        default:
            ret_boolean = false; 
    }

    return ret_boolean;
}

/*
 * Perform the appropriate ALU operation, setting NZCV flags if needed.
 * STUDENT TO-DO:
 */
comb_logic_t alu(uint64_t alu_vala, uint64_t alu_valb, uint8_t alu_valhw,
                 uint8_t nzcv, alu_op_t ALUop, bool set_flags, cond_t cond,
                 uint64_t *val_e, bool *cond_val, uint8_t *nzcv_dst) {
    *cond_val = cond_holds(cond, nzcv);

    switch (ALUop) {
        case PLUS_OP:
            init_rca(alu_vala, alu_valb, 0);
            ripple_carry_add(val_e);
            printf("%ld", *val_e);
            break;
        case MINUS_OP:
            break;
        case INV_OP:
            break;
        case OR_OP:
            break;
        case EOR_OP:
            break;
        case AND_OP:
            break;
        case MOV_OP:
            break;
        case MOVK_OP:
            break;
        case LSL_OP:
            break;
        case LSR_OP:
            break;
        case ASR_OP:
            break;
        case PASS_A_OP:
            break;
        default:
            return;
    }
    return;
    
}

/*
 * Read from data memory, Students do not modify.
 */
comb_logic_t dmem(uint64_t dmem_addr, uint64_t dmem_wval, bool dmem_read,
                  bool dmem_write, uint64_t *dmem_rval, bool *dmem_err) {
    if (!dmem_read && !dmem_write) {
        return;
    }
    // dmem_addr must be in "data memory" and a multiple of 8
    *dmem_err = (!addr_in_dmem(dmem_addr) || (dmem_addr & 0x7U));
    if (is_special_addr(dmem_addr))
        *dmem_err = false;
    if (dmem_read)
        *dmem_rval = (uint64_t) mem_read_L(dmem_addr);
    if (dmem_write)
        mem_write_L(dmem_addr, dmem_wval);
}
