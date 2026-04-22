/**************************************************************************
 * C S 429 system emulator
 *
 * instr_Fetch.c - Fetch stage of instruction processing pipeline.
 **************************************************************************/

#include "hw_elts.h"
#include "instr.h"
#include "instr_pipeline.h"
#include "machine.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

extern machine_t guest;
extern uint64_t F_PC;

/*
 * Select PC logic.
 * STUDENT TO-DO:
 * Write the next PC to *current_PC.
 */
static comb_logic_t
select_PC(uint64_t pred_PC,                  // The predicted PC
          opcode_t D_opcode, uint64_t val_a, // Possible correction from RET
          uint64_t D_seq_succ,               // this is only used in CBZ/CBNZ EC
          opcode_t M_opcode, bool M_cond_val, // b.cond correction
          uint64_t seq_succ,                  // Possible correction from B.cond
          uint64_t *current_PC) {
    /*
     * Students: Please leave this code
     * at the top of this function.
     * You may modify below it.
     */
    if (D_opcode == OP_RET && val_a == RET_FROM_MAIN_ADDR) {
        *current_PC = 0; // PC can't be 0 normally.
        return;
    } else if (D_opcode == OP_RET) { // when instruction OP from X stage is RET (TODO EC TO ADD)
        *current_PC = val_a;
    } else if (M_opcode == OP_B_COND && !M_cond_val) { // when instruction OP from M stage is B.COND and COND == FALSE
        *current_PC = seq_succ;
    } else {
        *current_PC = pred_PC;
    }


#ifdef EC
    if ((M_opcode == OP_CBZ || M_opcode == OP_CBNZ) && !M_cond_val) {
        *current_PC = seq_succ;
    } else if (D_opcode == OP_BR || D_opcode == OP_BLR) {
        *current_PC = val_a;
    }
#endif 

    return;
}

/*
 * Predict PC logic. Conditional branches are predicted taken.
 * STUDENT TO-DO:
 * Write the predicted next PC to *predicted_PC
 * and the next sequential pc to *seq_succ.
 */
static comb_logic_t predict_PC(uint64_t current_PC, uint32_t insnbits,
                               opcode_t op, uint64_t *predicted_PC,
                               uint64_t *seq_succ) {
    /*
     * Students: Please leave this code
     * at the top of this function.
     * You may modify below it.
     */
    if (!current_PC) {
        return; // We use this to generate a halt instruction.
    }

    // get sequential instruction i.e. PC + 4
    *seq_succ = current_PC + 4;

#ifdef EC
    if (op == OP_CBZ) {
        *predicted_PC = current_PC + (bitfield_s64(insnbits, 5, 19) << 2);
        return;
    } else if (op == OP_CBNZ) {
        *predicted_PC = current_PC + (bitfield_s64(insnbits, 5, 19) << 2);
        return; 
    }
#endif

    if (op == OP_B || op == OP_BL) { // B1 format
        *predicted_PC = current_PC + (bitfield_s64(insnbits, 0, 26) << 2);
    } else if (op == OP_B_COND) { // B2 Format
        *predicted_PC = current_PC + (bitfield_s64(insnbits, 5, 19) << 2);
    } else { // all other instruction for now
        *predicted_PC = *seq_succ;
    }

    return;
}

/*
 * Helper function to recognize the aliased instructions:
 * LSL (RI/RR), LSR (RI/RR), CMP, CMN, and TST. We do this only to simplify the
 * implementations of the shift operations (rather than having
 * to implement UBFM in full).
 * STUDENT TO-DO
 */
static void fix_instr_aliases(uint32_t insnbits, opcode_t *op) {
    switch (*op) {
        case OP_UBFM:
            *op = bitfield_u32(insnbits, 10, 6) == 63 ? OP_LSR_RI : OP_LSL_RI;
            break;

        case OP_UBFMV:
            *op = bitfield_u32(insnbits, 10, 1) == 1 ? OP_LSR_RR : OP_LSL_RR;
            break;
            
        case OP_SUBS_RR:
            *op = bitfield_u32(insnbits, 0, 5) == 31 ? OP_CMP_RR : OP_SUBS_RR;
            break;

        case OP_ADDS_RR:
            *op = bitfield_u32(insnbits, 0, 5) == 31 ? OP_CMN_RR : OP_ADDS_RR;
            break;

        case OP_ANDS_RR:
            *op = bitfield_u32(insnbits, 0, 5) == 31 ? OP_TST_RR : OP_ANDS_RR;
            break;

#ifdef EC
        // EC opcodes
        case OP_CSNEG:
            *op = bitfield_u32(insnbits, 10, 2) == 0 ? OP_CSINV : OP_CSNEG;
            break;
        case OP_CSEL:
            *op = bitfield_u32(insnbits, 10, 2) == 1 ? OP_CSINC : OP_CSEL;
            break;
#endif

        default:
            break;
    }

    return;
}

/*
 * Fetch stage logic.
 * STUDENT TO-DO:
 * Implement the fetch stage.
 *
 * Use in as the input pipeline register,
 * and update the out pipeline register as output.
 * Additionally, update PC for the next
 * cycle's predicted PC.
 *
 * You will also need the following helper functions:
 * select_pc, predict_pc, and imem.
 */
comb_logic_t fetch_instr(f_instr_impl_t *in, d_instr_impl_t *out) {
    bool imem_err = 0;
    uint64_t current_PC = 0;

    // Student TODO: Comment this line back in and fill in parameters
    select_PC(in->pred_PC, X_out->op, X_out->val_a, X_out->multipurpose_val.seq_succ_PC,
              M_out->op, M_out->cond_holds, M_out->multipurpose_val.seq_succ_PC, &current_PC);
    
    /*
     * Students: This case is for generating HLT instructions
     * to stop the pipeline. Only write your code in the **else** case.
     */
    if (!current_PC || F_in->status == STAT_HLT) {
        out->insnbits = 0xD4400000U;
        out->op = OP_HLT;
        out->print_op = OP_HLT;
        out->format = ftable[out->op];
        imem_err = false; 
    } else {
        imem(current_PC, &out->insnbits, &imem_err);
        out->op = itable[bitfield_u32(out->insnbits, 21, 11)]; 
        out->print_op = out->op;
        
        if (out->op == OP_ERROR) {
            out->format = FORMAT_ERROR;
            out->print_op = OP_ERROR; 
        } else {
            fix_instr_aliases(out->insnbits, &out->op);
            out->print_op = out->op;
            out->format = ftable[out->op]; 
        }

        // find next pc
        predict_PC(current_PC, out->insnbits, out->op, &F_PC, &out->multipurpose_val.seq_succ_PC);

        if (out->op == OP_ADRP) { // for ADRP case
            out->multipurpose_val.adrp_val = current_PC & 0xFFFFFFFFFFFFF000;
        } else if (out->op == OP_LDUR || out->op == OP_STUR || out->op == OP_ERROR || out->op == OP_HLT) {
            out->multipurpose_val.correction_PC = current_PC;
        } 
    }

    if (imem_err || out->op == OP_ERROR) {
        in->status = STAT_INS;
        F_in->status = in->status;
    } else if (out->op == OP_HLT) {
        in->status = STAT_HLT;
        F_in->status = in->status;
    } else {
        in->status = STAT_AOK;
    }
    out->status = in->status;

    return;
}