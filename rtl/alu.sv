`include "define.sv"

module alu (
    input  logic [31:0] ALUop1,    
    input  logic [31:0] ALUop2,    
    input  logic [3:0]  ALUctrl,    
    output logic [31:0] Result, 
    output logic EQ      
);

    always_comb begin
        Result = 32'b0;
        EQ = 1'b0;

        case (ALUctrl)
            `ALU_ADD: Result = ALUop1 + ALUop2; // ADD 
            `ALU_SUB: Result = ALUop1 - ALUop2; // SUB
            `ALU_MUL: Result = ALUop1 * ALUop2; // MULTIPLY
            `ALU_DIV: Result = ALUop1 / ALUop2; // DIVIDE
            `ALU_ABS: Result = {1'b0,ALUop1[30:0]}; //ABSOLUTE (AS PER IEEE 754 SINGLE PRECISION FLOATING POINT NUMBERS)
            `ALU_SLT: Result = (ALUop1 < ALUop2) ? 1:0; //LESS THAN
            `ALU_SEQ: begin //EQUALS
                Result = (ALUop1 == ALUop2) ? 1:0; 
                EQ = (ALUop1 == ALUop2) ? 1:0;
            end
            `ALU_MIN: Result = (ALUop1 < ALUop2) ? ALUop1:ALUop2; //MINIMUM INSTRUCTION
            default: Result = 32'b0;    
        endcase
    end
endmodule

