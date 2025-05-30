module preserveFSM(
    input clk,
    input rst,
    input req,
    output preserve
);

logic req_d;

always_ff @(posedge clk) begin
    if(rst) req_d <= req;
    else req_d <= req;
end


assign preserve = req_d | req;

endmodule
