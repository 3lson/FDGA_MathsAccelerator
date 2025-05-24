module signextension #(
    parameter DATA_WIDTH = 32
)(
    /* verilator lint_off UNUSED */
    input  logic [31:0] instr,           // I-type immediate field
    input  logic  [2:0] ImmSrc,          // Immediate source select
    output logic [DATA_WIDTH-1:0] ImmOp  // Sign-extended output
);

    always_comb begin
        case (ImmSrc)
            3'b000: ImmOp = {{18{instr[27]}}, instr[27:14]};                                        //I type 
            3'b001: ImmOp = {{17{instr[28]}}, instr[28:14]};                                        //M type LOAD
            3'b010: ImmOp = {{17{instr[28]}}, instr[28:19], instr[4:0]};                            //M type STORE
            3'b011: ImmOp = {{4{instr[28]}}, instr[28:13], instr[9:0], 2'b0};                       //C type JUMP, BRANCH
            3'b100: ImmOp = {{14{instr[28]}}, instr[28:13], 2'b0};                                  //C type CALL
            default: ImmOp = {{18{instr[27]}}, instr[27:14]};
        endcase
        
    end

endmodule
