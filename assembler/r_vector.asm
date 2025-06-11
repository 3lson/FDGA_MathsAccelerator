v.li v5, 42 # mem_addr = 42
v.li s1, 10 # s1(x6) = 10
v.sw s1, 0(v5) # storing 10 into mem[42]
v.lw s2, 0(v5) # loading s2(x7) = 10 (mem[42])
v.add s3, s1, s2 # Expect: s3(x8) = 20
v.sw s3, 1(v5) # storing 10 into mem[43]
v.sub s3, s1, s2 # Expect s3(x8) = 0
v.sw s3, 2(v5) # storing 10 into mem[44]
v.mul s3, s1, s2 # Expect s3(x8) = 100
v.sw s3, 3(v5) # storing 10 into mem[45]
v.div s3, s1, s2 # Expect s3(x8) = 1
v.sw s3, 4(v5) # storing 10 into mem[46]
v.slt s3, s1, s2 # Expect s3(x8) = 0
v.sw s3, 5(v5) # storing 10 into mem[47]
v.seq s3, s1, s2 # Expect s3(x8) = 1
v.sw s3, 6(v5) # storing 10 into mem[48]
v.snez s3, s1 # Expect s3(x8) = 0
v.sw s3, 7(v5) # storing 10 into mem[49]
v.li s4, -5
v.abs s4, s4 # Expect s4(x9) = 5
v.sw s4, 8(v5) # storing 10 into mem[50]
exit