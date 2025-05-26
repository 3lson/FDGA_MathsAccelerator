#!/bin/bash
# Navigate to project root (update path if needed)
cd "$(dirname "$0")/.."

# Step 1: Run the compiler test script
python3 scripts/test.py

# Step 2: Compile the assembler with C++17
# Step 3: Run the assembler
g++ -std=c++17 -o assembler/assembler assembler/assembler.cpp
./assembler/assembler
