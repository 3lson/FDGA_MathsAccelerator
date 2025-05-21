module decode (
    input  logic [31:0] instruction,
    output logic [2:0] opcode,
    output logic       predicate,
    output logic [4:0] rd,
    output logic [4:0] rs1,
    output logic [4:0] rs2,
    output logic [13:0] imm,
    output logic [4:0] funct5,
    output logic [2:0] funct3,
    output logic [1:0] funct2,
    output logic [27:0] payload
);

    assign opcode    = instruction[31:29];
    assign predicate = instruction[28];
    assign payload   = instruction[27:0];

    // Default outputs
    assign rd     = instruction[4:0];
    assign rs1    = instruction[9:5];
    assign funct5 = instruction[14:10];
    assign rs2    = instruction[19:15];
    assign imm    = instruction[28:15];
    assign funct3 = instruction[27:25];
    assign funct2 = instruction[28:27]; // for X-type

endmodule
