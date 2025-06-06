// common.sv
`ifndef COMMON_SV
`define COMMON_SV

// Global Macros
`define DATA_WIDTH 32
`define INSTRUCTION_WIDTH 32
`define DATA_MEMORY_ADDRESS_WIDTH 32
`define INSTRUCTION_MEMORY_ADDRESS_WIDTH 32

// Type Definitions
typedef logic [`DATA_WIDTH-1:0] data_t;
typedef logic [`INSTRUCTION_WIDTH-1:0] instruction_t;
typedef logic [`DATA_MEMORY_ADDRESS_WIDTH-1:0] data_memory_address_t;
typedef logic [`INSTRUCTION_MEMORY_ADDRESS_WIDTH-1:0] instruction_memory_address_t;

typedef struct packed {
    instruction_memory_address_t base_instructions_address;
    data_memory_address_t base_data_address; // This is where kernel function arguments are stored
    data_t num_blocks;
    data_t num_warps_per_block;
} kernel_config_t;

// RISC-V Definitions
`define OPCODE_WIDTH 7
`define FUNCT3_WIDTH 3
`define FUNCT4_WIDTH 4

// Halt Instruction Opcode
`define OPCODE_HALT     7'b1111111

// Vector-Scalar Instruction Opcodes (SX_SLTI and SX_SLT)
// SX_SLTI sets one bit of a scalar register based on thread's comparison result
`define OPCODE_SX_SLT   7'b1111110        // SX_SLT rd, rs1, rs2 <=> rd[id] = rs1 < rs2 ? 1 : 0
`define OPCODE_SX_SLTI  7'b1111101        // SX_SLTI rd, rs1, imm <=> rd[id] = rs1 < imm ? 1 : 0

// Instruction Opcodes
// The entire opcode is 7 bits, the most significant bit decides whether the instruction is vector or scalar
`define OPCODE_R        3'b000          // Used by all R-type instructions (ADD, SUB, SLL, SLT, XOR, SRL, SRA)
`define OPCODE_I        3'b001          // Used by ALU I-type instructions (ADDI, SLTI, XORI, ORI, ANDI, SLLI, SRLI, SRAI)
`define OPCODE_S        3'b100          // Used by store instructions (SB, SH, SW)
`define OPCODE_P        3'b011          // Used by LUI
`define OPCODE_LOAD     3'b100          // Used by load instructions (LB, LH, LW)
`define OPCODE_F        3'b010          // Floating Point F-Type

// Those instructions can only be used by scalar instructions
`define OPCODE_C        3'b111          // Control Flow C-Type

typedef logic [`OPCODE_WIDTH-1:0] opcode_t;
typedef logic [`FUNCT3_WIDTH-1:0] funct3_t;
typedef logic [`FUNCT4_WIDTH-1:0] funct4_t;
typedef logic [11:0] imm12_t;

// alu instructions enum
typedef enum logic [4:0] {
    // register instructions
    ADD,
    SUB,
    MUL,
    DIV,
    SLT,
    SGT,
    SEQ,
    SNEZ,
    MIN,
    ABS,

    ADDI,
    MULI,
    DIVI,
    SLLI,

    // compare instructions
    BEQ,

    // jump instructions
    JAL
} alu_instruction_t;

typedef enum logic [3:0] {
    FADD,
    FSUB,
    FMUL,
    FDIV,
    FLT,
    FNEG,
    FEQ,
    FMIN,
    FABS,
    FCVT_W_S,
    FCVT_S_W
} alu_f_instruction_t;

// warp state enum
typedef enum logic [2:0] {
    WARP_IDLE,
    WARP_FETCH,
    WARP_DECODE,
    WARP_REQUEST,
    WARP_WAIT,
    WARP_EXECUTE,
    WARP_UPDATE,
    WARP_DONE
} warp_state_t;

// fetcher state enum
typedef enum logic [1:0] {
    FETCHER_IDLE,
    FETCHER_FETCHING,
    FETCHER_DONE
} fetcher_state_t;

// lsu state enum
typedef enum logic [1:0] {
    LSU_IDLE,
    LSU_REQUESTING,
    LSU_WAITING,
    LSU_DONE
} lsu_state_t;

// reg input mux
typedef enum logic [2:0] {
    ALU_OUT,
    LSU_OUT,
    IMMEDIATE,
    PC_PLUS_1,
    VECTOR_TO_SCALAR
} reg_input_mux_t;

// sign extend function
function automatic data_t sign_extend(imm12_t imm12);
    data_t signed_imm12;
    if (imm12[11]) begin
        signed_imm12 = {{20{1'b1}}, imm12};
    end else begin
        signed_imm12 = {{20{1'b0}}, imm12};
    end
    return signed_imm12;
endfunction

// sign extend function for 13-bit immediate values
function automatic data_t sign_extend_13(logic[12:0] imm13);
    data_t signed_imm13;
    if (imm13[12]) begin
        signed_imm13 = {{19{1'b1}}, imm13};
    end else begin
        signed_imm13 = {{19{1'b0}}, imm13};
    end
    return signed_imm13;
endfunction

// sign extend function for 21-bit immediate values
function automatic data_t sign_extend_21(logic[20:0] imm21);
    data_t signed_imm21;
    if (imm21[20]) begin
        signed_imm21 = {{11{1'b1}}, imm21};
    end else begin
        signed_imm21 = {{11{1'b0}}, imm21};
    end
    return signed_imm21;
endfunction

`endif // COMMON_SV
