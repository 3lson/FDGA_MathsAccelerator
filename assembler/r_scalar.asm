s.li sp, 42 # mem_addr = 42
s.li s1, 10 # s1(x6) = 10
s.sw s1, 0(sp) # storing 10 into mem[42]
s.lw s2, 0(sp) # loading s2(x7) = 10 (mem[42])
s.add s3, s1, s2 # Expect: s3(x8) = 20
s.sub s3, s1, s2 # Expect s3(x8) = 0
s.mul s3, s1, s2 # Expect s3(x8) = 100
s.div s3, s1, s2 # Expect s3(x8) = 1
s.slt s3, s1, s2 # Expect s3(x8) = 0
s.seq s3, s1, s2 # Expect s3(x8) = 1
s.snez s3, s1 # Expect s3(x8) = 0
s.li s4, -5
s.abs s4, s4 # Expect s4(x9) = 5
exit