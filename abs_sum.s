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
    movz    x10, #2
    tst     x1, x9              // check odd or even
    b.ne    .handle_odd         // do odd case first
    add     x1, x1, #2          // because jumped to middle, +2 buffer
    b       .loop_check 

.loop2:
    ldur    x3, [x0, #0]        // load arr[i]
    ldur    x7, [x0, #8]        // load arr[i+1]
    add     x0, x0, #16         // increment i += 2
    asr     x4, x3, #63         // compute additive inverse if negative and keep is positive
    asr     x8, x7, #63         
    eor     x3, x3, x4
    eor     x7, x7, x8
    subs    x3, x3, x4          
    subs    x7, x7, x8          
    adds    x2, x2, x3          // add the 2 longs
    adds    x2, x2, x7          

.loop_check:
    subs    x1, x1, x10         // decrement x by 2
    b.ne    .loop2              // 1 misprediction at exit
    b       .done

.handle_odd:                    // process odd case
    ldur    x3, [x0, #0]        
    add     x0, x0, #8          // i += 1
    asr     x4, x3, #63         
    eor     x3, x3, x4          // compute additive inverse if negative
    subs    x3, x3, x4
    adds    x2, x2, x3          // add long
    subs    x1, x1, x9          // decrement x by 1    
    add     x1, x1, #2          // +2 buffer
    b.ne    .loop_check

.done:                          // LOOP IS DONE
    adds    x0, xzr, x2
    ret
.size   abs_sum, .-abs_sum
