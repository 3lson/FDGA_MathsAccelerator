# Custom ISA

## Overview  
The custom ISA is designed to efficiently execute K-means clustering algorithms on custom FPGA-based hardware.

## Bit Encoding Overview
All instr are 32 bit wide

| Field | Bits | 
| ------| -----|
| opcode | [31:29]|
| predicate | [28] |

**Note:** If predicate is `1`, instruction executes only if pred condition holds

## Instruction Types

### R-type (Register-Register Arith)
`opcode = 000`

| [31:29] | [28] | [27:19] | [18:14] | [13:10] | [9:5] | [4:0]
|--------|----|-----|------|-----|-----|----|
|opcode| pred |  9(x) | RS2 | FUNCT4 | RS1 | RD |

|Mnemonic|FUNCT5|Description|
| -------| -----|-----------|
| `ADD` | 0000| rd = rs1 + rs2 |
| `SUB` | 0001 | rd = rs1 - rs2 |
| `MUL` | 0010 | rd = rs1 * rs2 |
| `DIV` | 0011 | rd = rs1 / rs2 |
| `SLT` | 0100 | rd = (rs1 < rs2) ? 1 : 0 |
| `SEQ` | 0101 | rd = (rs1 == rs2) ? 1 : 0 |
| `MIN` | 0110 | rd = min(rs1, rs2) |
| `ABS` | 0111 | absolute value |

### I-type (Immediate Arith)
`opcode = 001`

| [31:29] | [28] | [27:14] | [13:10] | [9:5] | [4:0]
|--------|----|-----|---------|----|--|
|opcode| pred | IMM [13:0] (14 bits) | FUNCT4 | RS1 | RD |

|Mnemonic|FUNCT4|Description|
| -------| -----|-----------|
| `ADDI` | 0000| rd = rs1 + imm |
| `MULI` | 0001 | rd = rs1 * imm |
| `DIVI` | 0100 | rd = rs1 / imm |

### M-type (Memory Access)
`opcode = 100`

**LOAD** `lw rd, (rs1)`

| [31:29] | [28:14] | [13:10] | [9:5] | [4:0]
|--------|----|-------|--|-----|
|opcode| IMM [14:0] (15 bits) | FUNCT4 = 0000 | RS1 | RD |

**STORE** `sw rs2, (rs1)`

| [31:29] | [28:19] | [18:14] | [13:10] | [9:5] | [4:0]|
|--------|----|-----|------|---|-----|
|opcode| IMM [14:5] | RS2|FUNCT4=0001 | RS1 | IMM [4:0] |


### C-type  (Control Flow)
`opcode = 111`

**JUMP**

| [31:29] | [28:13] | [12:10] | [9:0] |
| -------| -------- | ------- | ------ |
| opcode | Imm[27:12] | FUNCT3 = 000 | Imm[11:2] |


**BRANCH** 

| [31:29] | [28:13] | [12:10] | [9:0] |
| -------| -------- | ------- | ------ |
| opcode | Imm[27:12] | FUNCT3 = 001 | Imm[11:2] |

**CALL** `call rd, imm(rs1)`

| [31:29] | [28:13] | [12:10] | [9:5] | [4:0]
| -------| -------- | ------- | ------ | ------ |
| opcode | Imm[17:2] | FUNCT3 = 010 | RS1 | RD|

**RET** `ret`

| [31:29] | [28:13] | [12:10] | [9:5] | [4:0]
| -------| -------- | ------- | ------ | ------ |
| opcode | 16{1'b0} | FUNCT3 = 011 | 5'b00001 | 5(x)|

**SYNC** 

| [31:29] | [28:13] | [12:10] | [9:0]
| -------| -------- | ------- | ------ |
| opcode | 16(x) | FUNCT3 = 110 | 10(x) |

**EXIT** 

| [31:29] | [28:13] | [12:10] | [9:0] |
| -------| -------- | ------- | ------ | 
| opcode | 16(x) | FUNCT3 = 111 | 10(x)|

### P-type (PseudoInstr)
This is a special instruction to resolve `lui`

`li rd` to resolve doing `lui rd, value[31:14]` and `addi rd, rd, value[13:0]`

### X-type 
`opcode = 101`

**Note:** This will be left empty but reversed for when we come to SIMD

| [31:29] | [28:27] | [18:15] | [14:3] | [2:0]
|--------|----|-----|----------|--|

