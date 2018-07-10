.orig x1200
add r6, r6, #-2
stw r0, r6, #0
add r6, r6, #-2
stw r1, r6, #0
add r6, r6, #-2
stw r2, r6, #0
add r6, r6, #-2
stw r3, r6, #0

lea r0, pagetable
ldw r0, r0, #0
lea r1, size
ldw r1, r1, #0
lea r3, clear
ldw r3, r3, #0

loop ldw r2, r0, #0
and r2, r2, r3
stw r2, r0 #0
add r0, r0, #2
add r1, r1, #-1
brp loop

ldw r3, r6, #0
add r6, r6, #2
ldw r2, r6, #0
add r6, r6, #2
ldw r1, r6, #0
add r6, r6, #2
ldw r0, r6, #0
add r6, r6, #2

rti

pagetable .fill x1000
size .fill x0080
clear .fill xfffe
.end
