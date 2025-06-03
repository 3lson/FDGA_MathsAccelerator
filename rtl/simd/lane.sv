module lane #(
    parameter THREAD_IDX = 0  // Default thread index for this lane
)(
    input   logic         clk,
    input   logic         reset,
    input   logic         stall,          // Global stall signal
    input   logic [3:0]   tIdx,           // Thread index (for multithreading)
    input   logic [31:0]  bIdx,           // Block index (for CUDA-like semantics)
    input   logic [4:0]   FUNCT4,         // Operation type
    input   logic [31:0]  IMM,            // Immediate value
    input   logic [4:0]   rs1,            // Source register 1
    input   logic [4:0]   rs2,            // Source register 2
    input   logic [4:0]   rd,             // Destination register
    input   logic         is_int,         // Integer operation
    input   logic         is_float,       // Floating-point operation
    input   logic         WE3,            // Write enable
    input   logic         pred_en,        // Predicate enable
    input   logic         pred_read,      // Predicate read value
    
    // Pipeline control
    output  logic         stall_out,      // Lane stall signal
    output  logic         pred_write      // Predicate write output
);

logic [31:0] RD1, RD2;  // Register read outputs

threadingregfile #(
    .thread_idx(THREAD_IDX)
) regfile (
    .clk(clk),
    .WE3(WE3 && !stall),  // Only write when not stalled
    .AD1(rs1),
    .AD2(rs2),
    .AD3(rd),
    .WD3(result_wb),      // From WB stage
    .thread_read(tIdx),
    .thread_write(tIdx),
    .bIdx(bIdx),
    .RD1(RD1),
    .RD2(RD2),
    .pred_en(pred_en),
    .pred_read(pred_read),
    .pred_write(pred_write)
);

typedef struct packed {
    logic [31:0]  op1, op2;
    logic [4:0]   rd;
    logic [4:0]   FUNCT4;
    logic         is_int, is_float;
    logic         WE3;
    logic [3:0]   tIdx;
} id_ex_reg;

typedef struct packed {
    logic [31:0]  result;
    logic [4:0]   rd;
    logic         WE3;
    logic [3:0]   tIdx;
} ex_mem_reg;

typedef struct packed {
    logic [31:0]  result;
    logic [4:0]   rd;
    logic         WE3;
    logic [3:0]   tIdx;
} mem_wb_reg;

id_ex_reg  id_ex, id_ex_next;
ex_mem_reg ex_mem, ex_mem_next;
mem_wb_reg mem_wb, mem_wb_next;

logic [31:0] alu_result, fpu_result;
logic alu_eq, fpu_eq;

ALU alu (
    .ALUop1(id_ex.op1),
    .ALUop2(id_ex.op2),
    .ALUctrl(id_ex.FUNCT4),
    .Result(alu_result),
    .EQ(alu_eq)
);

floating_alu fpu (
    .alu_op(id_ex.FUNCT4),
    .op1(id_ex.op1),
    .op2(id_ex.op2),
    .result(fpu_result),
    .cmp(fpu_eq)
);


// --- ID Stage ---
always_comb begin
    // Operands come directly from the register file
    id_ex_next.op1 = RD1;
    id_ex_next.op2 = (FUNCT4[4] ? IMM : RD2); // Select IMM or RF data
    id_ex_next.rd = rd;
    id_ex_next.FUNCT4 = FUNCT4;
    id_ex_next.is_int = is_int;
    id_ex_next.is_float = is_float;
    id_ex_next.WE3 = WE3;
    id_ex_next.tIdx = tIdx;
    
    stall_out = stall; // Can add lane-specific stall conditions
end

// --- EX Stage ---
always_comb begin
    // Select appropriate execution unit result
    if (id_ex.is_int) begin
        ex_mem_next.result = alu_result;
    end else if (id_ex.is_float) begin
        ex_mem_next.result = fpu_result;
    end else begin
        ex_mem_next.result = 32'h0; // Default
    end
    
    // Pass through other signals
    ex_mem_next.rd = id_ex.rd;
    ex_mem_next.WE3 = id_ex.WE3;
    ex_mem_next.tIdx = id_ex.tIdx;
end

// --- MEM Stage (Pass-through) ---
always_comb begin
    mem_wb_next = ex_mem; // No memory ops in current design
end

// --- WB Stage ---
logic [31:0] result_wb;
always_comb begin
    result_wb = mem_wb.result; // This goes back to regfile
end

// =============================================
// Pipeline Register Updates
// =============================================
always_ff @(posedge clk or posedge reset) begin
    if (reset) begin
        id_ex <= '0;
        ex_mem <= '0;
        mem_wb <= '0;
    end else if (!stall) begin
        id_ex <= id_ex_next;
        ex_mem <= ex_mem_next;
        mem_wb <= mem_wb_next;
    end
    // On stall, pipeline registers maintain their values
end

endmodule
