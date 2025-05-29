## Here I will document the design choices I have done for the assembler and compiler

- x28-31 are reserved as registers for multithreading
- fixed size of arrays and no.of clusters and points to make the compilation easier
- we will treat floating point registers to a register file mapping on fixed-point for the rtl
- `flw` and `fsw` will be treated the same as `lw` and `sw`
- major changes to hardcoding data types for float to int conversions in compiler


Updates: (29/05/25)
**Issue 1:** 
- treating `ft0` and `t0` the same is not a viable option as `t0` used to calculate memory address while `ft0` suppose to store float value but overwritten

```
lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -264
flw ft0, 0(t0)

lw t0, -120(s0)
slli t0, t0, 2
add t0, t0, s0
addi t0, t0, -240
fsw ft0, 0(t0)
```

There were 1 solns:
1) We create a new float reg file
