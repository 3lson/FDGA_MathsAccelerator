typedef struct packed {
    logic [31:0] ALUop1,    
    logic [31:0] ALUop2,    
    logic [3:0]  ALUctrl,    
    logic [31:0] Result, 
    logic EQ
} struct_alu;