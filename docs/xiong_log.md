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

## 26/5/2025

Fixed the multithreaded reg-file (legit just had to initialise registers). Added convert from floats to ints and vice versa.



Note:
```
In addition, rounding is done after normalization because we always expect a possible carry-out (overflow of the leading bit).

This overflow may occur before rounding, and rounding itself may create another overflow, so rounding must be applied after final normalization.

In subtraction, we normalize first (by left-shifting) to bring the leading 1 to bit 23. But since subtraction can lead to leading zeros, normalization is guaranteed to precede rounding. There's no overflow risk, so rounding can be safely done inside the same block once normalization is complete.
```

## Reasons for SIMD instead of SIMT

#### SIMD on PYNQ-Z2 FPGA — What & Why
SIMD (Single Instruction, Multiple Data) means executing the same instruction on multiple data elements in parallel. It’s a natural fit for FPGA hardware because you can instantiate multiple datapath lanes running the same logic on different inputs simultaneously. FPGAs excel at parallelism: You can just replicate your compute logic and feed different data elements to each lane. Compared to SIMT, SIMD doesn’t need complex divergence control or per-thread PCs because all lanes execute the exact same instruction synchronously.

#### Why SIMD Is Easier on PYNQ-Z2
- Simple control logic: Only one PC for all lanes. No thread divergence: All lanes execute the same instruction every cycle.

- Less context storage: No per-thread state besides register files.

- Resource-friendly: You trade off LUTs/DSPs by replicating lanes but avoid complex control overhead.

- Fits well with FPGA fabric: Replicating datapaths is a classic FPGA pattern.

#### How SIMD Fits in Your PYNQ-Z2 Design
- Lane Replication:
Replicate your processing lane N times (depending on resource availability). Each lane handles one data element.

- Shared Instruction:
All lanes fetch and execute the same instruction in lockstep.

- Register Files:
Each lane typically has its own local registers to store per-data values.

- Memory Access:
You need to carefully design memory interfaces so each lane can read/write its own data efficiently (e.g., multiple ports or banking).

#### Resource Considerations on PYNQ-Z2
PYNQ-Z2’s Zynq-7000 (Z-7020) has moderate resources, so you can expect to implement a SIMD unit with several lanes (e.g., 4 to 8 lanes) comfortably depending on your datapath complexity.
If your compute is simple (e.g., integer ALU or simple FP ops), you can scale lane count higher.
For complex floating-point or DSP-heavy lanes, fewer lanes might fit.

