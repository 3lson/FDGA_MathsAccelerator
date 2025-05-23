/*

The register file will be modelled after CUDA, but instead of using special registers, we
can implement the registers here instead as fixed because it is a 
read-only hardware-provided variable.

There are multiple special variables that we can use:

1. threadIdx (tIdx): The index of the thread in the block.  (tp: x4)
2. blockIdx (bIdx): The index of the block within a grid.   (t0: x5)
3. blockDim (bDim): The dimensions of the block in the grid.(t1: x6)
UNUSED 4. gridDim (gDim): The dimensions of the grid used.
5. laneId (lId): The ID of the lane allocated.              (t2: x7)
UNUSED 6. smid (smid): The streaming multiprocessor ID 

*/

module registerfile (
    input  logic        clk,
    input  logic        WE3,     // Write enable
    input  logic [4:0]  AD1,     // Read register 1 address
    input  logic [4:0]  AD2,     // Read register 2 address
    input  logic [4:0]  AD3,     // Write register address
    input  logic [31:0] WD3,     // Write data


    output logic [31:0] RD1,     // Read data 1
    output logic [31:0] RD2,     // Read data 2
    output logic [31:0] a0

    input logic [3:0] read_thread,
    input logic [17:0] block_idx,
    input logic [3:0] rs1,
    input logic [3:0] rs2,
    input logic [3:0] write_thread,
    input logic [3:0] write_rd,
    input logic [17:0] write_data,
    input logic predicate_write_en,
    input logic predicate_in,

    output logic [17:0] reg1_out,
    output logic [17:0] reg2_out,
    output logic        predicate_out
);

    logic [31:0] registers [31:0];
    logic [31:0] special_variables [5];
    logic 

    // Initialize registers
    initial begin
        for (int i = 0; i < 32; i = i + 1)
            registers[i] = 32'h0;
    end

    // Synchronous write with reset (non-continuous)
    always_ff @(posedge clk) begin
        if (rst) begin
            // Clear all registers on reset
            for (int i = 0; i < 32; i = i + 1)
                registers[i] <= 32'h0;
        end
        else if (WE3 && (AD3 != 5'b0)) begin // Don't write to x0
            registers[AD3] <= WD3;
        end
    end

    // Asynchronous read (continuous)
    assign RD1 = (AD1 == 5'b0) ? 32'h0 : registers[AD1];
    assign RD2 = (AD2 == 5'b0) ? 32'h0 : registers[AD2];

    assign a0 = registers[10];


endmodule
