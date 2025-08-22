`timescale 1ns/1ns

`include "common.svh"

// Vector register file with synchronous read for proper BRAM inference.
module reg_file #(
    parameter int THREADS_PER_WARP = 4,
    parameter int DATA_WIDTH = `DATA_WIDTH
) (
    input   wire                            clk,
    input   wire                            reset,
    input   wire                            enable, // Warp enable signal

    // Per-thread enable signals (execution mask)
    input   logic   [THREADS_PER_WARP-1:0]  thread_enable,

    // Warp and block identifiers
    input   data_t                          warp_id,
    input   data_t                          block_id,
    input   data_t                          block_size,
    input   warp_state_t                    warp_state,

    // Decoded instruction fields
    input   logic                           decoded_reg_write_enable,
    input   reg_input_mux_t                 decoded_reg_input_mux,
    input   data_t                          decoded_immediate,
    input   logic   [4:0]                   decoded_rd_address,
    input   logic   [4:0]                   decoded_rs1_address,
    input   logic   [4:0]                   decoded_rs2_address,

    // Inputs from ALU and LSU per thread
    input   data_t                          alu_out [THREADS_PER_WARP],
    input   data_t                          lsu_out [THREADS_PER_WARP],

    // Outputs per thread (now registered)
    output  data_t                          rs1     [THREADS_PER_WARP],
    output  data_t                          rs2     [THREADS_PER_WARP]
);

    // Special-purpose register indices
    localparam int ZERO_REG         = 0;
    localparam int THREAD_ID_REG    = 29;
    localparam int BLOCK_ID_REG     = 30;
    localparam int BLOCK_SIZE_REG   = 31;

    // Register file: an array of 32 registers for each thread.
    // This will be inferred as one or more Block RAMs.
    (* ram_style = "block" *) data_t registers [THREADS_PER_WARP][32];

    // Compute thread IDs combinationally. This is fast and fine.
    data_t thread_ids [THREADS_PER_WARP];
    always_comb begin
        for (int i = 0; i < THREADS_PER_WARP; i++) begin
            thread_ids[i] = warp_id * THREADS_PER_WARP + i;
        end
    end

    // =========================================================================
    // --- Synchronous Read and Write Logic ---
    // Both read and write operations happen on the clock edge.
    // This is the correct coding style to infer a Block RAM.
    // =========================================================================
    always @(posedge clk) begin
        if (reset) begin
            // Initialize all registers for all threads only on reset.
            for (int i = 0; i < THREADS_PER_WARP; i++) begin
                registers[i][ZERO_REG]      <= '0;
                registers[i][THREAD_ID_REG] <= thread_ids[i];
                registers[i][BLOCK_ID_REG]  <= block_id;
                registers[i][BLOCK_SIZE_REG]<= block_size;
                for (int j = 1; j < 29; j++) begin
                    registers[i][j] <= '0;
                end
            end
        end else if (enable) begin
            // Loop through each thread's register file.
            for (int i = 0; i < THREADS_PER_WARP; i++) begin
                // --- WRITE LOGIC (happens only in UPDATE state) ---
                // The write logic is guarded by the thread_enable mask and the warp state.
                if (thread_enable[i] && (warp_state == WARP_UPDATE)) begin
                    // Prevent writes to read-only registers.
                    if (decoded_reg_write_enable && decoded_rd_address > 0 && decoded_rd_address < 29 ) begin
                        case (decoded_reg_input_mux)
                            ALU_OUT:   registers[i][decoded_rd_address] <= alu_out[i];
                            LSU_OUT:   registers[i][decoded_rd_address] <= lsu_out[i];
                            IMMEDIATE: registers[i][decoded_rd_address] <= decoded_immediate;
                            // VECTOR_TO_SCALAR is a no-op for the vector file
                            default:   begin /* do nothing */ end
                        endcase
                    end
                end
            end
        end

        // --- SYNCHRONOUS READ LOGIC (always active when not in reset) ---
        // The read addresses are presented to the BRAM, and the data appears at the
        // `rs1`/`rs2` outputs on the NEXT clock cycle.
        // This logic is OUTSIDE the `if (enable)` block for the write path, but
        // it still respects the clock. However, to prevent reading invalid data
        // when the warp is stalled, we can also put it inside the `if (enable)`.
        // For simplicity and correctness with stalling, let's put it inside.
        if (enable) begin
             for (int i = 0; i < THREADS_PER_WARP; i++) begin
                rs1[i] <= (decoded_rs1_address == ZERO_REG) ? '0 : registers[i][decoded_rs1_address];
                rs2[i] <= (decoded_rs2_address == ZERO_REG) ? '0 : registers[i][decoded_rs2_address];
            end
        end
    end
endmodule
