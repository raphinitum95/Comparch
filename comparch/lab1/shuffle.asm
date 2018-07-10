.ORIG X3000
    AND R6, R6, #0

LOOP LEA R0, SHUFFLE ; load pointer to x4004 into R0
    LDW R0, R0, #0  ; load x4004 into R0
    LDB R0, R0, #0
    LEA R1, GETSTART ; ^^
    LDW R1, R1, #0 ; load x4000 into R1
    LEA R2, PUTSTART
    LDW R2, R2, #0 ; load x4005 into R2 
    ADD R2, R2, R6 

    ADD R5, R6, #0
    BRZ NEXT

    ADD R5, R6, #-1
    BRZ ONE

    ADD R5, R6, #-2
    BRZ TWO

    ADD R5, R6, #-3
    BRZ THREE

ONE RSHFL R0, R0, #2
    BR NEXT

TWO RSHFL R0, R0, #4
    BR NEXT

THREE RSHFL R0, R0, #6

NEXT AND R0, R0, #3
    ADD R1, R1, R0
    LDB R3, R1, #0
    STB R3, R2, #0
    
    ADD R5, R6, #-3
    BRZ DONE ;0404

    ADD R6, R6, #1 ;0x1DA1
    BR LOOP ;0E18

DONE HALT ; 0xF025

SHUFFLE .FILL X4004

GETSTART .FILL X4000

PUTSTART .FILL X4005

.END

