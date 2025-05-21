module program_counter (
    input logic clk,
    input logic rst,
    input logic branch_taken, 
    input logic [31:0] branch_target,
    output logic [31:0] pc_out
);

    always_ff @(posedge clk or posedge rst) begin
        if (rst)
            pc_out <= 32'd0;
        else if (branch_taken)
            pc_out <= branch_target;
        else
            pc_out <= pc_out + 32'd4; 
    end

endmodule
