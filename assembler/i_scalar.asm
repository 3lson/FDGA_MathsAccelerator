s.li sp, 42 # mem_addr = 42
s.li s1, 10 # s1(x6) = 10
s.sw s1, 0(sp) # storing 10 into mem[42]
s.lw s2, 0(sp) # loading s2(x7) = 10 (mem[42])
s.sw s3, 1(sp) # storing 10 into mem[43]
s.addi s3, s2, 10 # Expected s3(x8) = 20
s.sw s3, 2(sp) # storing 10 into mem[44]
s.muli s3, s2, 10 # Expected s3(x8) = 100
s.sw s3, 3(sp) # storing 10 into mem[45]
s.divi s3, s2, 10 # Expected s3(x8) = 1
s.sw s3, 4(sp) # storing 10 into mem[46]
s.slli s3, s2, 2 # Expected s3(x8) = 40
s.sw s3, 5(sp) # storing 10 into mem[46]
exit