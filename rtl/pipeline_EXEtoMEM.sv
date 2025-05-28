module pipeline_EXEtoMEM #(
    parameter WIDTH = 32
)(
    //data path
    
    //Execute stage
    input logic     clk,
    input logic     stall,
    input logic [WIDTH-1:0] ALUResultE,
    input logic [WIDTH-1:0] WriteDataE,
    input logic [WIDTH-1:0] PCPlus4E, //for writeback (JALR mux)
    input logic [4:0]       RdE,      //for hazard detection (to determine data hazards) / writeback aswell

    
    // Memory stage
    output logic [WIDTH-1:0] ALUResultM,
    output logic [WIDTH-1:0] WriteDataM,
    output logic [WIDTH-1:0] PCPlus4M,
    output logic [4:0]       RdM,     

    //Control Unit
    
    //Execute stage
    input logic RegWriteE,          //for writeback (into regsiter)
    input logic ResultSrcE,         //for writeback (result mux either datamem or ALU output)     
    input logic   WDMEE,    //for datamem selelction
    input logic WD3SrcE,            //for writeback

    // Memory stage
    output logic RegWriteM,
    output logic ResultSrcM,
    output logic  WDMEM,
    output logic WD3SrcM,

);

always_ff @ (posedge clk) begin
    if(!stall) begin
        //Control Unit
        RegWriteM <= RegWriteE;
        ResultSrcM <= ResultSrcE;
        WDMEM <= WDMEE;
        WD3SrcM <= WD3SrcE;

        //Data path
        ALUResultM <= ALUResultE;
        WriteDataM <= WriteDataE;
        PCPlus4M <= PCPlus4E;
        RdM <= RdE;
    end
    else begin //loads NOP signals
        //Control Unit
        RegWriteM <= 1'b1;
        ResultSrcM <= 1'b0;
        WDMEM <= 1'b0;
        WD3SrcM <= 1'b0;

        //Data path
        ALUResultM <= 32'd0;
        WriteDataM <= 32'd0;
        PCPlus4M <= 32'd0; //could be a problem 
        RdM <= 5'b0;
    end
end

endmodule
