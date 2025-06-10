s.li s0, 42
lui s1, %hi(.LC0)  # Load address of float 1.0
s.flw fs1, %lo(.LC0)(s1)
s.fsw fs1, -108(s0) # points_x[0] = 1.0
lui s1, %hi(.LC1)  # Load address of float 1.0
s.flw fs1, %lo(.LC1)(s1)
s.fsw fs1, -104(s0) # points_x[0] = 1.0

.section .rodata 
.align 2
.LC0:
	.word 1065353216
	.align 2
.LC1:
	.word 1065353216
	.align 2
exit


s.flw fs1, -108(s0) # 1.0
s.flw fs2, -104(s0) # 1.0
s.fadd.s fs3, fs1, fs2 # Expect: fs3(x3)  = 2.0
s.fsub.s fs3, fs1, fs2 # Expect s3(x8) = 0.0
s.fmul.s fs3, fs1, fs2 # Expect s3(x8) = 1.0
s.fdiv.s fs3, fs1, fs2 # Expect s3(x8) = 1.0
s.flt.s s3, fs1, fs2 # Expect s3(x8) = 0
s.fneg.s fs4, fs1 # Expect fs4(x4) = -1.0
s.feq.s s3, fs1, fs2 # Expect s3(x8) = 1
s.fabs.s fs5, fs4 # Expected fs5(x5) = 1.0
s.fcvt.w.s s1, fs1 # Expected s1(x6) = 1
exit