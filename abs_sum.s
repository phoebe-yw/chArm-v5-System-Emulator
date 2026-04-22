.arch armv8-a
.text
.align 2
.global abs_sum
.type abs_sum, %function

// Compute the sum of the absolute value of values in the array.
// x0: address of long array[32]
// x1: size of array (32)
abs_sum:
    movz    x2, #0              // Running sum.
    // Compute end = x0 + x1*8 via three doublings (avoids lsl, uses only ADD_RR).
    adds     x5, x1, x1          // x5 = 2*size
    adds     x5, x5, x5          // x5 = 4*size
    adds     x5, x5, x5          // x5 = 8*size  (byte count)
    adds     x5, x0, x5          // x5 = end address (x0 + 8*size)
.loop:
    ldur    x3, [x0, #0]        // Load element.
    add     x0, x0, #8          // Advance pointer (fills load-use gap before asr).
    asr     x4, x3, #63         // Sign mask: 0 if x3>=0, -1 if x3<0.
    eor     x3, x3, x4          // Conditional bit-flip.
    subs    x3, x3, x4          // Complete abs(x3). (+1 if was negative)
    adds    x2, x2, x3          // Accumulate.
    subs    xzr, x0, x5         // CMP: set flags from (x0 - end).
    b.ne    .loop               // Loop while x0 != end (1 misprediction at exit).
.done:
    adds    x0, xzr, x2         // Move sum into x0.
    ret
.size   abs_sum, .-abs_sum
