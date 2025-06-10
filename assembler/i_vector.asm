v.li sp, 42 # mem_addr = 42
v.li s1, 10 # s1(x6) = 10
v.sw s1, 0(sp) # storing 10 into mem[42]
v.lw s2, 0(sp) # loading s2(x7) = 10 (mem[42])
v.addi s3, s2, 10 # Expected s3(x8) = 20
v.muli s3, s2, 10 # Expected s3(x8) = 100
v.divi s3, s2, 10 # Expected s3(x8) = 1
v.slli s3, s2, 2 # Expected s3(x8) = 40
exit