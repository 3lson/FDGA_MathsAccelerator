module cache #(
    parameter ADDR_WIDTH = 32,
              DATA_WIDTH = 32
)(
    input logic clk,
    input logic reset,
    input logic [3:0] AddrMode, // determines operation type 
    input logic [ADDR_WIDTH-1:0] A, // address
    input logic [DATA_WIDTH-1:0] WD, // write data
    output logic hit, // cache hit indicator
    output logic [DATA_WIDTH-1:0] out, // output data

    // 62-bit total

    // performance counter outputs
    output logic [31:0] total_accesses,
    output logic [31:0] total_hits,
    output logic [31:0] total_misses
);

    // Cache store structure
    typedef struct packed {
        logic valid; // Valid bit
        logic [26:0] tag; // Tag for address comparison
        logic [DATA_WIDTH-1:0] data; // Cached data
    } cache_entry;

    cache_entry cache_mem [8]; // 8 sets in cache
    logic [DATA_WIDTH-1:0] RD; // Data fetched from memory

    // Address decoding
    logic [26:0] tag; // Tag from address
    logic [2:0] set; // Set index
    logic [1:0] byte_offset;

    // Performance counters
    logic [31:0] access_counter;
    logic [31:0] hit_counter;
    logic [31:0] miss_counter;

    // Register for hit signal
    logic hit_reg;

    // Decode address
    always_comb begin
        tag = A[ADDR_WIDTH-1:5]; // Tag bits (MSBs)
        set = A[4:2]; // Set index (middle bits)
        byte_offset = A[1:0]; // Byte offset (LSBs)

    end

    // Read logic
    always_comb begin
        if (cache_mem[set].valid && cache_mem[set].tag == tag) begin
            hit_reg = 1'b1; // Cache hit
            // Determine output data based on AddrMode
            case (AddrMode)
                3'b000: out = {{24{cache_mem[set].data[7]}}, cache_mem[set].data[7:0]};   // LB (signed byte)
                3'b001: out = {{16{cache_mem[set].data[15]}}, cache_mem[set].data[15:0]};  // LH (signed halfword)
                3'b010: out = cache_mem[set].data;                                         // LW (full word)
                3'b011: out = {24'b0, cache_mem[set].data[7:0]};                           // LBU (unsigned byte)
                3'b100: out = {16'b0, cache_mem[set].data[15:0]};                          // LHU (unsigned halfword)
                default: out = 32'b0;
            endcase
        end else begin
            hit_reg = 1'b0; // Cache miss
            out = RD;   // Data from main memory
        end
    end

    // Write logic
    always_ff @(posedge clk or posedge reset) begin
        if (reset) begin
            // Reset all cache entries
            for (int i = 0; i < 8; i++) begin
                cache_mem[i].valid <= 1'b0;
                cache_mem[i].tag <= 27'b0;
                cache_mem[i].data <= 32'b0;
            end
            // Reset performance counters
            access_counter <= 0;
            hit_counter <= 0;
            miss_counter <= 0;
            hit <= 1'b0;
        end else begin
            // Update performance counters
            // Only update performance counters for load/store operations
            if (AddrMode == 4'b0010 || AddrMode == 4'b0111 || AddrMode == 4'b0000 || AddrMode == 4'b0001 || AddrMode == 4'b0101 || AddrMode == 4'b0110) begin
                access_counter <= access_counter + 1;
                if (cache_mem[set].valid && cache_mem[set].tag == tag) begin
                    hit_counter <= hit_counter + 1; // Cache hit
                end else begin
                    miss_counter <= miss_counter + 1; // Cache miss
                    // Update cache on miss (simulate memory fetch)
                    cache_mem[set].valid <= 1'b1; // Mark as valid
                    cache_mem[set].tag <= tag;    // Update tag
                    cache_mem[set].data <= RD;   // Fetch data from memory
                end
            end

            // Handle write operations
            case (AddrMode)
                4'b0101: begin // SB (store byte)
                    case (byte_offset)
                        2'b00: cache_mem[set].data[7:0]   <= WD[7:0];
                        2'b01: cache_mem[set].data[15:8]  <= WD[7:0];
                        2'b10: cache_mem[set].data[23:16] <= WD[7:0];
                        2'b11: cache_mem[set].data[31:24] <= WD[7:0];
                    endcase
                end
                4'b0110: begin // SH (store halfword)
                    case (byte_offset[1]) // Upper or lower halfword
                        1'b0: cache_mem[set].data[15:0] <= WD[15:0];
                        1'b1: cache_mem[set].data[31:16] <= WD[15:0];
                    endcase
                end
                4'b0111: begin // SW (store word)
                    cache_mem[set].data <= WD;
                end
            endcase
        end
    end

    // Synchronize hit signal
    always_ff @(posedge clk) begin
        hit <= hit_reg;
    end

    // Expose performance counters
    assign total_accesses = access_counter;
    assign total_hits = hit_counter;
    assign total_misses = miss_counter;

    // External memory interface
    data_mem data_mem_inst (
        .clk(clk),
        .AddrMode(AddrMode),
        .A(A),
        .WD(WD),
        .RD(RD)
    );

endmodule
