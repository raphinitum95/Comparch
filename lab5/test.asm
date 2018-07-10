.orig x3000
.fill xa000
lea r0, unaligned
ldw r0, r0, #0
ldw r0, r0, #0
trap x25
unaligned .fill x0001
.end
