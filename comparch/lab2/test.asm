.ORIG X3000
    LEA R0, CODE 				;0x3000
    LDB R0, R0, #0				;0x3002

    LEA R1, A 					;0x3004
    LDB R1, R1, #0 				;0x3006

    LEA R2, B 					;0x3008
    LDB R2, R2, #0 				;0x300A
    
    LEA R3, C 					;0x300C
    LDB R3, R3, #0 				;0x300E

    LEA R4, D 					;0x3010
    LDB R4, R4, #0 				;0x3012

    LEA R5, GETSTART 				;0x3014
    LDW R5, R5 #0 				;0x3016

    STB R1, R5, #0 				;0x3018
    STB R2, R5, #1 				;0x301A
    STB R3, R5, #2 				;0x301C
    STB R4, R5, #3 				;0x301E
    STB R0, R5, #4 				;0x3020




    AND R6, R6, #0 				;0x3022

LOOP LEA R0, SHUFFLE  				;0x3024
    LDW R0, R0, #0  ; load x4004 into R0 	;0x3026
    LDB R0, R0, #0 				;0x3028
    LEA R1, GETSTART ; ^^ 			;0x302A
    LDW R1, R1, #0 ; load x4000 into R1 	;0x302C
    LEA R2, PUTSTART 				;0x302E
    LDW R2, R2, #0 ; load x4005 into R2  	;0x3030
    ADD R2, R2, R6  				;0x3032

    ADD R5, R6, #0 				;0x3034
    BRZ NEXT 					;0x3036

    ADD R5, R6, #-1 				;0x3038
    BRZ ONE 					;0x303A

    ADD R5, R6, #-2 				;0x303C
    BRZ TWO 					;0x303E

    ADD R5, R6, #-3 				;0x3040
    BRZ THREE 					;0x3042

ONE RSHFL R0, R0, #2 				;0x3044
    BR NEXT 					;0x3046

TWO RSHFL R0, R0, #4 				;0x3048
    BR NEXT 					;0x304A

THREE RSHFL R0, R0, #6 				;0x304C

NEXT AND R0, R0, #3 				;0x304E
    ADD R1, R1, R0 				;0x3050
    LDB R3, R1, #0 				;0x3052
    STB R3, R2, #0 				;0x3054
    
    ADD R5, R6, #-3 				;0x3056
    BRZ DONE ;0404 				;0x3058

    ADD R6, R6, #1 ;0x1DA1 			;0x305A
    BR LOOP ;0E18 				;0x305C

DONE HALT ; 0xF025 				;0x305E

SHUFFLE .FILL X4004 				;0x3060

GETSTART .FILL X4000 				;0x3062

PUTSTART .FILL X4005 				;0x3064

CODE .FILL X7800 				;0x3066

A .FILL XAA00 					;0x3068

B .FILL XBB00 					;0x306A
 
C .FILL XCC00 					;0x306C

D .FILL XDD00 					;0x306C

.END

