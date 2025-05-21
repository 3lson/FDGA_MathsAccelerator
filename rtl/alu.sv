module alu (
    input  logic [3:0] alu_op,
    input  logic [31:0] op1,
    input  logic [31:0] op2,
    output logic [31:0] result
);
    always_comb begin
        case (alu_op)
            4'd1:  result = op1 + op2;         // ADD/ADDI
            4'd2:  result = op1 - op2;         // SUB
            4'd3:  result = op1 * op2;         // MUL/MULI (simple)
            4'd6:  result = op1 << op2[4:0];  // SLL
            4'd7:  result = op1 >> op2[4:0];  // SRL
            4'd8:  result = ($signed(op1) < $signed(op2)) ? 32'd1 : 32'd0; // SLT
            default: result = 32'd0;
        endcase
    end
endmodule
