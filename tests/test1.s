.global _start

_start:
    li a5, 0
    li a4, 20
.l1:
    addi a5, a5, 1
    addi a5, a5, 1
    bne a5, a4 , .l1
