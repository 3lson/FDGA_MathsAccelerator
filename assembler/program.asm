s.li s0, 20
lui s1, %hi(.LC0)
s.flw fs1, %lo(.LC0)(s1)
s.fsw fs1, 4(s0)
lui s1, %hi(.LC1)
s.flw fs1, %lo(.LC1)(s1)
s.fsw fs1, 8(s0)
s.li sp, 44
s.flw fs1, 4(s0)
s.flw fs2, 8(s0)
s.fadd.s fs3, fs1, fs2
s.fsw fs3, 0(sp) 
exit

.section .rodata 
.align 2
.LC0:
	.word 1065353216
	.align 2
.LC1:
	.word 1073741824
	.align 2
