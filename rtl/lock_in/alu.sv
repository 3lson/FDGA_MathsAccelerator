`include "common.sv"

module alu (
    input   alu_instruction_t instruction,
    input   logic [31:0] ALUop1,    
    input   logic [31:0] ALUop2, 
    input   logic [31:0] IMM,   
    output  logic [31:0] Result, 
    output  logic EQ      
);

    always_comb begin
        Result = 32'b0;
        EQ = 1'b0;

        case (instruction)
            ADD: Result = ALUop1 + ALUop2; // ADD 
            SUB: Result = ALUop1 - ALUop2; // SUB
            MUL: Result = ALUop1 * ALUop2; // MULTIPLY
            DIV: Result = ALUop1 / ALUop2; // DIVIDE
            ABS: Result = {1'b0,ALUop1[30:0]}; //ABSOLUTE (AS PER IEEE 754 SINGLE PRECISION FLOATING POINT NUMBERS)
            SLT: Result = (ALUop1 < ALUop2) ? 1:0; //LESS THAN
            SGT: Result = (ALUop1 > ALUop2) ? 1:0; //GREATER THAN
            SEQ: begin //EQUALS
                Result = (ALUop1 == ALUop2) ? 1:0; 
                EQ = (ALUop1 == ALUop2) ? 1:0;
            end
            SNEZ: begin //Set if Not Equals
                Result = (ALUop1 != 0) ? 1 : 0;
                EQ = (ALUop1 != 0) ? 1:0;
            end
            MIN: Result = (ALUop1 < ALUop2) ? ALUop1:ALUop2; //MINIMUM INSTRUCTION
            SLLI: Result = ALUop1 << ALUop2;
            ADDI: Result = ALUop1 + IMM;
            MULI: Result = ALUop1 * IMM; // MULTIPLY
            DIVI: Result = ALUop1 / IMM; // DIVIDE
            SLLI: Result <= ALUop1 << IMM;
            default: Result = 32'b0;    
        endcase
    end
endmodule

