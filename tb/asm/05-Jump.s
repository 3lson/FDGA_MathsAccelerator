main:
    # Test 1: JAL (Jump and Link)
    # We will jump to a label (test_jump) and return to continue the program
    jal a0, test_jump         # a0 = PC + 4, jump to test_jump
    nop                       # No operation, just to separate the tests

test_jump:
    # This is where the JAL instruction jumps to
    nop                        # Just a dummy instruction for the jump to target
    nop                        # Another dummy instruction
    jalr x0, x0, 0           