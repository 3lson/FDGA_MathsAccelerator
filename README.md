# FPGA K-Means Clustering Accelerator

<p align="left">
  <a href="https://en.wikipedia.org/wiki/SystemVerilog">
    <img src="https://img.shields.io/badge/HDL-SystemVerilog-1DBA5A?style=for-the-badge&logo=verilog" alt="Hardware Description Language">
  </a>
  <a href="https://www.cplusplus.com/">
    <img src="https://img.shields.io/badge/Software-C++ Toolchain-00599C?style=for-the-badge&logo=c%2B%2B" alt="Software">
  </a>
  <a href="https://www.xilinx.com/products/silicon-devices/soc/zynq-7000.html">
    <img src="https://img.shields.io/badge/Platform-Xilinx%20PYNQ--Z1-F26222?style=for-the-badge&logo=xilinx" alt="Platform">
  </a>
  <a href="https://www.xilinx.com/products/design-tools/vivado.html">
    <img src="https://img.shields.io/badge/Tools-Vivado%20%7C%20Verilator%20%7C%20GTest-76b900?style=for-the-badge" alt="Toolchain">
  </a>
</p>

## Project Overview

This repository contains the complete design and software stack for a high-performance, hardware-accelerated K-means clustering engine implemented on a Xilinx PYNQ-Z1 FPGA. The project demonstrates a full-stack approach to hardware acceleration, encompassing custom processor design, a bespoke ISA, a C++ compiler toolchain, and system-level performance optimization.

The final design achieved a **~12x performance uplift** over a standard single-threaded C++ implementation.

## System Architecture

The accelerator leverages a hardware/software co-design methodology, using the PYNQ's ARM Processing System (PS) for control and sequential tasks, while offloading the massively parallel computations to a custom-designed SIMD processor fabric on the Programmable Logic (PL).

![High-Level Architecture Diagram](https://i.imgur.com/your_diagram_url.png)

**TO BE ADDED**

> **Diagram:** A high-level overview showing the PYNQ's ARM PS running the main Python application, which configures and controls the K-Means Accelerator IP on the PL via an AXI interface. The accelerator reads input data (points, centroids) from and writes results (sums) back to shared DDR memory.

## Key Features

*   **Custom SIMD Processor Core:** A 16-lane SIMD processor designed from the ground up in SystemVerilog, tailored specifically for the K-means algorithm's computational patterns.
*   **Bespoke RISC-V Style ISA:** A custom 32-bit instruction set architecture featuring specialized vector and floating-point instructions to maximize efficiency for the target workload.
*   **Pipelined Microarchitecture:** The processor core features a 5-stage pipeline (Fetch, Decode, Register Read, Execute, Writeback) with a full forwarding unit to mitigate data hazards.
*   **C++ Compiler & Assembler:** A complete software toolchain built in C++ that compiles a C-like kernel language down to the custom machine code, enabling rapid development and algorithm optimization.
*   **Hardware/Software Co-design:** Intelligent partitioning of the K-means algorithm, where the parallel distance calculations are accelerated on the FPGA fabric, while final centroid division and control flow are handled by the ARM processor in Python.

## Branch Guide

This repository captures the project at several key stages of development. To understand the project's evolution from a CPU-based simulator to a final, optimized FPGA implementation, please refer to the following branches:

*   **`main`**: This is the primary branch and represents the **final, deployed version** of the project. It contains the end-to-end PYNQ implementation with all hardware and software components, fully optimized for performance, timing closure, and resource utilization on the target FPGA. This branch reflects the hardware/software co-design principles discussed in this README.

*   **`gpgpu-isa-version`**: This branch represents the **fully-featured ISA implementation** before final optimizations. It contains a more expansive set of hardware features and instructions that were later streamlined or offloaded to software to meet the strict timing and resource constraints of the PYNQ-Z1 platform.

*   **`ui`**: This is an experimental feature branch containing the work for a real-time **user interface using OpenCV**. It demonstrates how the accelerator could be integrated into a larger, interactive application.

*   **`v0.0`**: This branch is an early-stage **proof-of-concept and CPU-only simulator**. It contains the complete compiler-assembler toolchain and a C++ model of the custom CPU running in a single-threaded environment. This version was crucial for validating the custom ISA and toolchain *before* committing to hardware development.

## Deep Dive: Core Components

### 1. The SIMD Processor Core & Execution Model

The heart of the accelerator is a custom-designed processor core that acts as the controller for a warp of 16 threads.

*   **Execution Model:** The architecture follows a Single Instruction, Multiple Thread (SIMT) model. The main core fetches and decodes a single instruction which is then executed by the 16 lanes in parallel.
*   **Resource-Efficient Design:** A key architectural decision was to instantiate a single, shared **pipelined FPU and Integer ALU** for the entire warp. While each thread has its own register file, they are scheduled onto the shared execution units. This approach dramatically reduces resource utilization compared to instantiating 16 full ALUs, enabling the design to fit on a smaller FPGA while maintaining high throughput.
*   **Custom ISA:** The ISA was designed to be dense and efficient for the K-means workload. Key instructions include:
    *   `VFADD`, `VFSUB`: Vector floating-point add/subtract for distance calculations.
    *   `VABS`: Vector absolute value.
    *   `VSLT`: Vector float less-than to find the minimum distance.
    *   `VLD`/`VST`: Vector load/store with immediate offsets to access point and centroid data.
    *   `BEQZ`: Scalar branch-if-equal-to-zero for control flow.

### 2. C++ Compiler and Assembler

A significant part of this project was developing the software to target the custom hardware. The toolchain operates in two stages:

1.  **Compiler:** Parses a simplified, C-like kernel syntax. It performs register allocation and translates high-level operations (e.g., `centroids[0] - points[i]`) into a sequence of assembly instructions for the custom ISA.
2.  **Assembler:** Converts the human-readable assembly instructions into their final 32-bit binary machine code representation, which is then loaded into the FPGA's memory for execution.

This toolchain was fundamental to the project's success, allowing for quick iteration on the K-means algorithm without needing to write machine code by hand.

## Relevant resources:
[FPGA-based implementation of signal processing systems; Second editon.
Roger Woods, John McAllister, Gaye Lightbody, Ying Yi.](https://library-search.imperial.ac.uk/discovery/fulldisplay?docid=alma991000933953101591&context=L&vid=44IMP_INST:ICL_VU1&lang=en&search_scope=MyInst_and_CI&adaptor=Local%20Search%20Engine&tab=Everything&query=any,contains,Digital%20Signal%20Processing%20with%20FPGAs)

[Computer architecture : a quantitative approach ; Fifth edition.
John L. Hennessy, David A. Patterson](https://library-search.imperial.ac.uk/discovery/fulldisplay?docid=alma9910112404401591&context=L&vid=44IMP_INST:ICL_VU1&lang=en&search_scope=MyInst_and_CI&adaptor=Local%20Search%20Engine&isFrbr=true&tab=Everything&query=any,contains,computer%20architecture%20john%20hennessy&sortby=date_d&facet=frbrgroupid,include,9015661278415079959&offset=0)

[Computer organization and design RISC-V edition : the hardware software interface ; Second edition.; RISC-V edition.
David A. Patterson, John L. Hennessy](https://library-search.imperial.ac.uk/discovery/fulldisplay?docid=alma991000613172401591&context=L&vid=44IMP_INST:ICL_VU1&lang=en&search_scope=MyInst_and_CI&adaptor=Local%20Search%20Engine&isFrbr=true&tab=Everything&query=any,contains,computer%20architecture%20john%20hennessy&sortby=date_d&facet=frbrgroupid,include,9035044794922040673&offset=0)
