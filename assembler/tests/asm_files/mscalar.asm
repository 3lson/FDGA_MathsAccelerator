s.li sp, 42
lui s1, %hi(.LC0)
s.flw fs1, %lo(.LC0)(s1)
s.fsw fs1, 0(sp)
s.li s2, 32
s.sw s2, 1(sp)
s.lw s3, 1(sp)
s.sw s3, 2(sp)
exit

.section .rodata 
.align 2
.LC0:
	.word 1065353216
	.align 2