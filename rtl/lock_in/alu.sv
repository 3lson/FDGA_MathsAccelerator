`default_nettype none
`timescale 1ns/1ns

`include "common.sv"

module alu (
    input wire clk, 
    input wire reset,
    input wire enable,
    input instruction_memory_address_t pc,
    input   alu_instruction_t instruction,
    input   data_t ALUop1,    
    input   data_t ALUop2, 
    input   data_t IMM,   
    output  data_t Result, 
    output  logic EQ      
);

    always_ff @(posedge clk) begin
        if (reset) begin
            Result <= 32'b0;
            EQ <= 1'b0;
        end else if (enable) begin
            case (instruction)
                // --- R-Type: Control Flow ---
                ADD: Result <= ALUop1 + ALUop2; // ADD 
                SUB: Result <= ALUop1 - ALUop2; // SUB
                MUL: Result <= ALUop1 * ALUop2; // MULTIPLY
                DIV: Result <= ALUop1 / ALUop2; // DIVIDE
                ABS: Result <= {1'b0,ALUop1[30:0]}; //ABSOLUTE (AS PER IEEE 754 SINGLE PRECISION FLOATING POINT NUMBERS)
                SLT: Result <= (ALUop1 < ALUop2) ? 1:0; //LESS THAN
                SGT: Result <= (ALUop1 > ALUop2) ? 1:0; //GREATER THAN
                SEQ: begin //EQUALS
                    Result <= (ALUop1 == ALUop2) ? 1:0; 
                    EQ <= (ALUop1 == ALUop2) ? 1:0;
                end
                SNEZ: begin //Set if Not Equals
                    Result <= (ALUop1 != 0) ? 1 : 0;
                    EQ <= (ALUop1 != 0) ? 1:0;
                end
                MIN: Result <= (ALUop1 < ALUop2) ? ALUop1:ALUop2; //MINIMUM INSTRUCTION

                // --- I-Type: Control Flow ---
                ADDI: Result <= ALUop1 + IMM;
                MULI: Result <= ALUop1 * IMM; // MULTIPLY
                DIVI: Result <= ALUop1 / IMM; // DIVIDE
                SLLI: Result <= ALUop1 << IMM[4:0];

                // --- C-Type: Control Flow ---
                BEQZ: begin
                    // The ALU's job is just to check the condition
                    Result <= (ALUop1 == 32'd0) ? 32'd1 : 32'd0;
                    EQ     <= (ALUop1 == 32'd0);
                end
                JAL: begin
                    // The result is the target address: PC + offset
                    Result <= pc + IMM;
                end
                default: Result <= 32'b0;    
            endcase
        end
    end
endmodule

