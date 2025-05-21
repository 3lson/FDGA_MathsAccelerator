module fetch (
    input logic clk,
    input logic rst,
    input logic branch_taken,
    input logic [31:0] branch_target,
    output logic [31:0] instruction,
    output logic [31:0] pc
);

    logic [31:0] pc_internal;

    program_counter pc_unit(
        .clk(clk),
        .rst(rst),
        .branch_taken(branch_taken),
        .branch_target(branch_target),
        .pc_out(pc_internal)
    );

    instruction_memory imem(
        .address(pc_internal),
        .instruction(instruction)
    );

    assign pc = pc_internal;

endmodule
