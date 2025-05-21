module instruction_memory (
    input  logic [31:0] address,
    output logic [31:0] instruction
);
    logic [31:0] instr_mem [0:255];

    initial begin
        $display("Loading instruction memory...");
        $readmemh("obj_dir/instr_mem.mem", instr_mem);
        $display("%h %h %h", instr_mem[0], instr_mem[1], instr_mem[2]);
    end

    //Word-aligned access (address >> 2)
    assign instruction = instr_mem[address[9:2]];
endmodule
