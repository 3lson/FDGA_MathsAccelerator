module program_counter #(
    parameter WIDTH = 32
)(
    input  logic                 clk,
    input  logic                 rst,
    input logic                  stall,
    input  logic  [1:0]          PCsrc,
    input  logic [31:0]          Result,
    input  logic [31:0]          ImmOp,
    output logic [WIDTH-1:0]     PCE,
    output logic [WIDTH-1:0]     PC
);

    always_ff @(posedge clk) begin
        if(!stall) begin
            if (rst) begin
                PC <= 32'h0;
            end else begin
                case(PCsrc)
                2'b00: PC <= PC + 32'd4;  // Normal increment
                2'b01: PC <= PC + ImmOp;  // Branch/Jump
                2'b10: PC <= Result;      //RET instruction
                default: PC <= PC + 32'd4;  // Normal increment
                endcase
            end
        end
    end

endmodule
