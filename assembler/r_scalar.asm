s.li sp, 42 # mem_addr = 42
s.li s1, 10 # s1(x6) = 10
s.sw s1, 0(sp) # storing 10 into mem[42]
s.lw s2, 0(sp) # loading s2(x7) = 10 (mem[42])
s.add s3, s1, s2 # Expect: s3(x8) = 20
s.sw s3, 1(sp) # storing 20 into mem[43]
s.sub s3, s1, s2 # Expect s3(x8) = 0
s.sw s3, 2(sp) # storing 0 into mem[44]
s.mul s3, s1, s2 # Expect s3(x8) = 100
s.sw s3, 3(sp) # storing 100 into mem[45]
s.div s3, s1, s2 # Expect s3(x8) = 1
s.sw s3, 4(sp) # storing 1 into mem[46]
s.slt s3, s1, s2 # Expect s3(x8) = 0
s.sw s3, 5(sp) # storing 0 into mem[47]
s.seq s3, s1, s2 # Expect s3(x8) = 1
s.sw s3, 6(sp) # storing 1 into mem[48]
s.snez s3, s1 # Expect s3(x8) = 1
s.sw s3, 7(sp) # storing 1 into mem[49]
s.li s4, -5
s.abs s4, s4 # Expect s4(x9) = 5
s.sw s4, 8(sp) # storing 5 into mem[50]
exit