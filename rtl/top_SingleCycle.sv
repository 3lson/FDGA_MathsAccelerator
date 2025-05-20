/* verilator lint_off SYNCASYNCNET */
module top (
    input logic clk,          // Clock signal
    input logic rst,          // Reset signal
    output logic [31:0] a0    // Contents of result (output)
);

    // Internal Signals
    logic [31:0] PC;                      // Program Counter
    logic [31:0] instr;                   // Current instruction
    logic [31:0] ImmOp;                   // Sign-extended immediate value
    logic [31:0] ALUop1, ALUop2, ALUout;  // ALU operands and result
    logic EQ;                             // Equality output from ALU
    logic [31:0] RD2, WD3;                // Register file read/write data
    logic RegWrite, ALUsrc;               // Control signals
    logic [1:0] PCsrc;                    // PC mux controls signal
    logic [2:0] ImmSrc;                   // 2-bit Immediate source signal
    logic [2:0] ALUctrl;                  // ALU control signal
    logic [2:0] AddrMode;                 // DataMemory control signal
    logic [31:0] ReadData;                // DataMemory output
    logic ResultSrc;                      // result mux control signal
    logic [31:0] Result;                  // result of output mux
    logic WD3Src;                         // control signal for Write port input to allow for jumps
    
    // Program Counter
    program_counter #(.WIDTH(32)) PC_Reg (
        .clk(clk),
        .rst(rst),
        .PCsrc(PCsrc),
        .Result(Result), //JALR PC value
        .ImmOp(ImmOp),
        .PC(PC)
    );

    // Instruction Memory (asynchronous read)
    /*rom #(
        .ADDRESS_WIDTH(32),
        .DATA_WIDTH(8)
    ) InstructionMemory (
        .addr(PC),
        .instr(instr)
    );*/

    // Instruction Memory 2.0 
    instr_mem #(
        .ADDRESS_WIDTH(32),
        .ADDRESS_REAL_WIDTH(12),
        .DATA_WIDTH(8),
        .DATA_OUT_WIDTH(32)
    ) InstructionMemory (
        .addr(PC),
        .instr(instr)
    );

    // Sign Extension Unit
    signextension #(
        .DATA_WIDTH(32)
    ) SignExtender (
        .instr(instr),
        .ImmSrc(ImmSrc),
        .ImmOp(ImmOp)
    );

    //WD3 Mux to implement Ra = PC + 4 for jump instructions
    always_comb begin
        case(WD3Src)
            1'b0: WD3 = Result;
            1'b1: WD3 = PC + 32'd4;
        endcase
    end

    // Register File with reset
    registerfile RegFile (
        .clk(clk),
        .rst(rst),
        .WE3(RegWrite),
        .AD1(instr[19:15]),
        .AD2(instr[24:20]),
        .AD3(instr[11:7]),
        .WD3(WD3),
        .RD1(ALUop1),
        .RD2(RD2),
        .a0(a0)
    );

    // ALU Operand MUX
    mux #(.DATA_WIDTH(32)) ALUOperandMux (
        .in0(RD2),
        .in1(ImmOp),
        .sel(ALUsrc),
        .out(ALUop2)
    );

    // ALU
    alu ArithmeticLogicUnit (
        .ALUop1(ALUop1),
        .ALUop2(ALUop2),
        .ALUctrl(ALUctrl),
        .Result(ALUout),
        .EQ(EQ)
    );

    // Control Unit
    controlunit controlunit (
        .instr(instr),
        .EQ(EQ),
        .RegWrite(RegWrite),
        .ALUsrc(ALUsrc),
        .ImmSrc(ImmSrc),
        .PCsrc(PCsrc),
        .ALUctrl(ALUctrl),
        .AddrMode(AddrMode),
        .ResultSrc(ResultSrc),
        .WD3Src(WD3Src)
    );
    
    //Data memory
    data_mem DataMemory (
        .clk(clk),
        .AddrMode(AddrMode),
        .A(ALUout),
        .WD(RD2),
        .RD(ReadData)
    );

    //Result mux
    always_comb begin
        case(ResultSrc)
        1'b0: Result = ALUout;
        1'b1: Result = ReadData;
        endcase
    end

endmodule
/* verilator lint_on SYNCASYNCNET */
