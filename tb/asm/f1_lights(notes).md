    .text                   # Start of the code section
main:
    li      t2, 0x2000       # Load the memory address 0x2000 into register t2 (initial address for loop)
loop:
    lw      s0, 0(t2)        # Load a word from memory at the address pointed by t2 into register s0
    jal     ra, subroutine    # Jump to the subroutine and link the return address (saving return address in ra)
    j       loop              # Jump back to the 'loop' label, creating an infinite loop
    ret                       # Return from the main function (this is unreachable due to the infinite loop)
    
subroutine:
    # The following instructions set the value of register a0 to a series of values
    # representing a sequence of increasing "light states", as powers of 2 minus 1.

    addi    a0, zero, 0x1     # Set a0 to 0x1 (binary 00000001, 1 light on)
    addi    a0, zero, 0x3     # Set a0 to 0x3 (binary 00000011, 2 lights on)
    addi    a0, zero, 0x7     # Set a0 to 0x7 (binary 00000111, 3 lights on)
    addi    a0, zero, 0xf     # Set a0 to 0xf (binary 00001111, 4 lights on)
    addi    a0, zero, 0x1f    # Set a0 to 0x1f (binary 00011111, 5 lights on)
    addi    a0, zero, 0x3f    # Set a0 to 0x3f (binary 00111111, 6 lights on)
    addi    a0, zero, 0x7f    # Set a0 to 0x7f (binary 01111111, 7 lights on)
    
    ret                        # Return from the subroutine
