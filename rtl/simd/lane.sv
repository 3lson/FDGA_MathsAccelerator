module lane (

    // verilator lint_off UNSIGNED
    input   logic         clk,
    input   logic         stall,
    input   logic [3:0]   tIdx,
    input   logic [31:0]  bIdx,
    input   logic [4:0]   FUNCT4,
    input   logic [31:0]  IMM,
    input   logic [4:0]   AD1,
    input   logic [4:0]   AD2,
    input   logic [4:0]   AD3,
    input   logic         is_int,
    input   logic         is_float,
    input   logic         WE3,
    // ID Stage Inputs from RF
    input   logic [31:0] RD1,
    input   logic [31:0] RD2,    
  
    // --- Interface to an EXTERNAL Register File ---
    // ID Stage Outputs to RF (for reading)
    output  logic [4:0]  AD1_out,
    output  logic [4:0]  AD2_out,
    output  logic [3:0]  tIdx_out,
    

    output  logic [4:0]  AD3_out,
    output  logic [31:0] WD3_out,
    output  logic        WE3_out,
    output  logic [3:0]  tIdx_out, // To select thread in RF for writing

    // Stall signal output by this lane to the instruction issuer
    output  logic        stall_out

);

// Temporary structs for pipeline, can be changed later if necessary

// ID -> EX Register
typedef struct packed {
    logic   [31:0]  op1_id;
    logic   [31:0]  op2_id;      // Will hold either RF data or IMM
    logic   [4:0]   RD_id;       // Destination register address
    logic   [4:0]   FUNCT4_id;
    logic           is_int_id;
    logic           is_float_id;
    logic           WE3_id; // Original WE3 for the instruction
    logic   [3:0]   tIdx_id; // Pipelined thread ID
} id_ex_pipe;

id_ex_pipe id_ex_data_in, id_ex_data_out

// EX -> MEM Register (MEM is mostly a pass-through here)
typedef struct packed {
    logic   [31:0]  result_ex; // Result from ALU or FPU
    logic   [4:0]   RD_ex;
    logic           WE3_ex;
    logic   [3:0]   tIdx_ex;
} ex_mem_pipe;

ex_mem_pipe ex_mem_data_in, ex_mem_data_out;

// MEM -> WB Register
typedef struct packed {
    logic   [31:0]  data_to_writeback;
    logic   [4:0]   RD_mem;
    logic           WE3_mem;
    logic   [3:0]   tIdx_mem;
} mem_wb_pipe;

mem_wb_pipe mem_wb_data_in, mem_wb_data_out;

// What we need in each lane:
// Regular ALU
// Floating Point ALU

/*
    Is any of this necessary??
    // --- Stall Logic ---
    // Lane stalls if:
    // 1. EX stage needs to stall (e.g., multi-cycle FPU not ready - ex_internal_stall)
    // 2. MEM stage needs to stall (e.g., waiting for memory - mem_internal_stall)
    // 3. WB stage is stalled by external RF (stall_from_wb)
    logic ex_internal_stall; // Assume 0 for now (ALU/FPU are 1 cycle)
    logic mem_internal_stall; // Assume 0 for now (no memory ops)

    assign ex_internal_stall = 1'b0;
    assign mem_internal_stall = 1'b0;

    logic stall_ex_stage, stall_mem_stage;
    assign stall_mem_stage   = mem_internal_stall || stall_from_wb;
    assign stall_ex_stage    = ex_internal_stall  || stall_mem_stage;
    assign lane_is_stalled_o = stall_ex_stage; // ID stage stalls if EX stalls


    // --- Stage 1: ID (Instruction Decode / Operand Fetch) ---
    // Outputs to external Register File for reading operands
    assign rf_AD1_read_o      = instr_AD1_rs1;
    assign rf_AD2_read_o      = instr_AD2_rs2;
    assign rf_thread_read_o   = instr_threads_id;

    // Logic to prepare data for id_ex_data register
    always_comb begin
        id_ex_data_next.operand1          = rf_RD1_data_i; // Data from RF
        id_ex_data_next.operand2          = instr_is_imm_op ? instr_IMM : rf_RD2_data_i;
        id_ex_data_next.rd_addr           = instr_AD3_rd;
        id_ex_data_next.funct4_ctrl       = instr_FUNCT4;
        id_ex_data_next.is_int_op         = instr_is_int;
        id_ex_data_next.is_float_op       = instr_is_float;
        id_ex_data_next.write_enable_instr= instr_WE3;
        id_ex_data_next.thread_id_pipe    = instr_threads_id;

        if (enable_new_instr && !lane_is_stalled_o) begin
            id_ex_data_next.valid = 1'b1;
        end else if (lane_is_stalled_o) begin
            id_ex_data_next = id_ex_data; // Hold previous data
        end else begin // No new instruction, and not stalled (so bubble)
            id_ex_data_next.valid = 1'b0;
        end
    end
*/

logic           int_eq, float_eq;
logic   [31:0]  alu_result, floatingalu_result;

ALU ALU(
    .ALUop1(id_ex_data_in.op1_id),    
    .ALUop2(id_ex_data_in.op2_id),    
    .ALUctrl(id_ex_data_in.FUNCT4_id),
    .Result(alu_result), 
    .EQ(int_eq)       
);

floating_alu floating_alu(
    .alu_op(id_ex_data_in.FUNCT4_id),
    .op1(id_ex_data_in.op1_id),
    .op2(id_ex_data_in.op2_id),
    .result(floatingalu_result),
    .cmp(float_eq)
);

// Logic to prepare data for ex_mem_data register
always_comb begin
    if (id_ex_data_out.is_int_id) begin
        ex_mem_data_in.result_ex = alu_result;
    end else if (id_ex_data_out.is_float_id) begin
        ex_mem_data_in.result_ex = floatingalu_result;
    end else begin
        ex_mem_data_in.result_ex = 32'hDEADBEEF; // Should be guarded by valid
    end
    ex_mem_data_in.RD_ex            = id_ex_data_out.RD_id;
    ex_mem_data_in.WE3_ex           = id_ex_data_out.WE3_id;
    ex_mem_data_in.tIdx_ex   = id_ex_data_out.tIdx_ex;

    if (stall) begin
        ex_mem_data_in = ex_mem_data; // Hold
    end
end

endmodule 

