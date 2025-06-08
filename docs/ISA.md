# Elson-V ISA


## Overview  
The custom ISA is designed to efficiently execute K-means clustering algorithms on custom FPGA-based hardware.

## Bit Encoding Overview
All instr are 32 bit wide

| Field | Bits | 
| ------| -----|
| opcode | [31:29]|
| scalar | [28] |

## Instruction Types

### R-type (Register-Register Arith)
`opcode = 000`

| [31:29] | [28] | [27:19] | [18:14] | [13:10] | [9:5] | [4:0]
|--------|----|-----|------|-----|-----|----|
|opcode| scalar |  9(x) | RS2 | FUNCT4 | RS1 | RD |

|Mnemonic|FUNCT4|Description|
| -------| -----|-----------|
| `add` | 0000| rd = rs1 + rs2 |
| `sub` | 0001 | rd = rs1 - rs2 |
| `mul` | 0010 | rd = rs1 * rs2 |
| `div` | 0011 | rd = rs1 / rs2 |
| `slt` | 0100 | rd = (rs1 < rs2) ? 1 : 0 |
| `sll` | 0101 | rd = rs1 << rs2 |
| `seq` | 0110 | rd = (rs1 == rs2) ? 1 : 0 |
| `snez` | 0111 | rd = (rs1 != 0) ? 1 : 0 (rs2 is don't cares here) |
| `min` | 1000 | rd = min(rs1, rs2) |
| `abs` | 1001 | absolute value |

### I-type (Immediate Arith)
`opcode = 001`

| [31:29] | [28] | [27:14] | [13:10] | [9:5] | [4:0]
|--------|----|-----|---------|----|--|
|opcode| scalar | IMM [13:0] (14 bits) | FUNCT4 | RS1 | RD |

|Mnemonic|FUNCT4|Description|
| -------| -----|-----------|
| `addi` | 0000| rd = rs1 + imm |
| `muli` | 0010 | rd = rs1 * imm |
| `divi` | 0011 | rd = rs1 / imm |
| `slli` | 1010 | rd = rs1 << uimm (shift left logical immediate) | 

**Note:** `uimm` means 5-bit unsigned immediate (i.e `IMM[4:0]`)

### M-type (Memory Access)
`opcode = 100`

**LOAD** `lw rd, imm(rs1)`

| [31:29] | [28:14] | [13] | [12:10] | [9:5] | [4:0]
|--------|----|---|-------|--|-----|
|opcode| IMM [14:0] (15 bits) | scalar | FUNCT3 = 000 | RS1 | RD |

**STORE** `sw rs2, imm(rs1)`

| [31:29] | [28:19] | [18:14] | [13] | [12:10] | [9:5] | [4:0]|
|--------|----|-----|---|------|---|-----|
|opcode| IMM [14:5] | RS2| scalar |FUNCT3=001 | RS1 | IMM [4:0] |

**FLOAT LOAD** `flw ft0, imm(t0)`

| [31:29] | [28:14] | [13] | [12:10] | [9:5] | [4:0]
|--------|----|---|-------|--|-----|
|opcode| IMM [14:0] (15 bits) | scalar | FUNCT3 = 010 | RS1 | RD |

**FLOAT STORE** `fsw ft0, imm(t0)`

| [31:29] | [28:19] | [18:14] | [13] | [12:10] | [9:5] | [4:0]|
|--------|----|-----|------|---|---|-----|
|opcode| IMM [14:5] | RS2| scalar | FUNCT3=011 | RS1 | IMM [4:0] |


**Note:** `flw` and `fsw` will be treated the same as `lw` and `sw` respectively

### C-type  (Control Flow)
`opcode = 111`

**jump** `j`

| [31:29] | [28:13] | [12:10] | [9:0] |
| -------| -------- | ------- | ------ |
| opcode | Imm[27:12] | FUNCT3 = 000 | Imm[11:2] |


**branch** `beqz rd, label`
**Note:** The label will be calculated in assembler to give the relative PC offset addressing
**branch** `beqz rd, label`
**Note:** The label will be calculated in assembler to give the relative PC offset addressing

| [31:29] | [28:19] | [18:14] | [13] | [12:10] | [9:5] | [4:0] |
| -------| -------- | ------ |---- |------- | ------ | ------- |
| opcode | Imm[17:8] |RS2(x0)| Imm[7] | FUNCT3 = 001 | RS1 | Imm[6:2] |

**call** `call rd, imm(rs1)`

| [31:29] | [28:13] | [12:10] | [9:5] | [4:0]
| -------| -------- | ------- | ------ | ------ |
| opcode | Imm[17:2] | FUNCT3 = 010 | RS1 | RD|

**ret** `ret`

| [31:29] | [28:13] | [12:10] | [9:5] | [4:0]
| -------| -------- | ------- | ------ | ------ |
| opcode | 16{1'b0} | FUNCT3 = 011 | 5'b00001 | 5(x)|

**sync** 

| [31:29] | [28:13] | [12:10] | [9:0]
| -------| -------- | ------- | ------ |
| opcode | 16(x) | FUNCT3 = 110 | 10(x) |

**exit** 

| [31:29] | [28:13] | [12:10] | [9:0] |
| -------| -------- | ------- | ------ | 
| opcode | 16(x) | FUNCT3 = 111 | 10(x)|

### P-type (PseudoInstr)
`opcode = 011`

`li t0, 100` 
This is a pseudo-instr that underlying would perform `addi t0, zero, 100`

`lui rd upimm`


| [31:29] | [28:9] | [8:6] | [5] | [4:0] |
| -------| ---- | ------ | ------ | -------|
| opcode | UpIMM[31:12] | 3(x) | scalar | RD |

**Note:** `upimm` gives 20-bit upper immediate IMM[31:12]


### X-type 
`opcode = 101`

**Note:** This will be left empty but reversed for when we come to SIMD

| [31:29] | [28:27] | [18:15] | [14:3] | [2:0]
|--------|----|-----|----------|--|

### F-type 
These are arithmetic instructions for FPU block
`opcode = 010`

| [31:29] | [28] | [27:19] | [18:14] | [13:10] | [9:5] | [4:0]
|--------|----|-----|------|-----|-----|----|
|opcode| scalar |  9(x) | RS2 | FUNCT4 | RS1 | RD |

|Mnemonic|FUNCT4|Description|
| -------| -----|-----------|
| `fadd.s` | 0000| rd = rs1 + rs2 |
| `fsub.s` | 0001 | rd = rs1 - rs2 |
| `fmul.s` | 0010 | rd = rs1 * rs2 |
| `fdiv.s` | 0011 | rd = rs1 / rs2 |
| `flt.s` | 0100 | rd = (rs1 < rs2) ? 1 : 0 |
| `fneg.s` | 0101 | Negate 32-bit floating point value in rs1 and store the result in rd (rs2 will be don't cares here) |
| `feq.s` | 0110 | rd = (rs1 == rs2) ? 1 : 0 |
| `fmin.s` | 0111 | rd = min(rs1, rs2) |
| `fabs.s` | 1000 | absolute value |
| `fcvt.w.s` | 1001 | Convert a floating-point number in floating-point register rs1 to a signed 32-bit in integer register rd. (rs2 will be don't cares here)|
| `fcvt.s.w` | 1010 | Convert a signed 32-bit integer in integer register rs1 to a signed 32-bit in integer floating-point number in floating-point register rd . (rs2 will be don't cares here)|

## Register File Assignment
We have chosen to stick with the RISCV Register layout with the addition of some extra special registers on our end for multithreading

**Int Scalar Register File**

| Name | Register Number | Usage |
| -----| ----------------| ------|
| zero | x0 | constant value 0 |
| ra | x1 | return address | 
| sp | x2 | stack pointer |
| gp | x3 | global pointer | 
| tp | x4 | thread pointer |
| s0 | x5 | protected |
| s1-s25 | x6-x30 | saved registers | 
| s26 | x31| reserved register | 

**Float Scalar Register File**

| Name | Register Number | Usage |
| -----| ----------------| ------|
| fs0-fs30 | x0-x30 | saved registers | 
| fs31 | x31 | reserved register | 


The following are mapped as the special registers

 | Register Number | Purpose | 
 | ---------------- |--------|
 | x29 | threadIdx | 
 | x30 | blockIdx |
 | x31 | block_size | 

**Int Vector Register File**

| Name | Register Number | Usage |
| -----| ----------------| ------|
| zero | x0 | constant value 0 |
| v1-v28 | x1-x28 | saved registers | 
| v29-v31 | x29-x31 | special registers | 

**Float Vector Register File**

The design choice of our float registers are as follows. Note reg convention for floats are not applied to that of RISCV as it is not necessary for our algorithm

| Name | Register Number | Usage |
| -----| ----------------| ------|
| zero | x0 | constant value 0 |
| fv1-fv28 | x1-x28 | saved registers | 
| fv29-fv31 | x29-x31 | special registers | 