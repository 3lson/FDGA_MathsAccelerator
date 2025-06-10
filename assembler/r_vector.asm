v.li sp, 42 # mem_addr = 42
v.li s1, 10 # s1(x6) = 10
v.sw s1, 0(sp) # storing 10 into mem[42]
v.lw s2, 0(sp) # loading s2(x7) = 10 (mem[42])
v.add s3, s1, s2 # Expect: s3(x8) = 20
v.sub s3, s1, s2 # Expect s3(x8) = 0
v.mul s3, s1, s2 # Expect s3(x8) = 100
v.div s3, s1, s2 # Expect s3(x8) = 1
v.slt s3, s1, s2 # Expect s3(x8) = 0
v.seq s3, s1, s2 # Expect s3(x8) = 1
v.snez s3, s1 # Expect s3(x8) = 0
v.li s4, -5
v.abs s4, s4 # Expect s4(x9) = 5
exit