    .globl main
main:
    # Test 1: BEQ (Branch if Equal)
    addi a0, zero, 5          # a0 = 5
    addi a1, zero, 5          # a1 = 5
    beq a0, a1, branch_equal  # Branch to branch_equal if a0 == a1
    nop                      
    
    # Check Test 1 result (Expected: branch is taken, so a0 = 5)
    li t0, 5                  # Load expected value (5)
    bne a0, t0, fail_test     # If a0 != 5, jump to fail_test
    j next_test_2             # Jump to next test

branch_equal:
    # This is where the BEQ instruction jumps
    nop                      
next_test_2:
    # Test 2: BNE (Branch if Not Equal)
    addi a0, zero, 5          # a0 = 5
    addi a1, zero, 10         # a1 = 10
    bne a0, a1, branch_unequal # Branch to branch_unequal if a0 != a1
    nop                      
    
    # Check Test 2 result (Expected: branch is taken, so a0 = 5)
    li t0, 5                  # Load expected value (5)
    bne a0, t0, fail_test     # If a0 != 5, jump to fail_test
    j next_test_3             # Jump to next test

branch_unequal:
    # This is where the BNE instruction jumps
    nop                       

next_test_3:
    # Test 3: BLT (Branch if Less Than) - Signed comparison
    addi a0, zero, -5         # a0 = -5
    addi a1, zero, 10         # a1 = 10
    blt a0, a1, branch_lt   
    nop                      
    
    # Check Test 3 result (Expected: branch is taken, so a0 = -5)
    li t0, -5                 # Load expected value (-5)
    bne a0, t0, fail_test     # If a0 != -5, jump to fail_test
    j next_test_4             # Jump to next test

branch_lt:
    # This is where the BLT instruction jumps
    nop                        

next_test_4:
    # Test 4: BGE (Branch if Greater or Equal) - Signed comparison
    addi a0, zero, 10         # a0 = 10
    addi a1, zero, -5         # a1 = -5
    bge a0, a1, branch_ge    
    nop                       
    
    # Check Test 4 result (Expected: branch is taken, so a0 = 10)
    li t0, 10                 # Load expected value (10)
    bne a0, t0, fail_test     # If a0 != 10, jump to fail_test
    j next_test_5             # Jump to next test

branch_ge:
    # This is where the BGE instruction jumps
    nop                      

next_test_5:
    # Test 5: BLTU (Branch if Less Than Unsigned)
    addi a0, zero, 5          # a0 = 5
    addi a1, zero, 10         # a1 = 10
    bltu a0, a1, branch_lt_unsigned
    nop                      
    
    # Check Test 5 result (Expected: branch is taken, so a0 = 5)
    li t0, 5                  # Load expected value (5)
    bne a0, t0, fail_test     # If a0 != 5, jump to fail_test
    j next_test_6             # Jump to next test

branch_lt_unsigned:
    # This is where the BLTU instruction jumps
    nop                      

next_test_6:
    # Test 6: BGEU (Branch if Greater or Equal Unsigned)
    addi a0, zero, 10         # a0 = 10
    addi a1, zero, 5          # a1 = 5
    bgeu a0, a1, branch_ge_unsigned 
    nop                      
    
    # Check Test 6 result (Expected: branch is taken, so a0 = 10)
    li t0, 10                 # Load expected value (10)
    bne a0, t0, fail_test     # If a0 != 10, jump to fail_test
    j success_test            # Jump to success_test

branch_ge_unsigned:
    # This is where the BGEU instruction jumps
    nop                        

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
