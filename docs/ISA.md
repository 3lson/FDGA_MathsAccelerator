# Custom ISA: IPPro-Inspired 


## Overview  
The custom ISA is designed to efficiently execute K-means clustering algorithms on custom FPGA-based hardware.
It will include:
- vector distance computation
- cluster assignment
- centroid update
- memory-efficient data streaming

## Bit Encoding Overview
All instr are 32 bit wide

| Field | Bits | 
| ------| -----|
| opcode | [31:29]|
| predicate | [28] |
| payload | [27:0] |

**Note:** If predicate is `1`, instruction executes only if pred condition holds

## Instruction Types

### R-type (Register-Register Arith)
`opcode = 000`

| [31:29] | [28] | [27:20] | [19:15] | [14:10] | [9:5] | [4:0]
|--------|----|-----|------|-----|-----|----|
|opcode| pred | don't care (x) | RS2 | FUNCT5 | RS1 | RD |

|Mnemonic|FUNCT5|Description|
| -------| -----|-----------|
| `ADD` | 00000| rd = rs1 + rs2 |
| `SUB` | 00001 | rd = rs1 - rs2 |
| `MUL` | 00010 | rd = rs1 * rs2 |
| `MAC` | 00011 | rd = rd + (rs1 * rs2) |
| `DIV` | 00100 | rd = rs1 / rs2 |
| `REM` | 00101 | rd = rs1 % rs2 |
| `SLL` | 01010 | rd = rs1 << rs2 |
| `SRL` | 01011 | rd = rs1 >> rs2 (logical) |
| `SQRT` | 01100 | rd = sqrt(rs1) |
| `SLT` | 11100 | rd = (rs1 < rs2) ? 1 : 0 |
| `SEQ` | 11101 | rd = (rs1 == rs2) ? 1 : 0 |
| `NOP` | 11111 | No operation  |

### I-type (Immediate Arith)
`opcode = 001`

| [31:29] | [28] | [27:15] | [14:10] | [9:5] | [4:0]
|--------|----|-----|---------|----|--|
|opcode| pred | IMM [13:0] | FUNCT5 | RS1 | RD |

|Mnemonic|FUNCT5|Description|
| -------| -----|-----------|
| `ADDI` | 00000| rd = rs1 + imm |
| `MULI` | 00010 | rd = rs1 * imm |
| `MACI` | 00011 | rd = rd + (rs1 * imm) |
| `DIVI` | 00100 | rd = rs1 / imm |
| `CLAMP` | 00110 | rd = clamp(rs1, min=0, max=imm) |
| `VLOADLEN` | 01000 | rd = vector_load_len(vector mode setup) |
| `VSETDIM` | 01001 | rd = dimension length(vector mode setup) |

### M-type (Memory Access)
`opcode = 100` and `predicate = bit 28`, and `FUNCT3 = [27:25]`

**LOAD** `lw rd, (rs1)`

| [31:29] | [28] | [27:25] | [24:15] | [14:10] | [9:5] | [4:0]
|--------|----|-----|-------|--|-----|----|
|opcode| pred | FUNCT3=000 | unused | RS1 | RD | unused |

**STORE** `sw rs2, (rs1)`

| [31:29] | [28] | [27:25] | [24:20]| [19:15] | [14:10] | [9:0]
|--------|----|-----|------|---|-----|----|
|opcode| pred | FUNCT3=001 |RS2 | unused | RS1 | unused |

**STREAMLOAD** `streamload v0, x3, x4`

This is for streaming a vector
(e.g features of a data point) into a bank of registers or internal vector buffer
(i.e load vector length x4 from memory pointer to by x3 into vector registers starting at v0)

| [31:29] | [28] | [27:25] | [24:20] | [19:15] | [14:10] | [9:0]
|--------|----|-----|--|---|----|-----|
|opcode| pred | FUNCT3 = 010 | VREG_IDX | RS1| RS2| unused |

**STOREVEC** `storevec v0, x3, x4`

(Not sure if needed) -> to support writing back an entire centroid or feature vector 

| [31:29] | [28] | [27:25] | [24:20] | [19:15] | [14:10] | [9:0]
|--------|----|-----|---|--|----|-----|
|opcode| pred | FUNCT3 = 011 | VREG_IDX | RS1 | RS2 | unused |

### X-type  (Application-Specific Accelerators)
`opcode = 101`

| [31:29] | [28:27] | [26:15] | [14:3] | [2:0]
|--------|----|-----|----------|--|
|opcode| FUNCT2 | ptr1/pt_ptr/cent_ptr | ptr2/cent_ptr/K/COUNT | unused |

|Mnemonic|FUNCT2|Description|
| -------| -----|-----------|
| `DIST_VEC` | 00| Compute squared L2 distance between two vectors |
| `ASSIGN_CLUSTER` | 01 | Assign point to closest centroid |
| `UPDATE_CENTROID` | 10 | Add point to centroid accumulator |
| `FINALIZE_CENTROID` | 11 | Divide centroid accumulator by count |


### C-type  (Control Flow)
`opcode = 111`

| [31:29] | [28:12] | [26:15] | [14:3] | [2:0]
|--------|----|-----|----------|--|
|opcode| Imm/Reserved | FUNCT3| Imm/RS1/unused | RD/unused/reserved |

|Mnemonic|Encoding|Description|
| -------| -----|-----------|
| `JUMP` | [31:29]=111, [28:12]=imm[27:11], [11:9]=000, [8:2]=imm[10:4], [1:0]=x| Unconditional jump to immediate address |
| `BRANCH` |[31:29]=111, [28:12]=imm[27:11], [11:9]=001, [8:2]=imm[10:4], [1:0]=x | Branch if zero condition to immediate address |
| `CALL` | [31:29]=111, [28:10]=imm[27:9], [9:5]=RS1, [4:0]=RD | Call subroutine, save return in RD |
| `RET` | [31:29]=111, [28:13]=x, [12:10]=011, [9:5]=RS1, [4:0]=x | Return from subroutine using RS1 |
| `SYNC` | [31:29]=111, [28:13]=x, [12:10]=110, [9:0]=x | Memory sync barrier |
| `EXIT` | [31:29]=111, [28:13]=x, [12:10]=111, [9:0]=x | Exit execution|


## Register File 
32 General Purpose Registers
**Special Registers**

| Reg | Name | Description |
|--------|----|-----|
|x0| `zero` | Constant 0 (read-only)|
|x1| `BlockIdx` | Block ID (runtime-assigned)|
|x2| `BlockDim` | Number of SIMD lanes (constant)|
|x3| `ThreadIdx` | Thread index within the SIMD unit|

## Design Rationale for K-means 

| Operation | Instruction | 
| --------| ---|
|Distance calc | `DIST_VEC`, `SUB`, `MAC`, `SQRT` |
|Cluster Assignment | `ASSIGN_CLUSTER` |
|Centroid accumulation | `UPDATE_CENTROID`, `MAC`, `ADD` |
|Average Computation | `FINALIZE_CENTROID`, `DIV` |
|Memory streaming | `STREAMLOAD`, `STORE` |
|Control + iteration | `BRANCH`, `JUMP`, `CALL`, `RET` |


