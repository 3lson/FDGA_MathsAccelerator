## 20/5/2025

Finalised project, concluded on the k-means algorithm. Began research on compiler optimisation and SIMD implementation in RISC-V Architecture.

## 21/5/2025

Created the floating point unit with simple addition and subtraction.

## 22/5/2025

Researching how to implement SIMD, implementing vector operations into the ALU.
Finished ALU except for FDIV instruction, unknown reason as to why it's not working.
To run the testbench, place tb_floating_alu.cpp in the obj_dir in the rtl folder and run the following:

## 24/5/2025

Created a regfile for the multithreading, doesn't seem to be writing properly yet but structure should be there

```
verilator -Wall --cc floating_alu.sv --exe tb_floating_alu.cpp
make -C obj_dir -j -f Vfloating_alu.mk Vfloating_alu
./obj_dir/Vfloating_alu
```

Note:
```
In addition, rounding is done after normalization because we always expect a possible carry-out (overflow of the leading bit).

This overflow may occur before rounding, and rounding itself may create another overflow, so rounding must be applied after final normalization.

In subtraction, we normalize first (by left-shifting) to bring the leading 1 to bit 23. But since subtraction can lead to leading zeros, normalization is guaranteed to precede rounding. There's no overflow risk, so rounding can be safely done inside the same block once normalization is complete.
```