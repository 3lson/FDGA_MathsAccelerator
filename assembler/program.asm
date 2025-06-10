s.li sp, 42
s.li s1, 10
s.sw s1, 0(sp)
s.lw s2, 0(sp)
s.add s3, s1, s2
s.sub s3, s1, s2
s.mul s3, s1, s2
s.div s3, s1, s2
s.slt s3, s1, s2
s.seq s3, s1, s2
s.snez s3, s1
s.li s4, -5
s.abs s4, s4
exit