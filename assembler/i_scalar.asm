s.li sp, 42 # mem_addr = 42
s.li s1, 10 # s1(x6) = 10
s.sw s1, 0(sp) # storing 10 into mem[42]
s.lw s2, 0(sp) # loading s2(x7) = 10 (mem[42])
s.addi s3, s2, 10 # Expected s3(x8) = 20
s.muli s3, s2, 10 # Expected s3(x8) = 100
s.divi s3, s2, 10 # Expected s3(x8) = 1
s.slli s3, s2, 2 # Expected s3(x8) = 40
exit