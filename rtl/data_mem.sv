module data_mem #(
        parameter   DATA_WIDTH = 32, 
                    ADDR_WIDTH = 32,
                    ADDR_REAL_WIDTH = 20, //We chose to have 2^20 memory locations
                    MEM_WIDTH = 8

)(
        input  logic                    clk,
        input  logic                    WDME,
        input  logic [ADDR_WIDTH-1:0]   A, // address
        input  logic [DATA_WIDTH-1:0]   WD, // write data
        output logic [DATA_WIDTH-1:0]   RD // read data
);

    logic [MEM_WIDTH-1:0] array [2**ADDR_REAL_WIDTH-1:0];
    logic [ADDR_WIDTH-1:0] temp;

    //sets all bytes in memory to 8'b0
    initial begin
        for(int i = 0; i < (1 << ADDR_REAL_WIDTH) - 1; i++) begin
            array[i] =   8'b0;
        end
        

        //loads data.hex into array of data memory
        //commented for the purpose of adding more instr
        $display("Loading program into data memory...");
        //s$readmemh("../tb/data/gaussian.mem", array, 20'h10000, 20'h1FFFF);

    end

    //load instructions
    always_comb begin
        // load word
        temp = {array[A+3], array[A+2], array[A+1], array[A]};
    end


    //store instructions 
    always_ff @(posedge clk) begin
        if(WDME == 1'b1)begin
            array[A] <= WD[7:0]; // stores the least significant byte
            array[A+1] <= WD[15:8];
            array[A+2] <= WD[23:16];
            array[A+3] <= WD[31:24]; // stores the most significant byte
        end
    end

    assign RD = temp;
    
endmodule
