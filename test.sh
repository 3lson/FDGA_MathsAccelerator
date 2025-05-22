# Basic test script for asm to CPU output
# Will be changed 


# Assemble the ASM code

cd assembler
g++ -std=c++17 assembler.cpp -o assembler
./assembler program.asm  


# Compiling the CPU output
cd ..
make clean
make