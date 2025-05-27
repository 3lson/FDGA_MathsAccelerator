/*

The register file will be modelled after CUDA, but instead of using special registers, we
can implement the registers here instead as fixed because it is a 
read-only hardware-provided variable.

There are multiple special variables that we can use:

1. threadIdx (tIdx): The index of the thread in the block.  (tp: x28)
2. blockIdx (bIdx): The index of the block within a grid.   (t0: x29)
3. blockDim (bDim): The dimensions of the block in the grid.(t1: x30)
UNUSED 4. gridDim (gDim): The dimensions of the grid used.
5. laneId (lId): The ID of the lane allocated.              (t2: x31)
UNUSED 6. smid (smid): The streaming multiprocessor ID 

*/

module registerfile (
    input   logic           clk,
    input   logic           WE3,            // Write enable
    input   logic [4:0]     AD1,            // Read register 1 address
    input   logic [4:0]     AD2,            // Read register 2 address
    input   logic [4:0]     AD3,            // Write register address
    input   logic [31:0]    WD3,            // Write data
    input   logic [4:0]     thread_read,
    input   logic [4:0]     thread_write,
    input   logic [31:0]    bIdx,

    output  logic [31:0]    RD1,            // Read data 1
    output  logic [31:0]    RD2,            // Read data 2
    output  logic [31:0]    a0

    // Save for predicates
    // input logic          predicate_en,
    // input logic          predicate_read,
    // output logic         predicate_write
);
    // Is registers[31:0] still necessary?
    /* verilator lint_off UNUSED */
    logic [31:0] registers [31:0];
    /* verilator lint_on UNUSED */
    logic [31:0] special_variables [4];
    logic [31:0] threading_registers [16][32]; // check how many registers we can actually use
    // remember to add thread predication for optimisation

    initial begin
        integer i, j;
        for (i = 0; i < 16; i = i + 1) begin
            for (j = 0; j < 32; j = j + 1) begin
                threading_registers[i][j] = 32'd0;
            end
        end
    end

    // assigning special variables, set thread id as 0 for now
    // set bDim as 1 because only 1 block necessary
    // laneId put as placeholder for now
    always_comb begin
        special_variables[0] = 32'd0;       // x28: tIdx
        special_variables[1] = bIdx;        // x29: bIdx
        special_variables[2] = 32'd1;       // x30: bDim
        special_variables[3] = 32'd0;       // x31: lId
    end

    // making conditions for special variables
    // if placed at r28-31, special variables are copied over and saved
    // if not, read regularly
    always_comb begin
        if (AD1 < 5'd28) begin  
            RD1 = threading_registers[thread_read][AD1]; 
        end else begin
            RD1 = special_variables[AD1-28];
        end

        if (AD2 < 5'd28) begin
            RD2 = threading_registers[thread_read][AD2];
        end else begin
            RD2 = special_variables[AD2-28];
        end
    end

    assign a0 = threading_registers[thread_read][10];

    // Synchronous write (non-continuous)
    always_ff @(posedge clk) begin
        if (WE3 && AD3 != 0 && AD3 < 28) begin // Don't write to x0
            threading_registers[thread_write][AD3] <= WD3;
        end
    end

endmodule