/**************************************************************************
 * C S 429 architecture emulator
 *
 * instr_Writeback.c - Writeback stage of instruction processing pipeline.
 **************************************************************************/

#include "instr.h"
#include "instr_pipeline.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

extern int64_t W_wval;
extern comb_logic_t regfile_write(uint8_t dst, uint64_t val_w, bool w_enable);

/*
 * Write-back stage logic.
 * STUDENT TO-DO:
 * Implement the writeback stage.
 *
 * Use in as the input pipeline register.
 *
 * You will need the global variable W_wval.
 */
comb_logic_t wback_instr(w_instr_impl_t *in) {
    uint8_t act_dst = in->W_sigs.dst_sel ? 30 : in->dst;
    W_wval = in->W_sigs.wval_sel ? in->val_mem : in->val_ex;
    regfile_write(act_dst, W_wval, in->W_sigs.w_enable);
    return;
}
