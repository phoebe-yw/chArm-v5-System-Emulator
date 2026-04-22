.arch armv8-a
.text
.align 2
.global abs_sum
.type abs_sum, %function

// Compute the sum of the absolute value of values in the array.
// x0: address of long array[32]
// x1: size of array (32)
abs_sum:
    movz    x2, #0
    movz    x9, #1
    asr     x5, x1, #1          // x5 = N/2 (pair count)
    adds    x8, x5, x5          // x8 = 2*(N/2)  [for parity test]
    subs    x6, x1, x8          // x6 = N mod 2  (0=even, 1=odd)
    adds    xzr, xzr, x5        // set Z if x5==0 (handles N=0 and N=1)
    b.eq    .handle_odd         // skip pair loop when no pairs
.loop2:
    ldur    x3, [x0, #0]        // load elem i
    ldur    x7, [x0, #8]        // load elem i+1 (no dep on x3)
    add     x0, x0, #16         // advance ptr (fills load-use gap for x7; x3 gap filled by ldur x7)
    asr     x4, x3, #63         // sign mask i   (x3 ready: 2 cycle gap ✓)
    asr     x8, x7, #63         // sign mask i+1 (x7 ready: 2 cycle gap ✓)
    eor     x3, x3, x4
    eor     x7, x7, x8
    subs    x3, x3, x4          // abs(elem i)
    subs    x7, x7, x8          // abs(elem i+1)
    adds    x2, x2, x3          // accumulate
    adds    x2, x2, x7          // accumulate
    subs    x5, x5, x9          // decrement pair counter
    b.ne    .loop2              // 1 misprediction at exit
.handle_odd:
    adds    xzr, xzr, x6        // set Z if N was even (x6==0)
    b.eq    .done
    ldur    x3, [x0, #0]        // process remaining odd element
    add     x0, x0, #8
    asr     x4, x3, #63
    eor     x3, x3, x4
    subs    x3, x3, x4
    adds    x2, x2, x3
.done:
    adds    x0, xzr, x2
    ret
.size   abs_sum, .-abs_sum
