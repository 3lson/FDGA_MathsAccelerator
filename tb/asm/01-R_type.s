main:
    # Test 1: Add positive numbers (Expected Output: 246)
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
    # Test 2: Add negative numbers (Expected Output: -246)
    addi a0, zero, -32        # a0 = -32
    addi a0, a0, -101         # a0 = -32 - 101 = -133
    addi a1, zero, -52        # a1 = -52
    addi a1, a1, -61          # a1 = -52 - 61 = -113
    add a0, a0, a1            # a0 = -133 - 113 = -246
    nop
    # Check Test 2 result
    li t0, -246               # Load expected value (-246)
    bne a0, t0, fail_test     # If a0 != -246, jump to fail_test
    j next_test_3             # Jump to next test

next_test_3:
    # Test 3: XOR operation (Expected Output: 20)
    addi a0, zero, 32         # a0 = 32
    addi a1, zero, 52         # a1 = 52
    xor a0, a0, a1            # a0 = 32 ^ 52 = 20
    nop
    # Check Test 3 result
    li t0, 20                 # Load expected value (20)
    bne a0, t0, fail_test     # If a0 != 20, jump to fail_test
    j next_test_4             # Jump to next test

next_test_4:
    # Test 4: Subtraction (Expected Output: 10)
    addi a0, zero, 50         # a0 = 50
    addi a0, a0, -25          # a0 = 50 - 25 = 25
    addi a1, zero, 15         # a1 = 15
    sub a0, a0, a1            # a0 = 25 - 15 = 10
    nop
    # Check Test 4 result
    li t0, 10                 # Load expected value (10)
    bne a0, t0, fail_test     # If a0 != 10, jump to fail_test
    j next_test_5             # Jump to next test

next_test_5:
    # Test 5: OR operation (Expected Output: 14)
    addi a0, zero, 0b1010     # a0 = 10 (binary 1010)
    addi a1, zero, 0b1100     # a1 = 12 (binary 1100)
    or a0, a0, a1             # a0 = 10 | 12 = 14 (binary 1110)
    nop
    # Check Test 5 result
    li t0, 14                 # Load expected value (14)
    bne a0, t0, fail_test     # If a0 != 14, jump to fail_test
    j next_test_6             # Jump to next test

next_test_6:
    # Test 6: LSL (Shift Left Logical) (Expected Output: a0 = 8)
    addi a0, zero, 4         # a0 = 4 (0000000000000100)
    addi a1, zero, 1         # a1 = 1 (shift by 1)
    sll a0, a0, a1           # a0 = 4 << 1 = 8
    nop
    # Check Test 6 result
    li t0, 8                  # Load expected value (8)
    bne a0, t0, fail_test     # If a0 != 8, jump to fail_test
    j next_test_7             # Jump to next test

next_test_7:
    # Test 7: LSR (Shift Right Logical) (Expected Output: a0 = 4)
    addi a0, zero, 16        # a0 = 16 (0000000000010000)
    addi a1, zero, 2         # a1 = 2 (shift by 2)
    srl a0, a0, a1           # a0 = 16 >> 2 = 4
    nop
    # Check Test 7 result
    li t0, 4                  # Load expected value (4)
    bne a0, t0, fail_test     # If a0 != 4, jump to fail_test
    j next_test_8             # Jump to next test

next_test_8:
    # Test 8: ASR (Arithmetic Shift Right) (Expected Output: a0 = -4)
    addi a0, zero, -16       # a0 = -16 (1111111111110000)
    addi a1, zero, 2         # a1 = 2 (shift by 2)
    sra a0, a0, a1           # a0 = -16 >>> 2 = -4
    nop
    # Check Test 8 result
    li t0, -4                 # Load expected value (-4)
    bne a0, t0, fail_test     # If a0 != -4, jump to fail_test
    j next_test_9             # Jump to next test

next_test_9:
    # Test 9: SLT (Set Less Than) (Expected Output: a0 = 1)
    addi a0, zero, -10       # a0 = -10
    addi a1, zero, 5         # a1 = 5
    slt a0, a0, a1           # a0 = (-10 < 5) ? 1 : 0
    nop
    # Check Test 9 result
    li t0, 1                  # Load expected value (1)
    bne a0, t0, fail_test     # If a0 != 1, jump to fail_test
    j next_test_10            # Jump to next test

next_test_10:
    # Test 10: SLTU (Set Less Than Unsigned) (Expected Output: a0 = 0)
    addi a0, zero, 10        # a0 = 10
    addi a1, zero, 5         # a1 = 5
    sltu a0, a0, a1          # a0 = (10 < 5) ? 1 : 0
    nop
    # Check Test 10 result
    li t0, 0                  # Load expected value (0)
    bne a0, t0, fail_test     # If a0 != 0, jump to fail_test
    j success_test            # Jump to success_test

fail_test:
    # If any test fails, set a0 to -1 to indicate failure
    li a0, -1                 # Set a0 to -1 to indicate failure
    j end_tests               # Jump to end_tests

success_test:
    # All tests passed, set a0 to 1 to indicate success
    li a0, 150                  # Set a0 to 150 to indicate success
    j end_tests               # Jump to end_tests

end_tests:
    # All tests completed, result in a0
    nop
