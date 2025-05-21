module register_file (
    input logic clk,
    input logic we,
    input logic [4:0] rs1_addr,
    input logic [4:0] rs2_addr,
    input logic [4:0] rd_addr,
    input logic [31:0] rd_data_in,
    output logic [31:0] rs1_data_out,
    output logic [31:0] rs2_data_out
);

    logic [31:0] regs [31:0];

    assign rs1_data_out = regs[rs1_addr];
    assign rs2_data_out = regs[rs2_addr];

    always_ff @(posedge clk) begin
        if (we && rd_addr != 5'd0)
            regs[rd_addr] <= rd_data_in;
    end

endmodule
