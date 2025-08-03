`timescale 1ns/1ns

// This simulates the execution of a block of warps which contains multiple threads

`include "common.svh"

module compute_core#(
    parameter int WARPS_PER_CORE = 4,            // Number of warps to in each core
    parameter int THREADS_PER_WARP = 16          // Number of threads per warp (max 32)
    )(
    input   wire                            clk,
    input   wire                            reset,

    input   logic                           start,
    output  logic                           done,

    input   data_t                          block_id,
    input   kernel_config_t                 kernel_config,

    // Instruction Memory
    input   logic   [WARPS_PER_CORE-1:0]    instruction_mem_read_ready,
    input   instruction_t                   instruction_mem_read_data       [WARPS_PER_CORE],
    output  logic   [WARPS_PER_CORE-1:0]    instruction_mem_read_valid,
    output  instruction_memory_address_t    instruction_mem_read_address    [WARPS_PER_CORE],

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
localparam int WARP_INDEX_WIDTH = (WARPS_PER_CORE > 1) ? $clog2(WARPS_PER_CORE) : 1;

// Warp State and Control
logic [WARP_INDEX_WIDTH-1:0] current_warp;
warp_state_t warp_state [WARPS_PER_CORE];
fetcher_state_t fetcher_state [WARPS_PER_CORE];
warp_state_t current_warp_state;
assign current_warp_state = warp_state[current_warp];
instruction_t fetched_instruction [WARPS_PER_CORE];

instruction_memory_address_t pc [WARPS_PER_CORE];
instruction_memory_address_t next_pc [WARPS_PER_CORE];

logic start_execution; // EDA: Unimportant hack used because of EDA tooling

data_t num_warps;
assign num_warps = kernel_config.num_warps_per_block;

// Decoded instruction fields per warp
logic decoded_reg_write_enable [WARPS_PER_CORE];
reg_input_mux_t decoded_reg_input_mux [WARPS_PER_CORE];
data_t decoded_immediate [WARPS_PER_CORE];
logic decoded_branch [WARPS_PER_CORE];
logic decoded_scalar_instruction [WARPS_PER_CORE];
logic [4:0] decoded_rd_address [WARPS_PER_CORE];
logic [4:0] decoded_rs1_address [WARPS_PER_CORE];
logic [4:0] decoded_rs2_address [WARPS_PER_CORE];
logic [4:0] decoded_alu_instruction [WARPS_PER_CORE];
logic decoded_halt [WARPS_PER_CORE];
logic [1:0] floatingRead_flag [WARPS_PER_CORE];
logic floatingWrite_flag [WARPS_PER_CORE];

// Signals are per-warp control not pre-thread
logic decoded_mem_read_enable_per_warp [WARPS_PER_CORE];
logic decoded_mem_write_enable_per_warp [WARPS_PER_CORE];
// logic decoded_mem_read_enable [THREADS_PER_WARP];
// logic decoded_mem_write_enable [THREADS_PER_WARP];

// Register File Outputs (per warp)
data_t scalar_int_rs1 [WARPS_PER_CORE];
data_t scalar_int_rs2 [WARPS_PER_CORE];
data_t scalar_float_rs1 [WARPS_PER_CORE];
data_t scalar_float_rs2 [WARPS_PER_CORE];

data_t vector_int_rs1 [WARPS_PER_CORE][THREADS_PER_WARP];
data_t vector_int_rs2 [WARPS_PER_CORE][THREADS_PER_WARP];
data_t vector_float_rs1 [WARPS_PER_CORE][THREADS_PER_WARP];
data_t vector_float_rs2 [WARPS_PER_CORE][THREADS_PER_WARP];

warp_mask_t warp_execution_mask [WARPS_PER_CORE];

logic all_warps_done = 1'b1;

// ALU/LSU Operands and Results
data_t final_op1 [THREADS_PER_WARP];
data_t final_op2 [THREADS_PER_WARP];
data_t alu_out [THREADS_PER_WARP];
data_t scalar_alu_out;
data_t lsu_out [THREADS_PER_WARP];
lsu_state_t lsu_state [THREADS_PER_WARP];
data_t scalar_lsu_out;
lsu_state_t scalar_lsu_state;
data_t vector_to_scalar_data [WARPS_PER_CORE];
logic vector_fpu_valid [THREADS_PER_WARP];
logic all_vector_fpus_done;

// Operand Muxing Logic
warp_mask_t current_warp_execution_mask;
assign current_warp_execution_mask = warp_execution_mask[current_warp];
data_t scalar_op1;
data_t scalar_op2;

// Combinational Mux for Scalar Operands
always_comb begin
    // $display("FloatingRead ", floatingRead_flag[current_warp]);
    // $display("Scalar_op1 ", scalar_op1);
    // $display("Scalar_op2 ", scalar_op2);
    // $display("Scalar_int1 ", scalar_int_rs1[current_warp]);
    // $display("Scalar_float2 ", scalar_float_rs2[current_warp]);
    case(floatingRead_flag[current_warp])
        2'b00: {scalar_op1, scalar_op2} = {scalar_int_rs1[current_warp], scalar_int_rs2[current_warp]};
        2'b01: {scalar_op1, scalar_op2} = {scalar_float_rs1[current_warp], scalar_int_rs2[current_warp]};
        2'b10: {scalar_op1, scalar_op2} = {scalar_int_rs1[current_warp], scalar_float_rs2[current_warp]};
        2'b11: {scalar_op1, scalar_op2} = {scalar_float_rs1[current_warp], scalar_float_rs2[current_warp]};
        default: {scalar_op1, scalar_op2} = {scalar_int_rs1[current_warp], scalar_int_rs2[current_warp]};
    endcase
end

//Generate Muxing logic for each of the Vector SIMT lanes
generate
for (genvar i = 0; i < THREADS_PER_WARP; i++) begin: g_operand_mux
    logic [31:0] vector_op1;
    logic [31:0] vector_op2;

    // Combinational Mux for Vector Operand for thread 'i'
    // This block now correctly selects from the 2D register file output arrays.
    always_comb begin
        case(floatingRead_flag[current_warp])
            2'b00: {vector_op1, vector_op2} = {vector_int_rs1[current_warp][i],   vector_int_rs2[current_warp][i]};
            2'b01: {vector_op1, vector_op2} = {vector_float_rs1[current_warp][i], vector_int_rs2[current_warp][i]};
            2'b10: {vector_op1, vector_op2} = {vector_int_rs1[current_warp][i],   vector_float_rs2[current_warp][i]};
            2'b11: {vector_op1, vector_op2} = {vector_float_rs1[current_warp][i], vector_float_rs2[current_warp][i]};
            default: {vector_op1, vector_op2} = {vector_int_rs1[current_warp][i], vector_int_rs2[current_warp][i]};
        endcase
        // $display("Scalar flag: ", decoded_scalar_instruction[current_warp]);
        // $display("Vector int rs1: ", vector_int_rs1[i]);
        // $display("Vector op2: ", vector_op2);
    end

    assign final_op1[i] = decoded_scalar_instruction[current_warp] ? scalar_op1 : vector_op1;
    assign final_op2[i] = decoded_scalar_instruction[current_warp] ? scalar_op2 : vector_op2;
end
endgenerate

always_comb begin
    all_warps_done = 1'b1;
    for (int i = 0; i < WARPS_PER_CORE; i = i + 1) begin
        if (i < num_warps) begin
            if (warp_state[i] != WARP_DONE) begin
                all_warps_done = 1'b0;
            end
        end
    end
end

always_comb begin
    all_vector_fpus_done = 1'b1;
    for (int i = 0; i < THREADS_PER_WARP; i++) begin
        if (current_warp_execution_mask[i] && !vector_fpu_valid[i]) begin
            all_vector_fpus_done=1'b0;
        end
    end
end

//State machine and Scheduler Logic
always @(posedge clk) begin
    // $display("Start execution: ", start_execution);
    // $display("Reset: ", reset);
    if (reset) begin
        //$display("Resetting core %0d", block_id);
        start_execution <= 0;
        done <= 0;
        for (int i = 0; i < WARPS_PER_CORE; i = i + 1) begin
            warp_state[i] <= WARP_IDLE;
            fetcher_state[i] <= FETCHER_IDLE;
            pc[i] <= 0;
            next_pc[i] <= 0;
            current_warp <= 0;
        end

    end else if (!start_execution) begin
        if (start) begin
            $display("Starting execution of block %d", block_id);
            $display("GPU: Kernel configuration (latched):");
            $display("     - Base instruction address: %h", kernel_config.base_instructions_address);
            $display("     - Base data address: %h", kernel_config.base_data_address);
            $display("     - Num %d blocks", kernel_config.num_blocks);
            $display("     - Number of warps per block: %d", kernel_config.num_warps_per_block);
            // Set all warps to fetch state on start
            start_execution <= 1;
            current_warp <= 0;
            for (int i = 0; i < WARPS_PER_CORE; i = i + 1) begin
                if (i < num_warps) begin
                    warp_state[i] = WARP_FETCH;
                    fetcher_state[i] = FETCHER_IDLE;
                    pc[i] = kernel_config.base_instructions_address;
                    next_pc[i] = kernel_config.base_instructions_address;
                end
            end
        end
    end else begin
        // In parallel, check if fetchers are done, and if so, move to decode
        for (int i = 0; i < WARPS_PER_CORE; i = i + 1) begin
            if ( i < num_warps ) begin
                if (warp_state[i] == WARP_FETCH && fetcher_state[i] == FETCHER_DONE) begin
                    $display("Block: %0d: Warp %0d: Fetched instruction %h at address %h", block_id, i, fetched_instruction[i], pc[i]);
                    warp_state[i] = WARP_DECODE;
                end
            end
        end
        done <= all_warps_done;

        if (current_warp_state == WARP_UPDATE || current_warp_state == WARP_DONE || current_warp_state == WARP_WAIT) begin
            int next_warp = (current_warp + 1) % num_warps;
            int found_warp = -1;
            $display("Block: %0d: Choosing next warp", block_id);
            for (int i = 0; i < WARPS_PER_CORE; i = i + 1) begin
                int warp_index = (next_warp + i) % num_warps;
                if ((warp_state[warp_index] != WARP_IDLE) && (warp_state[warp_index] != WARP_FETCH) && (warp_state[warp_index] != WARP_DONE)) begin
                    found_warp = warp_index;
                end
            end
            if (found_warp != -1) begin
                current_warp <= found_warp;
            end else begin
                current_warp <= current_warp;
            end
        end

        case (current_warp_state)
            WARP_IDLE: begin
                $display("Block: %0d: Warp %0d: Idle", block_id, current_warp);
            end
            WARP_FETCH: begin
                // not possible to choose a warp that is fetching cause
                // fetching is done in parallel
            end
            WARP_DECODE: begin
                // decoding takes one cycle
                warp_state[current_warp] <= WARP_REQUEST;
            end
            WARP_REQUEST: begin
                // takes one cycle cause we are just changing the LSU state
                warp_state[current_warp] <= WARP_WAIT;
            end
            WARP_WAIT: begin
                reg any_lsu_waiting = 1'b0;
                for (int i = 0; i < THREADS_PER_WARP; i++) begin
                    // Make sure no lsu_state = REQUESTING or WAITING
                    if (lsu_state[i] == LSU_REQUESTING || lsu_state[i] == LSU_WAITING) begin
                        any_lsu_waiting = 1'b1;
                        break;
                    end
                end

                if (scalar_lsu_state == LSU_REQUESTING || scalar_lsu_state == LSU_WAITING) begin
                    any_lsu_waiting = 1'b1;
                end

                // If no LSU is waiting for a response, move onto the next stage
                if (!any_lsu_waiting) begin
                    warp_state[current_warp] <= WARP_EXECUTE;
                end
            end
            WARP_EXECUTE: begin
                $display("===================================");
                $display("Mask: %32b", warp_execution_mask[current_warp]);
                $display("Block: %0d: Warp %0d: Executing instruction %h at address %h", block_id, current_warp, fetched_instruction[current_warp], pc[current_warp]);
                //$display("Instruction opcode: %b", fetched_instruction[current_warp][31:29]);
                //$display("Block: %0d: Warp %0d: Executing instruction %h", block_id, current_warp, fetched_instruction[current_warp]);
                
                // Determine the next state based on the instruction type
                if (decoded_alu_instruction[current_warp] >= FADD && decoded_alu_instruction[current_warp] < BEQZ) begin
                    // It's a Floating Point instruction, go to the FPU wait state
                    warp_state[current_warp] <= WARP_ALU_WAIT;
                end else if ((decoded_alu_instruction[current_warp] < FADD) || decoded_branch[current_warp] || (decoded_alu_instruction[current_warp] == JAL)) begin
                    warp_state[current_warp] <= WARP_INT_ALU_WAIT;
                end else begin
                    warp_state[current_warp] <= WARP_UPDATE;
                end

                // Handle vector-to-scalar write setup
                if (decoded_reg_input_mux[current_warp] == VECTOR_TO_SCALAR) begin
                    data_t scalar_write_value;
                    scalar_write_value = {`DATA_WIDTH{1'b0}};
                    for (int i = 0; i < THREADS_PER_WARP; i++) begin
                        scalar_write_value[i] = alu_out[i][0];
                    end
                    vector_to_scalar_data[current_warp] <= scalar_write_value;
                end else begin
                    vector_to_scalar_data[current_warp] <= {`DATA_WIDTH{1'b0}};
                end
            end

            WARP_INT_ALU_WAIT: begin
                // This state is just a one-cycle delay for the 2-stage ALU.
                warp_state[current_warp] <= WARP_UPDATE;
            end

            WARP_ALU_WAIT: begin
                // This state remains the same, for the FPU
                if (decoded_scalar_instruction[current_warp]) begin
                    if(scalar_fpu_valid) begin
                        warp_state[current_warp] <= WARP_UPDATE;
                    end
                end else begin
                    if(all_vector_fpus_done) begin
                        warp_state[current_warp] <= WARP_UPDATE;
                    end
                end
            end
            
            WARP_UPDATE: begin
                if (decoded_halt[current_warp]) begin
                    $display("Block: %0d: Warp %0d: Finished executing instruction %h", block_id, current_warp, fetched_instruction[current_warp]);
                    warp_state[current_warp] <= WARP_DONE;
                end else begin
                    logic [`INSTRUCTION_MEMORY_ADDRESS_WIDTH-1:0] resolved_next_pc = pc[current_warp] + 1;

                    if (decoded_branch[current_warp] && (scalar_alu_out == 1)) begin
                        resolved_next_pc = pc[current_warp] + decoded_immediate[current_warp];
                    end
                    else if (decoded_alu_instruction[current_warp] == JAL) begin
                        resolved_next_pc = scalar_alu_out;
                    end
                    
                    pc[current_warp] <= resolved_next_pc;
                    warp_state[current_warp] <= WARP_FETCH;
                end
            end

            WARP_DONE: begin
                // we chillin
            end
        endcase
    end
end

// This block generates warp control circuitry
generate
for (genvar i = 0; i < WARPS_PER_CORE; i = i + 1) begin : g_warp
    fetcher fetcher_inst(
        .clk(clk),
        .reset(reset),

        .warp_state(warp_state[i]),
        .pc(pc[i]),

        // Instruction Memory
        .instruction_mem_read_ready(instruction_mem_read_ready[i]),
        .instruction_mem_read_data(instruction_mem_read_data[i]),
        .instruction_mem_read_valid(instruction_mem_read_valid[i]),
        .instruction_mem_read_address(instruction_mem_read_address[i]),

        // Fetcher output
        .fetcher_state(fetcher_state[i]),
        .instruction(fetched_instruction[i])
    );

    decoder decoder_inst(
        .clk(clk),
        .reset(reset),
        .warp_state(warp_state[i]),

        .instruction(fetched_instruction[i]),

        .decoded_reg_write_enable(decoded_reg_write_enable[i]),
        .decoded_mem_write_enable(decoded_mem_write_enable_per_warp[i]),
        .decoded_mem_read_enable(decoded_mem_read_enable_per_warp[i]),
        .decoded_branch(decoded_branch[i]),
        .decoded_scalar_instruction(decoded_scalar_instruction[i]),
        .decoded_reg_input_mux(decoded_reg_input_mux[i]),
        .decoded_immediate(decoded_immediate[i]),
        .decoded_rd_address(decoded_rd_address[i]),
        .decoded_rs1_address(decoded_rs1_address[i]),
        .decoded_rs2_address(decoded_rs2_address[i]),
        .decoded_alu_instruction(decoded_alu_instruction[i]),

        .decoded_halt(decoded_halt[i]),
        .floatingRead(floatingRead_flag[i]),
        .floatingWrite(floatingWrite_flag[i])
    );
    // Scalar float register file
    scalar_reg_file #(
        .DATA_WIDTH(32)
    ) floating_scalar_reg_file_inst (
        .clk(clk),
        .reset(reset),
        .enable((current_warp == i)), // Enable when current_warp matches and warp is active

        //.warp_execution_mask(warp_execution_mask[i]),

        .warp_state(warp_state[i]),

        .decoded_reg_write_enable(decoded_reg_write_enable[i] && decoded_scalar_instruction[i] && floatingWrite_flag[i]),
        .decoded_reg_input_mux(decoded_reg_input_mux[i]),
        .decoded_immediate(decoded_immediate[i]),
        .decoded_rd_address(decoded_rd_address[i]),
        .decoded_rs1_address(decoded_rs1_address[i]),
        .decoded_rs2_address(decoded_rs2_address[i]),

        .alu_out(scalar_alu_out),
        .lsu_out(scalar_lsu_out),
        .pc(pc[i]),
        .vector_to_scalar_data(vector_to_scalar_data[i]),

        .rs1(scalar_float_rs1[i]),
        .rs2(scalar_float_rs2[i])
    );

    always_comb begin
        if (current_warp == i) begin
            //$display("Enable: ", current_warp == i);
            //$display("Warp State: ", warp_state[i]);
            //$display("Decoded_rs1: ", decoded_rs1_address[i]);
            //$display("Decoded_rs2: ", decoded_rs2_address[i]);
            //$display("ALU out", scalar_alu_out);
            //$display("ALU out", scalar_alu_out);
            //$display("Scalar_lsu: ", scalar_lsu_out);
            //$display("Scalar_float_rs1: ", scalar_float_rs1[i]);
            //$display("Scalar_float_rs2: ", scalar_float_rs2[i]);
            //$display("Floating Write Flag", floatingWrite_flag[i]);
        end
    end

    //Scalar integer register file
    scalar_reg_file #(
        .DATA_WIDTH(32)
    ) scalar_reg_file_inst (
        .clk(clk),
        .reset(reset),
        .enable((current_warp == i)), // Enable when current_warp matches and warp is active

        .warp_execution_mask(warp_execution_mask[i]),

        .warp_state(warp_state[i]),

        .decoded_reg_write_enable(decoded_reg_write_enable[i] && ((decoded_scalar_instruction[i] && !floatingWrite_flag[i]) ||(decoded_reg_input_mux[i] == VECTOR_TO_SCALAR))),
        .decoded_reg_input_mux(decoded_reg_input_mux[i]),
        .decoded_immediate(decoded_immediate[i]),
        .decoded_rd_address(decoded_rd_address[i]),
        .decoded_rs1_address(decoded_rs1_address[i]),
        .decoded_rs2_address(decoded_rs2_address[i]),

        .alu_out(scalar_alu_out),
        .lsu_out(scalar_lsu_out),
        .pc(pc[i]),
        .vector_to_scalar_data(vector_to_scalar_data[i]),

        .rs1(scalar_int_rs1[i]),
        .rs2(scalar_int_rs2[i])
    );
    // Vector float register file
    reg_file #(
            .THREADS_PER_WARP(THREADS_PER_WARP)
        ) floating_reg_file_inst (
            .clk(clk),
            .reset(reset),
            .enable((current_warp == i)), // Enable when current_warp matches and warp is active

            // Thread enable signals (execution mask)
            .thread_enable(warp_execution_mask[i]),

            // Warp and block identifiers
            .warp_id(i),
            .block_id(block_id),
            .block_size(kernel_config.num_warps_per_block * THREADS_PER_WARP),
            .warp_state(warp_state[i]),

            // Decoded instruction fields for this warp
            .decoded_reg_write_enable(decoded_reg_write_enable[i] && !decoded_scalar_instruction[i] && floatingWrite_flag[i]),
            .decoded_reg_input_mux(decoded_reg_input_mux[i]),
            .decoded_immediate(decoded_immediate[i]),
            .decoded_rd_address(decoded_rd_address[i]),
            .decoded_rs1_address(decoded_rs1_address[i]),
            .decoded_rs2_address(decoded_rs2_address[i]),

            // Inputs from ALU and LSU per thread
            .alu_out(alu_out), // ALU outputs for all threads
            .lsu_out(lsu_out),

            // Outputs per thread
            .rs1(vector_float_rs1[i]),
            .rs2(vector_float_rs2[i])
        );


    // Vector integer register file
    reg_file #(
            .THREADS_PER_WARP(THREADS_PER_WARP)
        ) reg_file_inst (
            .clk(clk),
            .reset(reset),
            .enable((current_warp == i)), // Enable when current_warp matches and warp is active

            // Thread enable signals (execution mask)
            .thread_enable(warp_execution_mask[i]),

            // Warp and block identifiers
            .warp_id(i),
            .block_id(block_id),
            .block_size(kernel_config.num_warps_per_block * THREADS_PER_WARP),
            .warp_state(warp_state[i]),

            // Decoded instruction fields for this warp
            .decoded_reg_write_enable(decoded_reg_write_enable[i] && !decoded_scalar_instruction[i] && !floatingWrite_flag[i]),
            .decoded_reg_input_mux(decoded_reg_input_mux[i]),
            .decoded_immediate(decoded_immediate[i]),
            .decoded_rd_address(decoded_rd_address[i]),
            .decoded_rs1_address(decoded_rs1_address[i]),
            .decoded_rs2_address(decoded_rs2_address[i]),

            // Inputs from ALU and LSU per thread
            .alu_out(alu_out), // ALU outputs for all threads
            .lsu_out(lsu_out),

            // Outputs per thread
            .rs1(vector_int_rs1[i]),
            .rs2(vector_int_rs2[i])
        );
    
end
endgenerate


// This block generates shared core resources (ALUs, LSUs)

//Scalar functional units
data_t scalar_int_alu_result, scalar_float_alu_result;
logic scalar_fpu_valid;
wire is_scalar_float_op = decoded_scalar_instruction[current_warp] && (decoded_alu_instruction[current_warp] >= FADD);
wire is_branch_or_jal = decoded_branch[current_warp] || (decoded_alu_instruction[current_warp] == JAL);
wire is_regular_int_op = decoded_alu_instruction[current_warp] < FADD;

// The ALU should be enabled if the current instruction is a scalar op that fits any of the above criteria.
wire is_scalar_int_op = decoded_scalar_instruction[current_warp] && (is_regular_int_op || is_branch_or_jal);

// Scalar ALU
alu scalar_alu_inst(
    .clk(clk),
    .rst(rst),
    .enable(is_scalar_int_op),
    .pc(pc[current_warp]),
    .ALUop1(scalar_op1),
    .ALUop2(scalar_op2),
    .IMM(decoded_immediate[current_warp]),
    .instruction(decoded_alu_instruction[current_warp]),

    .Result(scalar_int_alu_result)
    // .EQ() // unused
);

// Scalar Floating ALU
floating_alu scalar_fpu_inst(
    .clk(clk),
    .rst(rst),
    .valid(scalar_fpu_valid),
    .op1(scalar_op1),
    .op2(scalar_op2),
    .instruction(decoded_alu_instruction[current_warp]),

    .result(scalar_float_alu_result)
);

logic more_fadd;
assign more_fadd = (decoded_alu_instruction[current_warp] >= FADD);
logic less_beqz;
assign less_beqz = (decoded_alu_instruction[current_warp] < BEQZ);

assign scalar_alu_out = (more_fadd & less_beqz) ? scalar_float_alu_result : scalar_int_alu_result;
// assign scalar_alu_out = is_scalar_float_op ? scalar_float_alu_result : scalar_int_alu_result;

lsu scalar_lsu_inst(
    .clk(clk),
    .reset(reset),
    .enable(decoded_scalar_instruction[current_warp]),

    .warp_state(warp_state[current_warp]),

    .decoded_mem_read_enable(decoded_mem_read_enable_per_warp[current_warp]),
    .decoded_mem_write_enable(decoded_mem_write_enable_per_warp[current_warp]),

    .rs1(scalar_op1),
    .rs2(scalar_op2),
    .imm(decoded_immediate[current_warp]),

    // Data Memory connections
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

//Vecotr functional units
generate
    for (genvar i = 0; i < THREADS_PER_WARP; i = i + 1) begin : g_vector_units
        wire t_enable = current_warp_execution_mask[i] && !decoded_scalar_instruction[current_warp];
        wire is_vector_float_op = t_enable && (decoded_alu_instruction[current_warp] >= FADD);
        wire is_vector_int_op  = t_enable && (decoded_alu_instruction[current_warp] < FADD);
       
        data_t vector_int_alu_result, vector_float_alu_result;

        // Vector ALU
        alu vector_alu_inst(
            .clk(clk),
            .rst(rst),
            .enable(is_vector_int_op),
            .pc(pc[current_warp]),
            .ALUop1(final_op1[i]),
            .ALUop2(final_op2[i]),
            .IMM(decoded_immediate[current_warp]),
            .instruction(decoded_alu_instruction[current_warp]),

            .Result(vector_int_alu_result)
            // .EQ() // unused
        );
        // Vector Floating ALU
        floating_alu vector_fpu_inst(
            .clk(clk),
            .rst(rst),
            .valid(vector_fpu_valid[i]),
            .op1(final_op1[i]),
            .op2(final_op2[i]),
            .instruction(decoded_alu_instruction[current_warp]),

            .result(vector_float_alu_result)
        );

        assign alu_out[i] = (decoded_alu_instruction[current_warp] >= FADD) ? vector_float_alu_result : vector_int_alu_result;
        
        lsu lsu_inst(
            .clk(clk),
            .reset(reset),
            .enable(t_enable),

            .warp_state(warp_state[current_warp]),

            .decoded_mem_read_enable(decoded_mem_read_enable_per_warp[current_warp]),
            .decoded_mem_write_enable(decoded_mem_write_enable_per_warp[current_warp]),

            .rs1(final_op1[i]),
            .rs2(final_op2[i]),
            .imm(decoded_immediate[current_warp]),

            // Data Memory connections
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
endmodule
