.global _start

_start:
        addi    sp,sp,-16
        li      a5,4
        sw      a5,4(sp)
        li      a5,2
        sw      a5,8(sp)
        li      a5,1
        sw      a5,12(sp)
        lw      a5,4(sp)
        ble     a5,zero,.L2
        li      a4,0
.L3:
        lw      a3,8(sp)
        lw      a5,12(sp)
        addi    a4,a4,1
        mul     a5,a5,a3
        sw      a5,12(sp)
        lw      a5,4(sp)
        bgt     a5,a4,.L3
.L2:
        lw      a0,12(sp)
        addi    sp,sp,16
