module pipeline_MEMtoWB #(
    parameter WIDTH = 32
)(
    //data path
    //Memory stage
    input logic     clk,
    input logic [WIDTH-1:0] ALUResultM,
    input logic [WIDTH-1:0] ReadDataM,
    input logic [WIDTH-1:0] PCPlus4M,
    input logic [4:0]       RdM,
    //writeback stage
    output logic [WIDTH-1:0] ALUResultW,
    output logic [WIDTH-1:0] ReadDataW,
    output logic [WIDTH-1:0] PCPlus4W,
    output logic [4:0]       RdW,

    //Control path
    
    //Memory stage
    input logic RegWriteM,
    input logic ResultSrcM,
    input logic WD3SrcM,
    input logic floatingWriteM,


    //Writeback stage
    output logic RegWriteW,
    output logic ResultSrcW, 
    output logic WD3SrcW,
    output logic floatingWriteW,


    //hazard detection
    input logic flush
);

always_ff @ (posedge clk) begin
    //Control Unit
    ResultSrcW <= ResultSrcM;
    RegWriteW <= RegWriteM;
    WD3SrcW <= WD3SrcM;
    floatingWriteW <= floatingWriteM;

    //Data path
    ALUResultW <= ALUResultM;
    ReadDataW <= ReadDataM;
    PCPlus4W <= PCPlus4M;
    RdW <= RdM;
end

endmodule
