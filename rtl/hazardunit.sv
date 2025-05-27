module hazardunit (
    input logic [4:0] Rs1E,
    input logic [4:0] Rs2E,
    input logic [4:0] RdM,
    input logic [4:0] RdW,
    input logic        WDMEM,
    input logic  branchE,
    input logic [1:0] JumpE,
    input logic EQ,

    output logic [1:0] forwardAE,
    output logic [1:0] forwardBE,
    output logic stall,
    output logic flush
);

    logic BranchCondE;

    //Forwarding

    //Rs1 MUX forwarding
    always_comb begin
        if((Rs1E == RdW) && (Rs1E != 5'd0))begin
            if(Rs1E == RdM)begin
                forwardAE = 2'b01; // MEM TO EXE prioritisation
            end
            else begin
                forwardAE = 2'b10; // WR TO EXE
            end
        end
        else begin
            if((Rs1E == RdM) && (Rs1E != 5'd0))begin
                forwardAE = 2'b01; // MEM TO EXE
            end
            else begin
                forwardAE = 2'b00; //Rd1E -> SrcAE 
            end
        end
    end

    //Rs2 MUX forwarding
    always_comb begin
        if((Rs2E == RdW) && (Rs2E != 5'd0))begin
            if(Rs2E == RdM)begin
                forwardBE = 2'b01; // MEM TO EXE prioritisation
            end
            else begin
                forwardBE = 2'b10; // WR TO EXE
            end
        end
        else begin
            if((Rs2E == RdM) && (Rs2E != 5'd0))begin
                forwardBE = 2'b01; // MEM TO EXE
            end
            else begin
                forwardBE = 2'b00; //Rd2E -> SrcBE (assuming no immediate)
            end
        end
    end

    always_comb begin
        //Stall
        if ((WDMEM == 1'b1) && ((RdM == Rs1E) || (RdM == Rs2E))) begin
            stall = 1'b1;
        end
        else begin 
            stall = 1'b0;
        end
    end

    //flush conditional logic
    always_comb begin
        if(JumpE[1] == 1'b1) begin
            flush = 1'b1;
        end
        else begin
            if((branchE == 1'b1) && (EQ == 1'b1)) begin
                flush = 1'b1;
            end
            else begin
                flush = 1'b0;
            end
        end
    end


endmodule
