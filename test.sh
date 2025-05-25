#!/bin/bash

if ! python3 --version > /dev/null 2>&1; then
    . scripts/test.sh
else
    ./scripts/test.py $@
fi


# Temp holding for assembler run instr
g++ -o assembler/assembler assembler/assembler.cpp
./assembler/assembler     