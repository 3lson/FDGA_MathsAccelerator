v.li v5, 42
lui v1, %hi(.LC0)
v.flw fv1, %lo(.LC0)(v1)
v.fsw fv1, 0(v5)
v.li v2, 32
v.sw v2, 1(v5)
v.lw v3, 1(v5)
v.sw v3, 2(v5)
exit

.section .rodata 
.align 2
.LC0:
	.word 1065353216
	.align 2