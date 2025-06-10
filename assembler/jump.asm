s.li sp, 42
s.li s1, 4
sw s1, -116(sp) # i = 4
for_start0:
lw s2, -116(sp)
li s3, 3 
slt s4, s2, s3
j for_end1
for_end1:
s.li s5, 5
exit