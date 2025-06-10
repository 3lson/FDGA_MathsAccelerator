s.li sp, 42
s.li s1, 4
s.sw s1, -116(sp) # i = 4
for_start0:
s.lw s2, -116(sp)
s.li s3, 3 
s.slt s4, s2, s3 
s.beqz s4, for_end1 # if (i>=3) break
s.li sp, 42 # mem_addr = 42
s.li s5, 6
s.sw s5, 1(sp) # storing 10 into mem[43]
exit
for_end1:
s.li sp, 42 # mem_addr = 42
s.li s5, 5
s.sw s5, 0(sp) # storing 10 into mem[43]
exit