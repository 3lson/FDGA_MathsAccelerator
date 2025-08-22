`timescale 1ns/1ns

// Pipelined compute core with 5-stage pipeline:
// 1. FETCH - Instruction fetch
// 2. DECODE - Instruction decode and register read
// 3. EXECUTE - ALU/LSU operation
// 4. MEMORY - Memory access completion
// 5. WRITEBACK - Register writeback

`include "common.svh"

module compute_core#(
    parameter int THREADS_PER_WARP = 16          // Number of threads per warp (max 32)
    )(
    input   wire                            clk,
    input   wire                            reset,

    input   logic                           start,
    output  logic                           done,

    input   data_t                          block_id,
    input   kernel_config_t                 kernel_config,

    // Instruction Memory
    input   logic       instruction_mem_read_ready,
    input   instruction_t                   instruction_mem_read_data       ,
    output  logic       instruction_mem_read_valid,
    output  instruction_memory_address_t    instruction_mem_read_address    ,

    // Data Memory
    output  logic   [NUM_LSUS-1:0]          data_mem_read_valid,
    output  data_memory_address_t           data_mem_read_address           [NUM_LSUS],
    input   logic   [NUM_LSUS-1:0]          data_mem_read_ready,
    input   data_t                          data_mem_read_data              [NUM_LSUS],
    output  logic   [NUM_LSUS-1:0]          data_mem_write_valid,
    output  data_memory_address_t           data_mem_write_address          [NUM_LSUS],
    output  data_t                          data_mem_write_data             [NUM_LSUS],
    input   logic   [NUM_LSUS-1:0]          data_mem_write_ready
);

typedef logic [THREADS_PER_WARP-1:0] warp_mask_t;
localparam int NUM_LSUS = THREADS_PER_WARP + 1;

// Pipeline stage valid signals
logic fetch_valid, decode_valid, execute_valid, memory_valid, writeback_valid;
logic fetch_ready, decode_ready, execute_ready, memory_ready, writeback_ready;
logic fetch_stall, decode_stall, execute_stall, memory_stall;

// ========== FETCH STAGE ==========
typedef struct packed {
    instruction_memory_address_t pc;
    logic warp_id;
    warp_mask_t execution_mask;
} fetch_stage_t;

fetch_stage_t fetch_stage_reg, fetch_stage_next;
instruction_t fetched_instruction;
fetcher_state_t fetcher_state;

// ========== DECODE STAGE ==========
typedef struct packed {
    instruction_t instruction;
    instruction_memory_address_t pc;
    logic warp_id;
    warp_mask_t execution_mask;
} decode_stage_t;

decode_stage_t decode_stage_reg, decode_stage_next;

// Decoded instruction fields
logic decoded_reg_write_enable;
reg_input_mux_t decoded_reg_input_mux;
data_t decoded_immediate;
logic decoded_branch;
logic decoded_scalar_instruction;
logic [4:0] decoded_rd_address;
logic [4:0] decoded_rs1_address;
logic [4:0] decoded_rs2_address;
logic [4:0] decoded_alu_instruction;
logic decoded_halt;
logic [1:0] floatingRead_flag;
logic floatingWrite_flag;
logic decoded_mem_read_enable_per_warp;
logic decoded_mem_write_enable_per_warp;

// Register file outputs
data_t scalar_int_rs1, scalar_int_rs2;
data_t scalar_float_rs1, scalar_float_rs2;
data_t vector_int_rs1 [THREADS_PER_WARP];
data_t vector_int_rs2 [THREADS_PER_WARP];
data_t vector_float_rs1 [THREADS_PER_WARP];
data_t vector_float_rs2 [THREADS_PER_WARP];

// ========== EXECUTE STAGE ==========
typedef struct packed {
    instruction_t instruction;
    instruction_memory_address_t pc;
    logic warp_id;
    warp_mask_t execution_mask;
    logic reg_write_enable;
    reg_input_mux_t reg_input_mux;
    data_t immediate;
    logic branch;
    logic scalar_instruction;
    logic [4:0] rd_address;
    logic [4:0] rs1_address;
    logic [4:0] rs2_address;
    logic [4:0] alu_instruction;
    logic halt;
    logic [1:0] floatingRead_flag;
    logic floatingWrite_flag;
    logic mem_read_enable;
    logic mem_write_enable;
} execute_stage_t;

execute_stage_t execute_stage_reg, execute_stage_next;

// Execute stage operands and results
data_t final_op1 [THREADS_PER_WARP];
data_t final_op2 [THREADS_PER_WARP];
data_t scalar_op1, scalar_op2;
data_t alu_out [THREADS_PER_WARP];
data_t scalar_alu_out;

