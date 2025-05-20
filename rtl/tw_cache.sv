module tw_cache #( 
    parameter ADDR_WIDTH = 32,
              DATA_WIDTH = 32,
              NUM_SETS = 4,    // Reduced from 8 to 4 sets since we now have 2 ways
              NUM_WAYS = 2     // Two-way set associative
)(
    input logic clk,
    input logic reset,
    input logic [3:0] AddrMode,
    input logic [ADDR_WIDTH-1:0] A,
    input logic [DATA_WIDTH-1:0] WD,
    output logic hit,
    output logic [DATA_WIDTH-1:0] out,
    // Performance counter outputs
    output logic [31:0] total_accesses,
    output logic [31:0] total_hits,
    output logic [31:0] total_misses
);
    // Cache entry structure
    typedef struct packed {
        logic valid;
        logic [27:0] tag; 
        logic [DATA_WIDTH-1:0] data;
    } cache_entry;

    // Main cache storage: [sets][ways]
    cache_entry cache [NUM_SETS][NUM_WAYS];
    
    // LRU bits - one per set
    logic [NUM_SETS-1:0] lru_bits;
    
    // Internal signals
    logic [27:0] tag;
    logic [1:0] set;  // 2 bits for 4 sets
    logic [1:0] byte_offset;
    logic [NUM_WAYS-1:0] way_hits;
    logic [0:0] hit_way;

    // Performance counters
    logic [31:0] access_counter;
    logic [31:0] hit_counter;
    logic [31:0] miss_counter;

    logic lru_way; // Declare lru_way as logic

    // Extract address fields
    always_comb begin
        tag = A[ADDR_WIDTH-1:4];
        set = A[3:2];
        byte_offset = A[1:0];
    end

    // Read Logic
    always_comb begin
        hit = 1'b0;
        out = {DATA_WIDTH{1'b0}};
        way_hits = '0;
        hit_way = '0;

        for (int i = 0; i < NUM_WAYS; i++) begin
            if (cache[set][i].valid && cache[set][i].tag == tag) begin
                way_hits[i] = 1'b1;
                hit = 1'b1;
                hit_way = i[0:0];
                out = cache[set][i].data;
            end
        end
    end

    // Write Logic
    always_ff @(posedge clk or posedge reset) begin
        if (reset) begin
            // ... [reset logic remains the same]
        end else begin
            if (AddrMode == 4'b0111) begin  // Write operation
                access_counter <= access_counter + 1;
                if (hit) begin
                    hit_counter <= hit_counter + 1;
                    cache[set][hit_way].data <= WD;
                    lru_bits[set] <= ~hit_way;
                end else begin
                    miss_counter <= miss_counter + 1;
                    lru_way = lru_bits[set];
                    cache[set][lru_way].data <= WD;
                    cache[set][lru_way].tag <= tag;
                    cache[set][lru_way].valid <= 1'b1;
                    lru_bits[set] <= ~lru_way;
                end
            end else if (AddrMode == 4'b0010 || AddrMode == 4'b0000 || AddrMode == 4'b0001 || AddrMode == 4'b0101 || AddrMode == 4'b0110) begin  // Read operations
                access_counter <= access_counter + 1;
                if (hit) begin
                    hit_counter <= hit_counter + 1;
                    lru_bits[set] <= ~hit_way;
                end else begin
                    miss_counter <= miss_counter + 1;
                    // For a read miss, you might want to implement a memory fetch here
                    // For now, we'll just update the LRU bit
                    lru_bits[set] <= ~lru_bits[set];
                end
            end
        end
    end

    assign total_accesses = access_counter;
    assign total_hits = hit_counter;
    assign total_misses = miss_counter;

endmodule
