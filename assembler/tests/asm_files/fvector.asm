v.li v2, 20
lui v1, %hi(.LC0)
v.flw fv1, %lo(.LC0)(v1)
v.fsw fv1, 4(v2)
lui v1, %hi(.LC1)
v.flw fv1, %lo(.LC1)(v1)
v.fsw fv1, 8(v2)
v.li v5, 44
v.flw fv1, 4(v2)
v.flw fv2, 8(v2)
v.fadd.s fv3, fv1, fv2 
v.fsw fv3, 1(v5) 
v.fsub.s fv3, fv2, fv1 
v.fsw fv3, 2(v5)
v.fmul.s fv3, fv1, fv2
v.fsw fv3, 3(v5)
v.flt.s v3, fv1, fv2
v.sw v3, 5(v5)
v.fneg.s fv4, fv1
v.fsw fv4, 6(v5)
v.feq.s v3, fv1, fv2
v.sw v3, 7(v5)
v.fabs.s fv5, fv4
v.fsw fv5, 8(v5)
v.fcvt.w.s v1, fv1
v.sw v1, 9(v5)
exit

.section .rodata 
.align 2
.LC0:
	.word 1065353216
	.align 2
.LC1:
	.word 1073741824
	.align 2