// ========== MEMORY STAGE ==========
typedef struct packed {
    instruction_t instruction;
    instruction_memory_address_t pc;
    logic warp_id;
    warp_mask_t execution_mask;
    logic reg_write_enable;
    reg_input_mux_t reg_input_mux;
    data_t immediate;
    logic branch;
    logic scalar_instruction;
    logic [4:0] rd_address;
    logic halt;
    logic floatingWrite_flag;
    data_t scalar_alu_result;
} memory_stage_t;

memory_stage_t memory_stage_reg, memory_stage_next;
data_t alu_results_reg [THREADS_PER_WARP], alu_results_next [THREADS_PER_WARP];

data_t lsu_out [THREADS_PER_WARP];
lsu_state_t lsu_state [THREADS_PER_WARP];
data_t scalar_lsu_out;
lsu_state_t scalar_lsu_state;

// ========== WRITEBACK STAGE ==========
typedef struct packed {
    instruction_t instruction;
    instruction_memory_address_t pc;
    logic warp_id;
    warp_mask_t execution_mask;
    logic reg_write_enable;
    reg_input_mux_t reg_input_mux;
    data_t immediate;
    logic branch;
    logic scalar_instruction;
    logic [4:0] rd_address;
    logic halt;
    logic floatingWrite_flag;
    data_t scalar_final_result;
} writeback_stage_t;

data_t final_results_reg [THREADS_PER_WARP];
data_t final_results_next [THREADS_PER_WARP];

writeback_stage_t writeback_stage_reg, writeback_stage_next;

// Pipeline control and hazard detection
logic branch_taken;
instruction_memory_address_t branch_target;
data_t vector_to_scalar_data;

// Warp scheduling and control
logic current_warp;
warp_state_t warp_state;
data_t num_warps;
logic start_execution;
logic all_warps_done;
instruction_memory_address_t pc, next_pc;

assign num_warps = kernel_config.num_warps_per_block;

// ========== PIPELINE CONTROL LOGIC ==========

// Stall conditions
always_comb begin
    // Memory stage stalls if LSUs are still processing
    memory_stall = 1'b0;
    for (int i = 0; i < THREADS_PER_WARP; i++) begin
        if (lsu_state[i] == LSU_REQUESTING || lsu_state[i] == LSU_WAITING) begin
            memory_stall = 1'b1;
            break;
        end
    end
    if (scalar_lsu_state == LSU_REQUESTING || scalar_lsu_state == LSU_WAITING) begin
        memory_stall = 1'b1;
    end
    
    // Execute stage stalls if memory stage is stalled
    execute_stall = memory_stall && memory_valid;
    
    // Decode stage stalls if execute stage is stalled
    decode_stall = execute_stall && execute_valid;
    
    // Fetch stage stalls if decode stage is stalled
    fetch_stall = decode_stall && decode_valid;
end

// Ready signals (inverse of stall)
assign fetch_ready = !fetch_stall;
assign decode_ready = !decode_stall;
assign execute_ready = !execute_stall;
assign memory_ready = !memory_stall;
assign writeback_ready = 1'b1; // Writeback is always ready

