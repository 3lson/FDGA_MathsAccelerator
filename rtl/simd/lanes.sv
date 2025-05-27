module lanes (

    input logic         clk,
    input logic         stall,
    input logic [3:0]   threads,
    input logic [31:0]  bIdx,
    input logic [4:0]   FUNCT4,
    input logic [31:0]  IMM,
    input logic [4:0]   AD1,
    input logic [4:0]   AD2,
    input logic [4:0]   AD3,

    // types?
    // input logic is_int,
    // input logic is_imm,
    // input logic is_fadd,
    
    input logic         WE3,
    
    // predicates, need implementation later
    // input logic is_predicate_setter,
    // input logic is_predicate_getter,

    // for mmu?
    // input logic [8:0] mmu_read_data,
    // input logic is_memforce,
    // input logic is_mem,
    // output logic mmu_write_en,
    // output logic mmu_write_force,
    // output logic [8:0] mmu_write_data,
    // output logic [17:0] mmu_addr,

    // Need to fit this somewhere
    // input   logic [4:0]     thread_read,
    // input   logic [4:0]     thread_write,

);

logic [31:0] op1_int, op2_int;
logic [31:0] op1_float, op2_float;

// What we need in each lane:
// Registers
// Regular ALU
// Floating Point ALU

registerfile threading_registers(
    .clk(clk),
    .WE3(WE3),            // Write enable
    .AD1(AD1),            // Read register 1 address
    .AD2(AD2),            // Read register 2 address
    .AD3(AD3),            // Write register address
    .WD3(),            // Write data
    .thread_read(threads),
    .thread_write(),
    .bIdx(bIdx)
);

ALU ALU(
    .ALUop1(op1_int),    
    .ALUop2(op2_int),    
    .ALUctrl(FUNCT4)   
);

floating_alu floating_alu(
    .alu_op(FUNCT4),
    .op1(op1_float),
    .op2(op2_float)
);

// Pipeline also needs to be implemented locally

endmodule 

