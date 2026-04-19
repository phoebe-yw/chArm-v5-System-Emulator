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
.loop:
    subs    x1, x1, xzr         // Check if there are elements left in array to sum.
    b.eq    .done               // If not, we are done.

    ldur    x3, [x0, #0]        // Load element from memory.

    cmp     x3, xzr             // Check if element is positive
    b.ge    .positive

.negative:
    subs    x3, xzr, x3         // Element is negative, negate it.
                                // Fall through to positive case.

.positive:
    adds    x2, x2, x3          // Element is positive, add it to sum.

.end:
    add     x0, x0, #8          // Increment pointer.
    sub     x1, x1, #1          // Decrement remaining size.
    b       .loop               // Continue loop.

.done:
    adds    x0, xzr, x2         // Move sum into x0.
    ret
.size   abs_sum, .-abs_sum
