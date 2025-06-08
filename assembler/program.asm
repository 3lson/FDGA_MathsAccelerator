li s0, 200
lui t0, %hi(.LC0)
flw fa2, %lo(.LC0)(t0)
lui t0, %hi(.LC1)
flw ft3, %lo(.LC1)(t0)
fadd.s ft1, fa2, ft3
fneg.s ft2, ft1
fsw ft1, 0(s0)
fsw ft1, -116(s0)
flw ft2, -116(s0)
li t0, 5
fcvt.s.w fs11, 
li t2, 40 # 0x28
fadd.s ft7, fa2, ft3

.LC0:
    .word 1065353216
    .align 2
.LC1:
    .word 1073741824
    .align 2 