// ========== FETCH STAGE LOGIC ==========
always_comb begin
    fetch_stage_next = fetch_stage_reg;
    
    if (fetch_ready && warp_state == WARP_FETCH) begin
        fetch_stage_next.pc = pc;
        fetch_stage_next.warp_id = current_warp;
        // Set execution mask based on warp scheduling logic
        fetch_stage_next.execution_mask = {THREADS_PER_WARP{1'b1}}; // Simplified for now
    end
end

// Instruction fetcher
fetcher fetcher_inst(
    .clk(clk),
    .reset(reset),
    .warp_state(warp_state),
    .pc(fetch_stage_reg.pc),
    .instruction_mem_read_ready(instruction_mem_read_ready),
    .instruction_mem_read_data(instruction_mem_read_data),
    .instruction_mem_read_valid(instruction_mem_read_valid),
    .instruction_mem_read_address(instruction_mem_read_address),
    .fetcher_state(fetcher_state),
    .instruction(fetched_instruction)
);

// ========== DECODE STAGE LOGIC ==========
always_comb begin
    decode_stage_next = decode_stage_reg;
    
    if (decode_ready && fetch_valid && fetcher_state == FETCHER_DONE) begin
        decode_stage_next.instruction = fetched_instruction;
        decode_stage_next.pc = fetch_stage_reg.pc;
        decode_stage_next.warp_id = fetch_stage_reg.warp_id;
        decode_stage_next.execution_mask = fetch_stage_reg.execution_mask;
    end
end

// Instruction decoder
decoder decoder_inst(
    .clk(clk),
    .reset(reset),
    .warp_state(decode_valid ? WARP_DECODE : WARP_IDLE),
    .instruction(decode_stage_reg.instruction),
    .decoded_reg_write_enable(decoded_reg_write_enable),
    .decoded_mem_write_enable(decoded_mem_write_enable_per_warp),
    .decoded_mem_read_enable(decoded_mem_read_enable_per_warp),
    .decoded_branch(decoded_branch),
    .decoded_scalar_instruction(decoded_scalar_instruction),
    .decoded_reg_input_mux(decoded_reg_input_mux),
    .decoded_immediate(decoded_immediate),
    .decoded_rd_address(decoded_rd_address),
    .decoded_rs1_address(decoded_rs1_address),
    .decoded_rs2_address(decoded_rs2_address),
    .decoded_alu_instruction(decoded_alu_instruction),
    .decoded_halt(decoded_halt),
    .floatingRead(floatingRead_flag),
    .floatingWrite(floatingWrite_flag)
);

// ========== EXECUTE STAGE LOGIC ==========
always_comb begin
    execute_stage_next = execute_stage_reg;
    
    if (execute_ready && decode_valid) begin
        execute_stage_next.instruction = decode_stage_reg.instruction;
        execute_stage_next.pc = decode_stage_reg.pc;
        execute_stage_next.warp_id = decode_stage_reg.warp_id;
        execute_stage_next.execution_mask = decode_stage_reg.execution_mask;
        execute_stage_next.reg_write_enable = decoded_reg_write_enable;
        execute_stage_next.reg_input_mux = decoded_reg_input_mux;
        execute_stage_next.immediate = decoded_immediate;
        execute_stage_next.branch = decoded_branch;
        execute_stage_next.scalar_instruction = decoded_scalar_instruction;
        execute_stage_next.rd_address = decoded_rd_address;
        execute_stage_next.rs1_address = decoded_rs1_address;
        execute_stage_next.rs2_address = decoded_rs2_address;
        execute_stage_next.alu_instruction = decoded_alu_instruction;
        execute_stage_next.halt = decoded_halt;
        execute_stage_next.floatingRead_flag = floatingRead_flag;
        execute_stage_next.floatingWrite_flag = floatingWrite_flag;
        execute_stage_next.mem_read_enable = decoded_mem_read_enable_per_warp;
        execute_stage_next.mem_write_enable = decoded_mem_write_enable_per_warp;
    end
end

// Operand muxing (same as original but using execute stage signals)
always_comb begin
    case(execute_stage_reg.floatingRead_flag)
        2'b00: {scalar_op1, scalar_op2} = {scalar_int_rs1, scalar_int_rs2};
        2'b01: {scalar_op1, scalar_op2} = {scalar_float_rs1, scalar_int_rs2};
        2'b10: {scalar_op1, scalar_op2} = {scalar_int_rs1, scalar_float_rs2};
        2'b11: {scalar_op1, scalar_op2} = {scalar_float_rs1, scalar_float_rs2};
        default: {scalar_op1, scalar_op2} = {scalar_int_rs1, scalar_int_rs2};
    endcase
end

generate
for (genvar i = 0; i < THREADS_PER_WARP; i++) begin: g_operand_mux
    logic [31:0] vector_op1, vector_op2;
    
    always_comb begin
        case(execute_stage_reg.floatingRead_flag)
            2'b00: {vector_op1, vector_op2} = {vector_int_rs1[i],   vector_int_rs2[i]};
            2'b01: {vector_op1, vector_op2} = {vector_float_rs1[i], vector_int_rs2[i]};
            2'b10: {vector_op1, vector_op2} = {vector_int_rs1[i],   vector_float_rs2[i]};
            2'b11: {vector_op1, vector_op2} = {vector_float_rs1[i], vector_float_rs2[i]};
            default: {vector_op1, vector_op2} = {vector_int_rs1[i], vector_int_rs2[i]};
        endcase
    end
    
    assign final_op1[i] = execute_stage_reg.scalar_instruction ? scalar_op1 : vector_op1;
    assign final_op2[i] = execute_stage_reg.scalar_instruction ? scalar_op2 : vector_op2;
end
endgenerate

// ========== MEMORY STAGE LOGIC ==========
always_comb begin
    memory_stage_next = memory_stage_reg;
    alu_results_next = alu_results_reg;
    
    if (memory_ready && execute_valid) begin
        memory_stage_next.instruction = execute_stage_reg.instruction;
        memory_stage_next.pc = execute_stage_reg.pc;
        memory_stage_next.warp_id = execute_stage_reg.warp_id;
        memory_stage_next.execution_mask = execute_stage_reg.execution_mask;
        memory_stage_next.reg_write_enable = execute_stage_reg.reg_write_enable;
        memory_stage_next.reg_input_mux = execute_stage_reg.reg_input_mux;
        memory_stage_next.immediate = execute_stage_reg.immediate;
        memory_stage_next.branch = execute_stage_reg.branch;
        memory_stage_next.scalar_instruction = execute_stage_reg.scalar_instruction;
        memory_stage_next.rd_address = execute_stage_reg.rd_address;
        memory_stage_next.halt = execute_stage_reg.halt;
        memory_stage_next.floatingWrite_flag = execute_stage_reg.floatingWrite_flag;
        
        // Pass ALU results
        alu_results_next = alu_out;
        memory_stage_next.scalar_alu_result = scalar_alu_out;
    end
end

// ========== WRITEBACK STAGE LOGIC ==========
always_comb begin
    writeback_stage_next = writeback_stage_reg;
    final_results_next = final_results_reg;
    
    if (writeback_ready && memory_valid && !memory_stall) begin
        writeback_stage_next.instruction = memory_stage_reg.instruction;
        writeback_stage_next.pc = memory_stage_reg.pc;
        writeback_stage_next.warp_id = memory_stage_reg.warp_id;
        writeback_stage_next.execution_mask = memory_stage_reg.execution_mask;
        writeback_stage_next.reg_write_enable = memory_stage_reg.reg_write_enable;
        writeback_stage_next.reg_input_mux = memory_stage_reg.reg_input_mux;
        writeback_stage_next.immediate = memory_stage_reg.immediate;
        writeback_stage_next.branch = memory_stage_reg.branch;
        writeback_stage_next.scalar_instruction = memory_stage_reg.scalar_instruction;
        writeback_stage_next.rd_address = memory_stage_reg.rd_address;
        writeback_stage_next.halt = memory_stage_reg.halt;
        writeback_stage_next.floatingWrite_flag = memory_stage_reg.floatingWrite_flag;
        
        // Combine ALU and LSU results
        for (int i = 0; i < THREADS_PER_WARP; i++) begin
            final_results_next[i] = (memory_stage_reg.reg_input_mux == LSU_OUT) ? 
                                                   lsu_out[i] : alu_results_reg[i];
        end
        writeback_stage_next.scalar_final_result = (memory_stage_reg.reg_input_mux == LSU_OUT) ? 
                                                  scalar_lsu_out : memory_stage_reg.scalar_alu_result;
    end
end

// ========== PIPELINE REGISTER UPDATES ==========
always_ff @(posedge clk) begin
    if (reset) begin
        fetch_valid <= 1'b0;
        decode_valid <= 1'b0;
        execute_valid <= 1'b0;
        memory_valid <= 1'b0;
        writeback_valid <= 1'b0;
        
        fetch_stage_reg <= '0;
        decode_stage_reg <= '0;
        execute_stage_reg <= '0;
        memory_stage_reg <= '0;
        writeback_stage_reg <= '0;

        for (int i = 0; i < THREADS_PER_WARP; i = i + 1) begin
            final_results_reg[i] <= '0;
        end
                
        start_execution <= 1'b0;
        done <= 1'b0;
        warp_state <= WARP_IDLE;
        pc <= 0;
        next_pc <= 0;
        current_warp <= 0;
    end else begin
        // Update pipeline stages
        if (fetch_ready) begin
            fetch_stage_reg <= fetch_stage_next;
            fetch_valid <= (warp_state == WARP_FETCH);
        end
        
        if (decode_ready) begin
            decode_stage_reg <= decode_stage_next;
            decode_valid <= fetch_valid && (fetcher_state == FETCHER_DONE);
        end
        
        if (execute_ready) begin
            execute_stage_reg <= execute_stage_next;
            execute_valid <= decode_valid;
        end
        
        if (memory_ready) begin
            memory_stage_reg <= memory_stage_next;
            memory_valid <= execute_valid;
        end
        
        if (writeback_ready) begin
            writeback_stage_reg <= writeback_stage_next;
            final_results_reg <= final_results_next;
            writeback_valid <= memory_valid && !memory_stall;
        end
        
        // Warp scheduling and control (simplified)
        if (!start_execution) begin
            if (start) begin
                start_execution <= 1'b1;
                current_warp <= 0;
                if (num_warps > 0) begin
                    warp_state <= WARP_FETCH;
                    pc <= kernel_config.base_instructions_address;
                    next_pc <= kernel_config.base_instructions_address;
                end
            end
        end else begin
            // Handle branch and PC updates
            if (writeback_valid && writeback_stage_reg.branch) begin
                if (writeback_stage_reg.scalar_final_result == 1) begin
                    next_pc <= writeback_stage_reg.pc + writeback_stage_reg.immediate;
                end else begin
                    next_pc <= writeback_stage_reg.pc + 1;
                end
            end else if (writeback_valid && !writeback_stage_reg.halt) begin
                next_pc <= writeback_stage_reg.pc + 1;
            end
            
            // Update PC when fetch stage is ready
            if (fetch_ready && warp_state == WARP_FETCH) begin
                pc <= next_pc;
            end
            
            // Handle halt condition
            if (writeback_valid && writeback_stage_reg.halt) begin
                warp_state <= WARP_DONE;
                done <= 1'b1;
            end
        end
    end
end

// ========== FUNCTIONAL UNITS (same as original) ==========

// Scalar ALU
data_t scalar_int_alu_result, scalar_float_alu_result;
logic alu_enable, alu_valid;
assign alu_enable = execute_valid;
assign alu_valid = execute_valid;

alu scalar_alu_inst(
    .clk(clk),
    .rst(reset),
    .enable(alu_enable),
    .pc(execute_stage_reg.pc),
    .ALUop1(scalar_op1),
    .ALUop2(scalar_op2),
    .IMM(execute_stage_reg.immediate),
    .instruction(execute_stage_reg.alu_instruction),
    .Result(scalar_int_alu_result)
);

floating_alu scalar_fpu_inst(
    .clk(clk),
    .rst(reset),
    .enable(alu_enable),
    .valid(alu_valid),
    .op1(scalar_op1),
    .op2(scalar_op2),
    .instruction(execute_stage_reg.alu_instruction),
    .result(scalar_float_alu_result)
);

assign scalar_alu_out = (execute_stage_reg.alu_instruction >= FADD) ? 
                       scalar_float_alu_result : scalar_int_alu_result;

// Scalar LSU
lsu scalar_lsu_inst(
    .clk(clk),
    .reset(reset),
    .enable(execute_stage_reg.scalar_instruction),
    .warp_state(memory_valid ? WARP_WAIT : WARP_IDLE),
    .decoded_mem_read_enable(execute_stage_reg.mem_read_enable),
    .decoded_mem_write_enable(execute_stage_reg.mem_write_enable),
    .rs1(scalar_op1),
    .rs2(scalar_op2),
    .imm(execute_stage_reg.immediate),
    .mem_read_valid(data_mem_read_valid[THREADS_PER_WARP]),
    .mem_read_address(data_mem_read_address[THREADS_PER_WARP]),
    .mem_read_ready(data_mem_read_ready[THREADS_PER_WARP]),
    .mem_read_data(data_mem_read_data[THREADS_PER_WARP]),
    .mem_write_valid(data_mem_write_valid[THREADS_PER_WARP]),
    .mem_write_address(data_mem_write_address[THREADS_PER_WARP]),
    .mem_write_data(data_mem_write_data[THREADS_PER_WARP]),
    .mem_write_ready(data_mem_write_ready[THREADS_PER_WARP]),
    .lsu_state(scalar_lsu_state),
    .lsu_out(scalar_lsu_out)
);

// Vector functional units
generate
    for (genvar i = 0; i < THREADS_PER_WARP; i = i + 1) begin : g_vector_units
        wire t_enable = execute_stage_reg.execution_mask[i] && !execute_stage_reg.scalar_instruction && execute_valid;
       
        data_t vector_int_alu_result, vector_float_alu_result;

        alu vector_alu_inst(
            .clk(clk),
            .rst(reset),
            .enable(alu_enable),
            .pc(execute_stage_reg.pc),
            .ALUop1(final_op1[i]),
            .ALUop2(final_op2[i]),
            .IMM(execute_stage_reg.immediate),
            .instruction(execute_stage_reg.alu_instruction),
            .Result(vector_int_alu_result)
        );

        floating_alu vector_fpu_inst(
            .clk(clk),
            .rst(reset),
            .enable(alu_enable),
            .valid(alu_valid),
            .op1(final_op1[i]),
            .op2(final_op2[i]),
            .instruction(execute_stage_reg.alu_instruction),
            .result(vector_float_alu_result)
        );

        assign alu_out[i] = (execute_stage_reg.alu_instruction >= FADD) ? 
                           vector_float_alu_result : vector_int_alu_result;

        lsu lsu_inst(
            .clk(clk),
            .reset(reset),
            .enable(t_enable),
            .warp_state(memory_valid ? WARP_WAIT : WARP_IDLE),
            .decoded_mem_read_enable(execute_stage_reg.mem_read_enable),
            .decoded_mem_write_enable(execute_stage_reg.mem_write_enable),
            .rs1(final_op1[i]),
            .rs2(final_op2[i]),
            .imm(execute_stage_reg.immediate),
            .mem_read_valid(data_mem_read_valid[i]),
            .mem_read_address(data_mem_read_address[i]),
            .mem_read_ready(data_mem_read_ready[i]),
            .mem_read_data(data_mem_read_data[i]),
            .mem_write_valid(data_mem_write_valid[i]),
            .mem_write_address(data_mem_write_address[i]),
            .mem_write_data(data_mem_write_data[i]),
            .mem_write_ready(data_mem_write_ready[i]),
            .lsu_state(lsu_state[i]),
            .lsu_out(lsu_out[i])
        );
    end
endgenerate

// ========== REGISTER FILES (modified for pipeline) ==========

// Note: Register files need to use writeback stage signals for writes
// but decode stage signals for reads to avoid read-after-write hazards

// Scalar float register file
floating_scalar_reg_file #(
    .DATA_WIDTH(32)
) floating_scalar_reg_file_inst (
    .clk(clk),
    .reset(reset),
    .enable(1'b1), // Always enabled for pipeline
    .warp_state(writeback_valid ? WARP_UPDATE : WARP_IDLE),
    .decoded_reg_write_enable(writeback_valid && writeback_stage_reg.reg_write_enable && 
                             writeback_stage_reg.scalar_instruction && writeback_stage_reg.floatingWrite_flag),
    .decoded_reg_input_mux(writeback_stage_reg.reg_input_mux),
    .decoded_immediate(32'h0), // Not used in writeback
    .decoded_rd_address(writeback_stage_reg.rd_address),
    .decoded_rs1_address(decoded_rs1_address), // Use decode stage for reads
    .decoded_rs2_address(decoded_rs2_address), // Use decode stage for reads
    .alu_out(writeback_stage_reg.scalar_final_result),
    .lsu_out(writeback_stage_reg.scalar_final_result),
    .pc(writeback_stage_reg.pc),
    .vector_to_scalar_data(vector_to_scalar_data),
    .rs1(scalar_float_rs1),
    .rs2(scalar_float_rs2)
);

// Scalar integer register file
scalar_reg_file #(
    .DATA_WIDTH(32)
) scalar_reg_file_inst (
    .clk(clk),
    .reset(reset),
    .enable(1'b1), // Always enabled for pipeline
    .warp_state(writeback_valid ? WARP_UPDATE : WARP_IDLE),
    .decoded_reg_write_enable(writeback_valid && writeback_stage_reg.reg_write_enable && 
                             writeback_stage_reg.scalar_instruction && !writeback_stage_reg.floatingWrite_flag),
    .decoded_reg_input_mux(writeback_stage_reg.reg_input_mux),
    .decoded_immediate(32'h0), // Not used in writeback
    .decoded_rd_address(writeback_stage_reg.rd_address),
    .decoded_rs1_address(decoded_rs1_address), // Use decode stage for reads
    .decoded_rs2_address(decoded_rs2_address), // Use decode stage for reads
    .alu_out(writeback_stage_reg.scalar_final_result),
    .lsu_out(writeback_stage_reg.scalar_final_result),
    .pc(writeback_stage_reg.pc),
    .vector_to_scalar_data(vector_to_scalar_data),
    .rs1(scalar_int_rs1),
    .rs2(scalar_int_rs2)
);

// Vector integer register files
generate
    for (genvar i = 0; i < THREADS_PER_WARP; i = i + 1) begin : g_vector_int_regfiles
        reg_file #(
            .DATA_WIDTH(32)
        ) vector_reg_file_inst (
            .clk(clk),
            .reset(reset),
            .enable(1'b1), // Always enabled for pipeline
            .thread_enable(thread_enable),
            .warp_state(writeback_valid ? WARP_UPDATE : WARP_IDLE),
            .decoded_reg_write_enable(writeback_valid && writeback_stage_reg.reg_write_enable && 
                                     !writeback_stage_reg.scalar_instruction && !writeback_stage_reg.floatingWrite_flag &&
                                     writeback_stage_reg.execution_mask[i]),
            .decoded_reg_input_mux(writeback_stage_reg.reg_input_mux),
            .decoded_immediate(32'h0), // Not used in writeback
            .decoded_rd_address(writeback_stage_reg.rd_address),
            .decoded_rs1_address(decoded_rs1_address), // Use decode stage for reads
            .decoded_rs2_address(decoded_rs2_address), // Use decode stage for reads
            .alu_out(final_results_reg),
            .lsu_out(final_results_reg),
            .warp_id(0),
            // .pc(writeback_stage_reg.pc),
            // .thread_id(i[4:0]),
            .block_id(block_id),
            .rs1(vector_int_rs1),
            .rs2(vector_int_rs2)
        );
    end
endgenerate

// Vector floating-point register files
generate
    for (genvar i = 0; i < THREADS_PER_WARP; i = i + 1) begin : g_vector_float_regfiles
        reg_file #(
            .DATA_WIDTH(32)
        ) floating_vector_reg_file_inst (
            .clk(clk),
            .reset(reset),
            .enable(1'b1), // Always enabled for pipeline
            .thread_enable(thread_enable),
            .warp_state(writeback_valid ? WARP_UPDATE : WARP_IDLE),
            .decoded_reg_write_enable(writeback_valid && writeback_stage_reg.reg_write_enable && 
                                     !writeback_stage_reg.scalar_instruction && writeback_stage_reg.floatingWrite_flag &&
                                     writeback_stage_reg.execution_mask[i]),
            .decoded_reg_input_mux(writeback_stage_reg.reg_input_mux),
            .decoded_immediate(32'h0), // Not used in writeback
            .decoded_rd_address(writeback_stage_reg.rd_address),
            .decoded_rs1_address(decoded_rs1_address), // Use decode stage for reads
            .decoded_rs2_address(decoded_rs2_address), // Use decode stage for reads
            .alu_out(final_results_reg),
            .lsu_out(final_results_reg),
            // .pc(writeback_stage_reg.pc),
            // .thread_id(i[4:0]),
            .warp_id(0),
            .block_id(block_id),
            .rs1(vector_float_rs1),
            .rs2(vector_float_rs2)
        );
    end
endgenerate

// ========== HAZARD DETECTION AND FORWARDING ==========

// Data hazard detection logic
logic [2:0] hazard_rs1, hazard_rs2;
logic stall_for_hazard;

always_comb begin
    hazard_rs1 = 3'b000;
    hazard_rs2 = 3'b000;
    stall_for_hazard = 1'b0;
    
    // Check for RAW hazards between decode and later stages
    if (decode_valid) begin
        // Check against execute stage
        if (execute_valid && execute_stage_reg.reg_write_enable && 
            execute_stage_reg.rd_address != 0) begin
            if (decoded_rs1_address == execute_stage_reg.rd_address) begin
                hazard_rs1[0] = 1'b1;
            end
            if (decoded_rs2_address == execute_stage_reg.rd_address) begin
                hazard_rs2[0] = 1'b1;
            end
        end
        
        // Check against memory stage
        if (memory_valid && memory_stage_reg.reg_write_enable && 
            memory_stage_reg.rd_address != 0) begin
            if (decoded_rs1_address == memory_stage_reg.rd_address) begin
                hazard_rs1[1] = 1'b1;
            end
            if (decoded_rs2_address == memory_stage_reg.rd_address) begin
                hazard_rs2[1] = 1'b1;
            end
        end
        
        // Check against writeback stage
        if (writeback_valid && writeback_stage_reg.reg_write_enable && 
            writeback_stage_reg.rd_address != 0) begin
            if (decoded_rs1_address == writeback_stage_reg.rd_address) begin
                hazard_rs1[2] = 1'b1;
            end
            if (decoded_rs2_address == writeback_stage_reg.rd_address) begin
                hazard_rs2[2] = 1'b1;
            end
        end
        
        // For simplicity, stall on any load-use hazard
        // In a more sophisticated design, we could implement forwarding
        if ((hazard_rs1 != 3'b000) || (hazard_rs2 != 3'b000)) begin
            // Check if it's a load instruction that causes the hazard
            if ((execute_valid && execute_stage_reg.reg_input_mux == LSU_OUT) ||
                (memory_valid && memory_stage_reg.reg_input_mux == LSU_OUT && memory_stall)) begin
                stall_for_hazard = 1'b1;
            end
        end
    end
end

// ========== BRANCH PREDICTION AND CONTROL ==========

// Simple branch prediction (always not taken)
logic branch_prediction;
logic branch_mispredicted;
instruction_memory_address_t predicted_pc, actual_pc;

always_comb begin
    // Simple prediction: always not taken
    branch_prediction = 1'b0;
    predicted_pc = pc + 1;
    
    // Check for misprediction in writeback stage
    if (writeback_valid && writeback_stage_reg.branch) begin
        if (writeback_stage_reg.scalar_final_result == 1) begin
            actual_pc = writeback_stage_reg.pc + writeback_stage_reg.immediate;
        end else begin
            actual_pc = writeback_stage_reg.pc + 1;
        end
        
        branch_mispredicted = (actual_pc != (writeback_stage_reg.pc + 1));
    end else begin
        branch_mispredicted = 1'b0;
        actual_pc = predicted_pc;
    end
end

// Pipeline flush logic for branch misprediction
logic flush_pipeline;
assign flush_pipeline = branch_mispredicted;

// ========== ENHANCED PIPELINE CONTROL ==========

// Update stall conditions to include hazard detection
always_comb begin
    // Memory stage stalls if LSUs are still processing
    memory_stall = 1'b0;
    for (int i = 0; i < THREADS_PER_WARP; i++) begin
        if (lsu_state[i] == LSU_REQUESTING || lsu_state[i] == LSU_WAITING) begin
            memory_stall = 1'b1;
            break;
        end
    end
    if (scalar_lsu_state == LSU_REQUESTING || scalar_lsu_state == LSU_WAITING) begin
        memory_stall = 1'b1;
    end
    
    // Execute stage stalls if memory stage is stalled or hazard detected
    execute_stall = (memory_stall && memory_valid) || stall_for_hazard;
    
    // Decode stage stalls if execute stage is stalled
    decode_stall = execute_stall && execute_valid;
    
    // Fetch stage stalls if decode stage is stalled or instruction fetch not ready
    fetch_stall = (decode_stall && decode_valid) || 
                  (warp_state == WARP_FETCH && fetcher_state != FETCHER_DONE);
end

// ========== PERFORMANCE COUNTERS ==========

// Optional performance monitoring counters
logic [31:0] cycle_count;
logic [31:0] instruction_count;
logic [31:0] stall_count;
logic [31:0] branch_count;
logic [31:0] branch_mispredict_count;

always_ff @(posedge clk) begin
    if (reset) begin
        cycle_count <= 32'h0;
        instruction_count <= 32'h0;
        stall_count <= 32'h0;
        branch_count <= 32'h0;
        branch_mispredict_count <= 32'h0;
    end else if (start_execution && !done) begin
        cycle_count <= cycle_count + 1;
        
        if (writeback_valid) begin
            instruction_count <= instruction_count + 1;
        end
        
        if (fetch_stall || decode_stall || execute_stall || memory_stall) begin
            stall_count <= stall_count + 1;
        end
        
        if (writeback_valid && writeback_stage_reg.branch) begin
            branch_count <= branch_count + 1;
            if (branch_mispredicted) begin
                branch_mispredict_count <= branch_mispredict_count + 1;
            end
        end
    end
end

// ========== WARP SCHEDULER ENHANCEMENT ==========

// Enhanced warp scheduling for multiple warps
logic [31:0] warp_pc [2]; // Support up to 2 warps for simplicity
warp_state_t warp_states [2];
logic [1:0] active_warps;

always_ff @(posedge clk) begin
    if (reset) begin
        for (int i = 0; i < 2; i++) begin
            warp_pc[i] <= 32'h0;
            warp_states[i] <= WARP_IDLE;
        end
        active_warps <= 2'b00;
    end else if (start_execution) begin
        // Initialize warps based on kernel configuration
        for (int i = 0; i < 2; i++) begin
            if (i < num_warps) begin
                warp_pc[i] <= kernel_config.base_instructions_address;
                warp_states[i] <= WARP_FETCH;
                active_warps[i] <= 1'b1;
            end else begin
                warp_states[i] <= WARP_IDLE;
                active_warps[i] <= 1'b0;
            end
        end
    end else begin
        // Update warp states and PCs
        if (writeback_valid) begin
            int warp_id = writeback_stage_reg.warp_id;
            if (writeback_stage_reg.halt) begin
                warp_states[warp_id] <= WARP_DONE;
                active_warps[warp_id] <= 1'b0;
            end else if (writeback_stage_reg.branch && branch_mispredicted) begin
                warp_pc[warp_id] <= actual_pc;
            end else if (!fetch_stall) begin
                warp_pc[warp_id] <= next_pc;
            end
        end
    end
end

// Check if all warps are done
assign all_warps_done = (active_warps == 2'b00);

endmodule
