.arch armv8-a
.text
.align 2
.global abs_sum
.type abs_sum, %function

// Compute the sum of the absolute value of values in the array.
// x0: address of long array[32]
// x1: size of array (32)
abs_sum:
    // Modify below here
    movz    x2, #0              // Running sum.
    movz    x5, #1              // Constant 1 for decrement (SUBS_RI not in ISA).
.loop:
    ldur    x3, [x0, #0]        // Load element from memory.
    add     x0, x0, #8          // Advance pointer (fills load-use gap for asr below).
    asr     x4, x3, #63         // Sign mask: 0 if x3 >= 0, -1 (0xFFFF...) if x3 < 0.
    eor     x3, x3, x4          // Flip bits if negative (no flags set).
    subs    x3, x3, x4          // Add 1 if negative: abs(x3). Flags overridden below.
    adds    x2, x2, x3          // Accumulate abs value. Flags overridden below.
    subs    x1, x1, x5          // Decrement counter, set flags for loop check.
    b.ne    .loop               // Branch if x1 != 0 (predicted taken, 1 misprediction).
.done:
    adds    x0, xzr, x2         // Move sum into x0.
    ret
.size   abs_sum, .-abs_sum
