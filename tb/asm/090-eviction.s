main:
    # Initialize base addresses
    la x10, 0x0         # Base address in memory
    li x13, 0x1         # Data to write

    # Write data to addresses that cause conflicts in direct-mapped cache
    # Assuming cache has 8 sets in direct-mapped (3 index bits)
    # Addresses: 0x0, 0x40, 0x80 all map to the same set in direct-mapped cache
    sw x13, 0(x10)      # Write 0x1 to address 0x0
    addi x13, x13, 0x1  # Increment data
    sw x13, 64(x10)     # Write 0x2 to address 0x40
    addi x13, x13, 0x1  # Increment data
    sw x13, 128(x10)    # Write 0x3 to address 0x80

    # Read data back to test cache behavior
    lb x11, 0(x10)      # Read address 0x0 (expect miss in direct-mapped due to eviction)
    lb x11, 64(x10)     # Read address 0x40 (expect hit in two-way, miss in direct-mapped)
    lb x11, 128(x10)    # Read address 0x80 (expect hit in two-way, miss in direct-mapped)
    lb x11, 0(x10)      # Read address 0x0 again (miss in direct-mapped, hit in two-way)

    # Introduce additional accesses to force conflicts and evictions
    addi x13, x13, 0x1
    sw x13, 192(x10)    # Write 0x4 to address 0xC0 (maps to same set in direct-mapped)

    lb x11, 64(x10)     # Read address 0x40 (expect eviction in direct-mapped)
    lb x11, 128(x10)    # Read address 0x80 (expect hit in two-way, miss in direct-mapped)

    # End program
    nop
    nop
