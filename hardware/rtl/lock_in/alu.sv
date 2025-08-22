
`timescale 1ns/1ns

`include "common.svh"

// 2-Stage Pipelined 32-bit Integer ALU

module alu (
    input   logic clk,
    input   logic rst,
    input logic enable, // For stalling

    // Original Inputs
    input   instruction_memory_address_t pc,
    input   alu_instruction_t instruction,
    input   data_t ALUop1,
    input   data_t ALUop2,
    input   data_t IMM,

    // Original Output (now registered)
    output  data_t Result
);

    // ========================================================================
    // --- Pipeline Stage 1: Input Registers ---
    // These registers hold the inputs stable for the calculation in Stage 2.
    // We use the "_s1" suffix to denote "Stage 1".
    // ========================================================================
    instruction_memory_address_t pc_s1;
    alu_instruction_t instruction_s1;
    data_t ALUop1_s1, ALUop2_s1, IMM_s1;

    // This clocked block captures the inputs on every rising clock edge.
    always_ff @(posedge clk) begin
        if (rst) begin
            // On reset, clear all pipeline registers to a known state.
            pc_s1          <= '0;
            instruction_s1 <= NOP; // NOP is a safe default instruction
            ALUop1_s1      <= '0;
            ALUop2_s1      <= '0;
            IMM_s1         <= '0;
        end else if (enable) begin
            // In normal operation, capture the current inputs.
            pc_s1          <= pc;
            instruction_s1 <= instruction;
            ALUop1_s1      <= ALUop1;
            ALUop2_s1      <= ALUop2;
            IMM_s1         <= IMM;
        end
    end


    // ========================================================================
    // --- Pipeline Stage 2: Combinational Calculation Logic ---
    // This logic calculates the result based on the registered inputs from Stage 1.
    // The output of this block, `Result_next`, is an intermediate wire that
    // will feed the final output register.
    // ========================================================================
    data_t Result_next;

    always_comb begin
        // The case statement is identical to your original, but it now uses the
        // registered "_s1" inputs instead of the direct module inputs.
        case (instruction_s1)
            // --- R-Type: Control Flow ---
            ADD:  Result_next = ALUop1_s1 + ALUop2_s1;
            SUB:  Result_next = ALUop1_s1 - ALUop2_s1;
            MUL:  Result_next = ALUop1_s1 * ALUop2_s1; // This now uses registered inputs
            ABS:  Result_next = (ALUop1_s1[31]) ? -ALUop1_s1 : ALUop1_s1;
            SLT:  Result_next = (ALUop1_s1 < ALUop2_s1) ? 32'd1 : 32'd0;
            SEQ:  Result_next = (ALUop1_s1 == ALUop2_s1) ? 32'd1 : 32'd0;
            SNEZ: Result_next = (ALUop1_s1 != 0) ? 32'd1 : 32'd0;
            MIN:  Result_next = (ALUop1_s1 < ALUop2_s1) ? ALUop1_s1 : ALUop2_s1;
            SLL:  Result_next = ALUop1_s1 << ALUop2_s1[4:0]; // Use only lower 5 bits for shift amount

            // --- I-Type: Control Flow ---
            ADDI: Result_next = ALUop1_s1 + IMM_s1;
            MULI: Result_next = ALUop1_s1 * IMM_s1; // This now uses registered inputs
            SLLI: Result_next = ALUop1_s1 << IMM_s1[4:0];
            SEQI: Result_next = (ALUop1_s1 == IMM_s1) ? 32'd1 : 32'd0;

            // --- C-Type: Control Flow ---
            BEQO: Result_next = (ALUop1_s1 == 32'd1) ? 32'd1 : 32'd0;
            BEQZ: Result_next = (ALUop1_s1 == 32'd0) ? 32'd1 : 32'd0;
            JAL:  Result_next = pc_s1 + IMM_s1;
            
            // --- CRITICAL WARNING ON DIVISION ---
            // The '/' operator creates extremely large and slow logic that will
            // likely FAIL timing, even in this pipelined structure. For a real
            // FPGA design, you MUST replace this with a dedicated multi-cycle
            // Divider IP Core from the Vivado IP Catalog.
            DIV:  Result_next = ALUop1_s1 / ALUop2_s1;
            DIVI: Result_next = ALUop1_s1 / IMM_s1;

            default: Result_next = 32'b0;
        endcase
    end

    // ========================================================================
    // --- Pipeline Stage 2: Final Output Register ---
    // The module's `Result` output is now a register. It captures the value
    // from the combinational logic (`Result_next`) on each clock edge.
    // ========================================================================
    always_ff @(posedge clk) begin
        if (rst) begin
            Result <= '0;
        end else if (enable) begin
            Result <= Result_next;
        end
    end

endmodule
