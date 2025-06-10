lui s1, %hi(.LC0)  # Load address of float 1.0
s.flw fs1, %lo(.LC0)(s1)
s.fsw fs1, -108(s0) # points_x[0] = 1.0


.section .rodata 
.align 2
.LC0:
	.word 1065353216
	.align 2