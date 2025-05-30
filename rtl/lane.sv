module lane (

    // verilator lint_off UNSIGNED
    input logic         clk,
    input logic         stall,
    input logic [3:0]   threads,
    input logic [31:0]  bIdx,
    input logic [3:0]   FUNCT4,
    input logic [31:0]  IMM,
    input logic [4:0]   AD1,
    input logic [4:0]   AD2,
    input logic [4:0]   AD3,

    // types?
    input logic is_int,
    // input logic is_imm,
    input logic is_float,
    
    input logic         WE3    
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

assign op1_int   = RD1[0];
assign op2_int   = RD2[0];
assign op1_float = RD1[0];
assign op2_float = RD2[0];

logic [31:0] RD1 [3:0], RD2 [3:0];
logic float_eq_shared, int_eq_shared;
logic [31:0] alu_result, floatingalu_result;
logic [31:0] final_result;


// What we need in each lane:
// Registers
// Regular ALU
// Floating Point ALU

threading_regfile threading_reg_file(
    .clk(clk),
    .WE3(WE3),              // Write enable
    .AD1(AD1),              // Read register 1 address
    .AD2(AD2),              // Read register 2 address
    .AD3(AD3),              // Write register address
    .WD3(final_result),                 // Write data
    .thread_read(threads),
    .thread_write(threads),
    .bIdx(bIdx),
    .RD1(RD1[thread_index]),              // Read data 1
    .RD2(RD2[thread_index])
);

alu ALU(
    .ALUop1(op1_int),    
    .ALUop2(op2_int),    
    .ALUctrl(FUNCT4),
    .Result(alu_result), 
    .EQ(int_eq_shared)       
);

floating_alu floating_alu(
    .alu_op(FUNCT4),
    .op1(op1_float),
    .op2(op2_float),
    .result(floatingalu_result),
    .cmp(float_eq_shared)
);

// Pipeline also needs to be implemented locally

always_comb begin
    if (is_int)
        final_result = alu_result;
    else if (is_float)
        final_result = floatingalu_result;
    else
        final_result = 32'hDEADBEEF; // invalid instruction
end

endmodule 

