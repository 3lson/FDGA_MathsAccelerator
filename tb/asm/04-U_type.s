    .globl main
main:
    # Test 1: LUI (Load Upper Immediate)
    lui a0, 0x12345          # a0 = 0x12345 << 12 = 0x12345000
    nop                       # No operation, just to separate the tests

    # Check Test 1 result (Expected: a0 = 0x12345000)
    li t0, 0x12345000        # Load expected value (0x12345000)
    bne a0, t0, fail_test    # If a0 != 0x12345000, jump to fail_test
    j next_test_2            # Jump to next test

next_test_2:
    # Test 2: AUIPC (Add Upper Immediate to PC)
    auipc a0, 0x12345        # a0 = PC + (0x12345 << 12)
    nop                       # No operation, just to separate the tests

    # Check Test 2 result
    li t1, 0x12345014        # change reflects AUIPC functionality PC + signExt(U)
    bne a0, t1, fail_test    # If a0 != 0x12356000, jump to fail_test
    j success_test           # Jump to success_test

fail_test:
    # If any test fails, set a0 to -1 to indicate failure
    li a0, -1                # Set a0 to -1 to indicate failure
    j end_tests              # Jump to end_tests

success_test:
    # All tests passed, set a0 to 150 to indicate success
    li a0, 150               # Set a0 to 150 to indicate success
    j end_tests              # Jump to end_tests

end_tests:
    # All tests completed, result in a0
    nop
