s.li sp, 42
s.li s1, 10
s.sw s1, 0(sp)
s.lw s2, 0(sp)
s.add s3, s1, s2
s.sw s3, 1(sp)
s.sub s3, s1, s2
s.sw s3, 2(sp)
s.mul s3, s1, s2
s.sw s3, 3(sp)
s.div s3, s1, s2
s.sw s3, 4(sp)
s.slt s3, s1, s2
s.sw s3, 5(sp)
s.seq s3, s1, s2
s.sw s3, 6(sp)
s.snez s3, s1
s.sw s3, 7(sp)
s.li s4, -5
s.abs s4, s4
s.sw s4, 8(sp)
exit