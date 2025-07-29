s.li sp, 42
s.li s1, 4
s.sw s1, 0(sp)
for_start0:
s.lw s2, 0(sp)
s.li s3, 3
s.slt s4, s2, s3
s.beqz s4, for_end1
s.li s5, 6
s.sw s5, 1(sp)
exit
for_end1:
s.li s5, 5
s.sw s5, 2(sp)
exit