# Basic test script for asm to CPU output
# Will be changed 

# Assemble the ASM code

g++ -o assembler/assembler assembler/assembler.cpp
./assembler/assembler     


# Compiling the CPU output

cd "$(dirname "$0")/tb" || exit 1

# Run the test script, pass all arguments
./doit.sh "$@"