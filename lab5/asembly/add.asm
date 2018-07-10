.ORIG X3000
;adding locations xc000 to xc013
LEA R0, START
LDW R0, R0, #0
AND R2, R2, #0   ;COUNTER
AND R3, R3, #0   ;SUM
ADD R2, R2, #10
ADD R2, R2, #10

DO LDB R1, R0, #0
ADD R0, R0 #1
ADD R3, R3, R1
ADD R2, R2, #-1
BRP DO
STW R3, R0, #0

;testing for protection exception
JMP R3

HALT

START .FILL XC000
.END
