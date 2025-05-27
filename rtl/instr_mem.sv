module instr_mem #(
    parameter   ADDRESS_WIDTH = 32,
                ADDRESS_REAL_WIDTH = 12,
                DATA_WIDTH = 8,
                DATA_OUT_WIDTH = 32
) (
    input logic [ADDRESS_WIDTH-1:0] addr,
    output logic [DATA_OUT_WIDTH-1:0] instr
);

logic [DATA_WIDTH-1:0] array [2**ADDRESS_REAL_WIDTH-1:0];

initial begin
    $display("Loading program into instruction memory...");
    $readmemh("../rtl/program.hex", array);
end

// Put the 4 bytes back together to a 32-bit word
assign instr = {array[addr+3], array[addr+2], array[addr+1], array[addr]};

endmodule
