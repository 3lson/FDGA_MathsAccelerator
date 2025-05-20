module alu (
    input  logic [31:0] ALUop1,    
    input  logic [31:0] ALUop2,    
    input  logic [3:0]  ALUctrl,    
    output logic [31:0] Result,    
    output logic EQ                
);

//Signed values for ASR and SLT
wire signed [31:0] ALUop1_signed;
wire signed [31:0] ALUop2_signed;
assign ALUop1_signed = ALUop1;
assign ALUop2_signed = ALUop2;

    always_comb begin
        Result = 32'b0;
        EQ = 0;

        case (ALUctrl)
            4'b0000: Result = ALUop1 + ALUop2; // ADD (used in addi)
            4'b0001: begin                     // SUB (used in bne)
                Result = ALUop1 - ALUop2;
                EQ = (Result == 0);
            end
            4'b0010:     Result = ALUop1 & ALUop2; //AND
            4'b0011:     Result = ALUop1 | ALUop2; //OR
            4'b0100:     Result = ALUop1 ^ ALUop2; //XOR
            4'b0101:     Result = ALUop1 << ALUop2;      // LSL (Logical Shift Left)
            4'b0110:     Result = ALUop1 >> ALUop2;      // LSR (Logical Shift Right)
            4'b0111:     Result = ALUop1_signed >>> ALUop2; // ASR (Arithmetic Shift Right)
            4'b1000:     Result = (ALUop1_signed < ALUop2_signed) ? 1 : 0; // SLT (Set Less Than - Signed comparison)
            4'b1001:     Result = (ALUop1 < ALUop2) ? 1 : 0; // SLTU (Set Less Than Unsigned)
            4'b1010:     Result = ALUop2; //B
            default: Result = 32'b0;    
        endcase
        EQ = (Result == 0'b0) ? 1 : 0;
    end
endmodule
