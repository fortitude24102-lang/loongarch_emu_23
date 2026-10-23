; Sum 1..10. Expected: $r1 = 11, $r2 = 10, $r3 = 55.
; The simulator has no HALT instruction; it stops at the 200-cycle limit.
    ADDI.W $r1, $r0, 1
    ADDI.W $r2, $r0, 10
    ADDI.W $r3, $r0, 0

loop:
    ADD.W  $r3, $r3, $r1
    ADDI.W $r1, $r1, 1
    BGE    $r2, $r1, loop

; Preserve the result while waiting for the simulator's cycle limit.
done:
    B done
