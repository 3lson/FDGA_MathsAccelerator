## Here I will document the design choices I have done for the assembler and compiler

- x28-31 are reserved as registers for multithreading
- fixed size of arrays and no.of clusters and points to make the compilation easier
- we will treat floating point registers to a register file mapping on fixed-point for the rtl
- `flw` and `fsw` will be treated the same as `lw` and `sw`
- major changes to hardcoding data types for float to int conversions in compiler