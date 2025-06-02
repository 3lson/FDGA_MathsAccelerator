module lane_pipelined (
    input logic         clk,
    input logic         rst_n,

    // Decoded Instruction Inputs (from a Scheduler/Controller to the ID stage)
    input logic         enable_new_instr, // Signal that new instruction inputs are valid
    input logic         stall_from_wb,    // Stall signal from WB stage (e.g., RF not ready for write)

    input logic [3:0]   instr_threads_id, // Current thread_id for this instruction
    input logic [31:0]  instr_bIdx,       // bIdx for this instruction (passed to RF if needed by RF logic)
    input logic [4:0]   instr_FUNCT4,
    input logic [31:0]  instr_IMM,
    input logic [4:0]   instr_AD1_rs1,
    input logic [4:0]   instr_AD2_rs2,
    input logic [4:0]   instr_AD3_rd,
    input logic         instr_is_int,
    input logic         instr_is_float,
    input logic         instr_is_imm_op, // Added: Is the second operand an immediate?
    input logic         instr_WE3,       // Original write enable for this instruction

    // --- Interface to an EXTERNAL Register File ---
    // ID Stage Outputs to RF (for reading)
    output logic [4:0]  rf_AD1_read_o,
    output logic [4:0]  rf_AD2_read_o,
    output logic [3:0]  rf_thread_read_o, // To select thread in RF for reading
    // ID Stage Inputs from RF
    input  logic [31:0] rf_RD1_data_i,
    input  logic [31:0] rf_RD2_data_i,

    // WB Stage Outputs to RF (for writing)
    output logic [4:0]  rf_AD3_write_o,
    output logic [31:0] rf_WD3_write_o,
    output logic        rf_WE3_write_o,
    output logic [3:0]  rf_thread_write_o, // To select thread in RF for writing

    // Stall signal output by this lane to the instruction issuer
    output logic        lane_is_stalled_o
);

    // --- Pipeline Register Definitions ---
    // ID -> EX Register
    typedef struct packed {
        logic [31:0] operand1;
        logic [31:0] operand2;      // Will hold either RF data or IMM
        logic [4:0]  rd_addr;       // Destination register address
        logic [4:0]  funct4_ctrl;
        logic        is_int_op;
        logic        is_float_op;
        logic        write_enable_instr; // Original WE3 for the instruction
        logic [3:0]  thread_id_pipe; // Pipelined thread ID
        logic        valid;           // Is data in this register valid?
    } id_ex_reg_t;
    id_ex_reg_t id_ex_data, id_ex_data_next;

    // EX -> MEM Register (MEM is mostly a pass-through here)
    typedef struct packed {
        logic [31:0] execute_result; // Result from ALU or FPU
        logic [4:0]  rd_addr_pipe;
        logic        write_enable_pipe;
        logic [3:0]  thread_id_pipe;
        logic        valid;
    } ex_mem_reg_t;
    ex_mem_reg_t ex_mem_data, ex_mem_data_next;

    // MEM -> WB Register
    typedef struct packed {
        logic [31:0] data_to_writeback;
        logic [4:0]  rd_addr_pipe;
        logic        write_enable_pipe;
        logic [3:0]  thread_id_pipe;
        logic        valid;
    } mem_wb_reg_t;
    mem_wb_reg_t mem_wb_data, mem_wb_data_next;


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


    // --- Stage 2: EX (Execute) ---
    logic [31:0] alu_ex_result;
    logic        int_eq_ex;
    logic [31:0] fpu_ex_result;
    logic        float_eq_ex;

    // Instantiate ALUs (these operate on data from id_ex_data register)
    alu ALU_inst (
        .ALUop1(id_ex_data.operand1),
        .ALUop2(id_ex_data.operand2),
        .ALUctrl(id_ex_data.funct4_ctrl),
        .Result(alu_ex_result),
        .EQ(int_eq_ex)
    );

    floating_alu floating_alu_inst (
        .alu_op(id_ex_data.funct4_ctrl),
        .op1(id_ex_data.operand1),
        .op2(id_ex_data.operand2),
        .result(fpu_ex_result),
        .cmp(float_eq_ex)
    );

    // Logic to prepare data for ex_mem_data register
    always_comb begin
        if (id_ex_data.is_int_op) begin
            ex_mem_data_next.execute_result = alu_ex_result;
        end else if (id_ex_data.is_float_op) begin
            ex_mem_data_next.execute_result = fpu_ex_result;
        end else begin
            ex_mem_data_next.execute_result = 32'hDEADBEEF; // Should be guarded by valid
        end
        ex_mem_data_next.rd_addr_pipe      = id_ex_data.rd_addr;
        ex_mem_data_next.write_enable_pipe = id_ex_data.write_enable_instr;
        ex_mem_data_next.thread_id_pipe    = id_ex_data.thread_id_pipe;

        if (id_ex_data.valid && !stall_ex_stage) begin
            ex_mem_data_next.valid = id_ex_data.valid;
        end else if (stall_ex_stage) begin
            ex_mem_data_next = ex_mem_data; // Hold
        end else begin // Bubble from ID
            ex_mem_data_next.valid = 1'b0;
        end
    end


    // --- Stage 3: MEM (Memory Access) ---
    // Pass-through for this design, as no memory ops are defined in the lane's inputs
    // Logic to prepare data for mem_wb_data register
    always_comb begin
        mem_wb_data_next.data_to_writeback = ex_mem_data.execute_result;
        mem_wb_data_next.rd_addr_pipe      = ex_mem_data.rd_addr_pipe;
        mem_wb_data_next.write_enable_pipe = ex_mem_data.write_enable_pipe;
        mem_wb_data_next.thread_id_pipe    = ex_mem_data.thread_id_pipe;

        if (ex_mem_data.valid && !stall_mem_stage) begin
            mem_wb_data_next.valid = ex_mem_data.valid;
        end else if (stall_mem_stage) begin
            mem_wb_data_next = mem_wb_data; // Hold
        end else begin // Bubble from EX
            mem_wb_data_next.valid = 1'b0;
        end
    end


    // --- Stage 4: WB (Write Back) ---
    // Outputs to external Register File for writing the result
    assign rf_AD3_write_o    = mem_wb_data.rd_addr_pipe;
    assign rf_WD3_write_o    = mem_wb_data.data_to_writeback;
    assign rf_WE3_write_o    = mem_wb_data.write_enable_pipe && mem_wb_data.valid && !stall_from_wb;
    assign rf_thread_write_o = mem_wb_data.thread_id_pipe;


    // --- Pipeline Register Sequential Updates ---
    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            id_ex_data.valid  <= 1'b0;
            ex_mem_data.valid <= 1'b0;
            mem_wb_data.valid <= 1'b0;
        end else begin
            // ID -> EX register
            if (!lane_is_stalled_o) begin // Stall signal for ID/EX reg is lane_is_stalled_o
                id_ex_data <= id_ex_data_next;
            end
            // else id_ex_data holds if lane_is_stalled_o is true

            // EX -> MEM register
            if (!stall_ex_stage) begin // Stall signal for EX/MEM reg
                ex_mem_data <= ex_mem_data_next;
            end
            // else ex_mem_data holds

            // MEM -> WB register
            if (!stall_mem_stage) begin // Stall signal for MEM/WB reg
                mem_wb_data <= mem_wb_data_next;
            end
            // else mem_wb_data holds
        end
    end

endmodule
