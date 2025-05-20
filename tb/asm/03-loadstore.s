    .globl main
main:
    # Test 1: Store and Load Word (Expected Output: 0x12345678)
    li x10, 0x12345678      # Pseudo-instruction: regfile[x10] = 0x12345678
    li x11, 0xFF886543      # Pseudo-instruction: regfile[x11] = 0xFF886543

    # Store the word from regfile[x10] into data location indexed 0
    sw x10, 0(x19)          
    # Store the word from regfile[x11] into data location indexed 4
    sw x11, 4(x19)          

    # Load the word back from memory location indexed 0 into regfile[x12]
    lw x12, 0(x19)         
    nop
    # Check Test 1 result
    li t0, 0x12345678       # Load expected value (0x12345678)
    bne x12, t0, fail_test  # If x12 != 0x12345678, jump to fail_test
    j next_test_2           # Jump to next test

next_test_2:
    # Test 2: Store and Load Byte (Expected Output: 0x78)
    sb x10, 8(x19)          # Store byte (lowest byte of x10)
    
    # Load the byte back from memory location indexed 8 into regfile[x17]
    lw x17, 8(x19)         
    nop
    # Check Test 2 result
    li t0, 0x00000078       # Load expected value (0x78, byte value)
    bne x17, t0, fail_test  # If x17 != 0x00000078, jump to fail_test
    j next_test_3           # Jump to next test

next_test_3:
    # Test 3: Store and Load Half-Word (Expected Output: 0x5678)
    sh x10, 12(x19)         # Store halfword (lower 2 bytes of x10)
    
    # Load the word back from memory location indexed 12 into regfile[x18]
    lw x18, 12(x19)         
    nop
    # Check Test 3 result
    li t0, 0x00005678       # Load expected value (0x5678, half-word value)
    bne x18, t0, fail_test  # If x18 != 0x00005678, jump to fail_test
    j next_test_4           # Jump to next test

next_test_4:
    # Test 4: Load Byte (Expected Output: 0x78)
    lb x12, 0(x19)          # Load first byte from memory location indexed 0
    nop
    # Check Test 4 result
    li t0, 0x00000078       # Load expected value (0x78, byte value)
    bne x12, t0, fail_test  # If x12 != 0x00000078, jump to fail_test
    j next_test_5           # Jump to next test

next_test_5:
    # Test 5: Load Half-Word (Expected Output: 0xFF88)
    lh x13, 6(x19)          # Load half-word from memory location indexed 6
    nop
    # Check Test 5 result
    li t0, 0xFFFFFF88       # Load expected value (0xFF88, with sign extension)
    bne x13, t0, fail_test  # If x13 != 0xFFFFFF88, jump to fail_test
    j next_test_6           # Jump to next test

next_test_6:
    # Test 6: Load Word (Expected Output: 0x12345678)
    lw x14, 0(x19)          # Load word from memory location indexed 0
    nop
    # Check Test 6 result
    li t0, 0x12345678       # Load expected value (0x12345678)
    bne x14, t0, fail_test  # If x14 != 0x12345678, jump to fail_test
    j next_test_7           # Jump to next test

next_test_7:
    # Test 7: Load Unsigned Byte (Expected Output: 0x88)
    lbu x15, 6(x19)         # Load unsigned byte from memory location indexed 6
    nop
    # Check Test 7 result
    li t0, 0x00000088       # Load expected value (0x88, unsigned byte value)
    bne x15, t0, fail_test  # If x15 != 0x00000088, jump to fail_test
    j next_test_8           # Jump to next test

next_test_8:
    # Test 8: Load Unsigned Half-Word (Expected Output: 0xFF88)
    lhu x16, 6(x19)         
    nop
    # Check Test 8 result
    li t0, 0x0000FF88       
    bne x16, t0, fail_test  # If x16 != 0x0000FF88, jump to fail_test
    j success_test          # Jump to success_test

fail_test:
    # If any test fails, set x10 to -1 to indicate failure
    li x10, -1              # Set x10 to -1 to indicate failure
    j end_tests             # Jump to end_tests

success_test:
    # All tests passed, set x10 to 150 to indicate success
    li x10, 150             # Set x10 to 150 to indicate success
    j end_tests             # Jump to end_tests

end_tests:
    # All tests completed, result in x10
    nop
