    .globl main
main:
    # Test 1: ADDI (Add Immediate) (Expected Output: 246)
    addi a0, zero, 32         # a0 = 32
    addi a0, a0, 101          # a0 = 32 + 101 = 133
    addi a1, zero, 52         # a1 = 52
    addi a1, a1, 61           # a1 = 52 + 61 = 113
    add a0, a0, a1            # a0 = 133 + 113 = 246
    nop
    # Check Test 1 result
    li t0, 246                # Load expected value (246)
    bne a0, t0, fail_test     # If a0 != 246, jump to fail_test
    j next_test_2             # Jump to next test

next_test_2:
    # Test 2: ORI (OR Immediate) (Expected Output: 14)
    addi a0, zero, 10         # a0 = 10 (decimal, binary 1010)
    ori a0, a0, 12            # a0 = a0 | 12 = 10 | 12 = 14 (binary 1110)
    nop
    # Check Test 2 result
    li t0, 14                 # Load expected value (14)
    bne a0, t0, fail_test     # If a0 != 14, jump to fail_test
    j next_test_3             # Jump to next test

next_test_3:
    # Test 3: XORI (XOR Immediate) (Expected Output: 20)
    addi a0, zero, 32         # a0 = 32
    xori a0, a0, 52           # a0 = 32 ^ 52 = 20 (XOR with immediate)
    nop
    # Check Test 3 result
    li t0, 20                 # Load expected value (20)
    bne a0, t0, fail_test     # If a0 != 20, jump to fail_test
    j next_test_4             # Jump to next test

next_test_4:
    # Test 4: SLLI (Shift Left Logical Immediate) (Expected Output: 8)
    addi a0, zero, 4          # a0 = 4 (binary 0000000000000100)
    slli a0, a0, 1            # a0 = 4 << 1 = 8 (Shift left by 1 immediate)
    nop
    # Check Test 4 result
    li t0, 8                  # Load expected value (8)
    bne a0, t0, fail_test     # If a0 != 8, jump to fail_test
    j next_test_5             # Jump to next test

next_test_5:
    # Test 5: SRLI (Shift Right Logical Immediate) (Expected Output: 4)
    addi a0, zero, 16         # a0 = 16 (binary 0000000000010000)
    srli a0, a0, 2            # a0 = 16 >> 2 = 4 (Shift right by 2 immediate)
    nop
    # Check Test 5 result
    li t0, 4                  # Load expected value (4)
    bne a0, t0, fail_test     # If a0 != 4, jump to fail_test
    j next_test_6             # Jump to next test

next_test_6:
    # Test 6: SRAI (Arithmetic Shift Right Immediate) (Expected Output: -1)
    addi a0, zero, -4          # a0 = 8 (binary 0000000000001000)
    srai a0, a0, 2           
    nop
    # Check Test 6 result
    li t0, -1                 # Load expected value (-4)
    bne a0, t0, fail_test     # If a0 != -4, jump to fail_test
    j next_test_7             # Jump to next test

next_test_7:
    # Test 7: SLTI (Set Less Than Immediate) (Expected Output: 1)
    addi a0, zero, -10        # a0 = -10
    slti a0, a0, 5            
    nop
    # Check Test 7 result
    li t0, 1                  # Load expected value (1)
    bne a0, t0, fail_test     # If a0 != 1, jump to fail_test
    j next_test_8             # Jump to next test

next_test_8:
    # Test 8: SLTIU (Set Less Than Immediate Unsigned) (Expected Output: 0)
    # Test 8: SLTIU (Set Less Than Immediate Unsigned) (Expected Output: 0)
    addi a0, zero, 10         # a0 = 10
    sltiu a0, a0, 5          
    nop
    # Check Test 8 result
    li t0, 0                  # Load expected value (0)
    bne a0, t0, fail_test     # If a0 != 0, jump to fail_test
    j success_test            # Jump to success_test

fail_test:
    # If any test fails, set a0 to -1 to indicate failure
    li a0, -1                 # Set a0 to -1 to indicate failure
    j end_tests               # Jump to end_tests

success_test:
    # All tests passed, set a0 to 150 to indicate success
    li a0, 150                # Set a0 to 150 to indicate success
    j end_tests               # Jump to end_tests

end_tests:
    # All tests completed, result in a0
    nop
