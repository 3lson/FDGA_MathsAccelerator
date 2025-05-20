module signextension #(
    parameter DATA_WIDTH = 32
)(
    input  logic [31:0] instr,           // I-type immediate field
    input  logic  [2:0] ImmSrc,          // Immediate source select
    output logic [DATA_WIDTH-1:0] ImmOp  // Sign-extended output
);

    always_comb begin
        case (ImmSrc)
            3'b000: ImmOp = {{20{instr[31]}}, instr[31:20]};                                        //I type instruction
            3'b001: ImmOp = {{20{instr[31]}}, instr[31:25], instr[11:7]};                           //S type instruction
            3'b010: ImmOp = {{20{instr[31]}}, instr[7], instr[30:25], instr[11:8], 1'b0};           //B type instruction
            3'b011: ImmOp = {instr[31:12], 12'b0};                                                  //U type instruction
            3'b100: ImmOp = {{12{instr[31]}}, instr[19:12], instr[20],instr[30:21],1'b0};           //J type instruction
            default: ImmOp = {{20{instr[31]}}, instr[31:20]};
        // Sign extend the immediate value
        endcase
        
        // Debug print
        //$display("Sign Extension: Input = %h, Output = %h", instr, ImmOp);
    end

endmodule
