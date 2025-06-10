lui s1, %hi(.LC0)  # Load address of float 1.0
s.li sp, 42 # mem_addr = 42
s.flw fs1, %lo(.LC0)(s1)
s.sw fs1, 0(sp) # storing 10 into mem[43]
s.fsw fs1, -108(s0) # points_x[0] = 1.0
s.sw fs1, 1(sp) # storing 10 into mem[43]
exit

.section .rodata 
.align 2
.LC0:
	.word 1065353216
	.align 2