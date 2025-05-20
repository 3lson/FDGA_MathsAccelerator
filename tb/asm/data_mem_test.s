main:
    li x10, 0x12345678      # Pseudo-instruction: regfile[x10] = 0x12345678
    li x11, 0xFF886543      # Pseudo-instruction: regfile[x11] = 0xFF886543

    #testing store word instructions
    sw x10, 0(x19)          # Store the word from regfile[x10] into data location indexed 0
    sw x11, 4(x19)          # Store the word from regfile[x11] into data location indexed 4
    
    #testing load instructions
    lb x12, 0(x19)          # Load first byte test    | regfile[x12] = 0x00000078 | 
    lh x13, 6(x19)          # Load half word test     | regfile[x13] = 0xFFFFFF88 | tests sign extension
    lw x14, 0(x19)          # Load word test          | regfile[x14] = 0x12345678 | 
    lbu x15, 6(x19)         # Load unsigned byte test | regfile[x15] = 0x00000088 |
    lhu x16, 6(x19)         # Load half word unsigned | regfile[x16] = 0x0000FF88 | unsigned equivalent to line 11


    #testing store instructions
    sb x10, 8(x19)           # store byte              | data_mem[x8] = 0x78        |
    lw x17, 8(x19)           # store byte test         | regfile[x17] = 0x00000078  | 
    sh x10, 12(x19)          # store halfword          | data_mem[x13,x12] = 0x5678 |
    lw x18, 12(x19)          # store halfword test     | regfile[x18] = 0x00005678  | 

# load instructions used to test store instructions as makes running verification tests clearer as
# a0 result output corresponds to the stored instruction.
# regfile[x11] = 32'b0, therefore using 'base + offset' addressing we accessing the first 4 (0 through to 3)
# memory locations in the data memory.