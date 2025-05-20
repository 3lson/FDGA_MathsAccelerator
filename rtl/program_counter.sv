module program_counter #(
    parameter WIDTH = 32
)(
    input  logic                 clk,
    input  logic                 rst,
    input logic                  stall,
    input  logic [1:0]           PCsrc,
    input  logic [31:0]          ImmOp,
    input  logic [31:0]          Result,
    input  logic [WIDTH-1:0]     PCE,
    output logic [WIDTH-1:0]     PC
);

    always_ff @(posedge clk) begin
        if (!stall) begin
            if (rst) begin
                PC <= 32'h0;
            end else begin
                if (PCsrc == 2'b01) 
                    PC <= PCE + ImmOp;  // Branch/Jump
                else if(PCsrc == 2'b10)
                    PC <= Result;       // JALR (pipeline: switch this to the output of the ALU)
                else
                    PC <= PC + 32'd4;   // Normal increment
            end
        end
    end

endmodule
