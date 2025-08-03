`timescale 1ns / 1ps
`include "common.svh"
// Added for the purpose of Vivado gpu wrapper to test the modules and get feedback

module gpu_wrapper (
    // Only the primary clock and reset are needed for synthesis analysis.
    input wire clk,
    input wire reset
);

    localparam NUM_CORES_PARAM                  = 1;
    localparam WARPS_PER_CORE_PARAM             = 2; // Matches your successful test config
    localparam THREADS_PER_WARP_PARAM           = 16;
    localparam DATA_MEM_NUM_CHANNELS_PARAM      = 8;
    localparam INSTR_MEM_NUM_CHANNELS_PARAM     = 8;

    // --- Control and Configuration Inputs (Tied to constants) ---
    logic  execution_start_in = 1'b0;
    logic [31:0] base_instr_in      = 32'd0;
    logic [31:0] base_data_in       = 32'd0;
    logic [31:0] num_blocks_in      = 32'd1;
    logic [31:0] warps_per_block_in = WARPS_PER_CORE_PARAM;

    // --- Control Output (Absorbed by a wire) ---
    wire   execution_done_out;

    // --- Instruction Memory Interface ---
    // Outputs from GPU are absorbed by internal wires.
    wire [INSTR_MEM_NUM_CHANNELS_PARAM-1:0]   imem_read_valid_out;
    wire [`INSTRUCTION_MEMORY_ADDRESS_WIDTH-1:0] imem_read_address_out [INSTR_MEM_NUM_CHANNELS_PARAM];
    // Inputs to GPU are driven by static logic. We tie 'ready' high to prevent stalls.
    logic [INSTR_MEM_NUM_CHANNELS_PARAM-1:0]  imem_read_ready_in = '1; // Always ready
    logic [`INSTRUCTION_WIDTH-1:0]            imem_read_data_in  [INSTR_MEM_NUM_CHANNELS_PARAM]; // Data can be X

    // --- Data Memory Interface ---
    // Outputs from GPU are absorbed by internal wires.
    wire [DATA_MEM_NUM_CHANNELS_PARAM-1:0]    dmem_read_valid_out;
    wire [`DATA_MEMORY_ADDRESS_WIDTH-1:0]   dmem_read_address_out [DATA_MEM_NUM_CHANNELS_PARAM];
    wire [DATA_MEM_NUM_CHANNELS_PARAM-1:0]    dmem_write_valid_out;
    wire [`DATA_MEMORY_ADDRESS_WIDTH-1:0]   dmem_write_address_out [DATA_MEM_NUM_CHANNELS_PARAM];
    wire [`DATA_WIDTH-1:0]                    dmem_write_data_out [DATA_MEM_NUM_CHANNELS_PARAM];
    // Inputs to GPU are driven by static logic. We tie 'ready' high to prevent stalls.
    logic [DATA_MEM_NUM_CHANNELS_PARAM-1:0]   dmem_read_ready_in  = '1; // Always ready
    logic [DATA_MEM_NUM_CHANNELS_PARAM-1:0]   dmem_write_ready_in = '1; // Always ready
    logic [`DATA_WIDTH-1:0]                   dmem_read_data_in   [DATA_MEM_NUM_CHANNELS_PARAM]; // Data can be X


    gpu #(
        .DATA_MEM_NUM_CHANNELS      (DATA_MEM_NUM_CHANNELS_PARAM),
        .INSTRUCTION_MEM_NUM_CHANNELS(INSTR_MEM_NUM_CHANNELS_PARAM),
        .NUM_CORES                  (NUM_CORES_PARAM),
        .WARPS_PER_CORE             (WARPS_PER_CORE_PARAM),
        .THREADS_PER_WARP           (THREADS_PER_WARP_PARAM)
    ) gpu_inst (
        .clk                        (clk),
        .reset                      (reset),

        // --- Configuration Inputs ---
        .base_instr                 (base_instr_in),
        .base_data                  (base_data_in),
        .num_blocks                 (num_blocks_in),
        .warps_per_block            (warps_per_block_in),
        .execution_start            (execution_start_in),
        .execution_done             (execution_done_out),

        // --- Instruction Memory Interface ---
        .instruction_mem_read_valid   (imem_read_valid_out),
        .instruction_mem_read_address (imem_read_address_out),
        .instruction_mem_read_ready   (imem_read_ready_in),
        .instruction_mem_read_data    (imem_read_data_in),

        // --- Data Memory Interface ---
        .data_mem_read_valid        (dmem_read_valid_out),
        .data_mem_read_address      (dmem_read_address_out),
        .data_mem_read_ready        (dmem_read_ready_in),
        .data_mem_read_data         (dmem_read_data_in),
        .data_mem_write_valid       (dmem_write_valid_out),
        .data_mem_write_address     (dmem_write_address_out),
        .data_mem_write_data        (dmem_write_data_out),
        .data_mem_write_ready       (dmem_write_ready_in)
    );

endmodule
