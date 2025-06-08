#!/bin/bash
# Navigate to project root (update path if needed)
cd "$(dirname "$0")/.."

# run docker environment
./docker.sh

# Step 1: Run the compiler test script
python3 scripts/test.py -m

#exit docker
exit 

# Step 2: Compile the assembler with C++17
# Step 3: Run the assembler
g++ -std=c++17 -o assembler/assembler assembler/assembler.cpp
./assembler/assembler

#Run the rtl
cd tb
./doit.sh
