module pipeline_FECtoDEC #(
    parameter WIDTH = 32
)(
    input logic clk,
    input logic flush,
    input logic stall,
    //Fetch stage
    input logic [WIDTH-1:0] PCF,
    input logic [WIDTH-1:0] instrF,
    input logic [WIDTH-1:0] PCPlus4F,
    //Decode Stage
    output logic [WIDTH-1:0] PCD,
    output logic [WIDTH-1:0] instrD,
    output logic [WIDTH-1:0] PCPlus4D
);

always_ff @ (posedge clk) begin

    if(!stall)begin
        if(!flush)begin
            instrD <= instrF;
            PCD <= PCF;
            PCPlus4D <= PCPlus4F;
        end
        else begin
            instrD <= 32'h00000013;
            PCD <= 32'd0; //doesnt matter
            PCPlus4D <= 32'd0; //doesnt matter
        end
    end


end

endmodule
