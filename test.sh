# Basic test script for asm to CPU output
# Will be changed 


# Assemble the ASM code

g++ -o assembler/assembler assembler/assembler.cpp
./assembler/assembler     


# Compiling the CPU output
cd ..
make clean
make