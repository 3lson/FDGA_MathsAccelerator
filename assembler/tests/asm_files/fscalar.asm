s.li s0, 20
lui s1, %hi(.LC0)
s.flw fs1, %lo(.LC0)(s1)
s.fsw fs1, 4(s0)
lui s1, %hi(.LC1)
s.flw fs1, %lo(.LC1)(s1)
s.fsw fs1, 8(s0)
s.li sp, 44
s.fsw fs3, 0(sp) 
s.flw fs1, 4(s0)
s.flw fs2, 8(s0)
s.fadd.s fs3, fs1, fs2 
s.fsw fs3, 1(sp) 
s.fsub.s fs3, fs2, fs1 
s.fsw fs3, 2(sp)
s.fmul.s fs3, fs1, fs2
s.fsw fs3, 3(sp)
s.flt.s s3, fs1, fs2
s.sw s3, 5(sp)
s.fneg.s fs4, fs1
s.fsw fs4, 6(sp)
s.feq.s s3, fs1, fs2
s.sw s3, 7(sp)
s.fabs.s fs5, fs4
s.fsw fs5, 8(sp)
s.fcvt.w.s s1, fs1
s.sw s1, 9(sp)
exit

.section .rodata 
.align 2
.LC0:
	.word 1065353216
	.align 2
.LC1:
	.word 1073741824
	.align 2