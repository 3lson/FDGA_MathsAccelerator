// Defining opcode and functs

// Opcode
`define Rtype 3'b000
`define Itype 3'b001
`define Mtype 3'b100
`define Ctype 3'b111
`define Ptype 3'b011 
`define Xtype 3'b101
`define Ftype 3'b010 

// ALUOps
`define ALU_ADD 4'b0000 
`define ALU_SUB 4'b0001 
`define ALU_MUL 4'b0010  
`define ALU_DIV 4'b0011 
`define ALU_SLT 4'b0100 
`define ALU_SGT 4'b0101
`define ALU_SEQ 4'b0110 
`define ALU_SNEZ 4'b0111
`define ALU_MIN 4'b1000
`define ALU_ABS 4'b1001
`define ALU_SLLI 4'b1010


//Floating Point ALUOps
`define FALU_ADD 4'b0000
`define FALU_SUB 4'b0001 
`define FALU_MUL 4'b0010
`define FALU_DIV 4'b0011 
`define FALU_SLT 4'b0100 
`define FALU_NEG 4'b0101
`define FALU_EQ 4'b0110
`define FALU_MIN 4'b0111
`define FALU_ABS 4'b1000
`define FALU_FCVT_WS 4'b1001
`define FALU_FCVT_SW 4'b1010


