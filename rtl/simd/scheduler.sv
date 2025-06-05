module scheduler #(
    parameter NUM_LANES = 16,
    parameter NUM_THREADS = 8,
    parameter MAX_BLOCKS = 16,
    // parameter TIME_QUANTUM = 10  // Clock cycles per thread before switching
)(
    input   logic           clk,
    input   logic           reset,
    
    // Input instruction interface
    input  logic   [3:0]    tIdx_in,
    input  logic   [31:0]   bIdx_in,
    input  logic   [4:0]    FUNCT4_in,
    input  logic   [31:0]   IMM_in,
    input  logic   [4:0]    rs1_in,
    input  logic   [4:0]    rs2_in,
    input  logic   [4:0]    rd_in,
    input  logic            is_int_in,
    input  logic            is_float_in,
    input  logic            WE3_in,
    input  logic            pred_en_in,
    input  logic            pred_read_in,
    input   logic           stall_in,
    input   logic           pred_write_in,
    input   logic           instr_valid,
    
    // Lane interfaces
    output  logic   [3:0]   tIdx        [NUM_LANES],
    output  logic   [31:0]  bIdx        [NUM_LANES],
    output  logic   [4:0]   FUNCT4      [NUM_LANES],
    output  logic   [31:0]  IMM         [NUM_LANES],
    output  logic   [4:0]   rs1         [NUM_LANES],
    output  logic   [4:0]   rs2         [NUM_LANES],
    output  logic   [4:0]   rd          [NUM_LANES],
    output  logic           is_int      [NUM_LANES],
    output  logic           is_float    [NUM_LANES],
    output  logic           WE3         [NUM_LANES],
    output  logic           pred_en     [NUM_LANES],
    output  logic           pred_read   [NUM_LANES],
    
    output  logic           ready_for_instr
);

typedef enum logic [1:0] {
    THREAD_INACTIVE,
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_STALLED
} thread_state_t;

// Thread control structures
thread_state_t thread_state[NUM_THREADS];
logic [31:0] thread_block[NUM_THREADS];
logic [NUM_LANES-1:0] thread_lane_map[NUM_THREADS];
logic [31:0] thread_pc[NUM_THREADS];
// logic [31:0] thread_time_counter[NUM_THREADS]; // Time quantum counters

// Scheduling
logic [3:0] schedule_pointer;
logic [NUM_LANES-1:0] lane_available;

// Determine available lanes
always_comb begin
    lane_available = {NUM_LANES{1'b1}};
    for (int t = 0; t < NUM_THREADS; t++) begin
        if (thread_state[t] == THREAD_RUNNING || thread_state[t] == THREAD_STALLED) begin
            lane_available &= ~thread_lane_map[t];
        end
    end
end

// Thread state management
always_ff @(posedge clk or posedge reset) begin
    if (reset) begin
        // Reset all threads
        for (int i = 0; i < NUM_THREADS; i++) begin
            thread_state[i] <= THREAD_INACTIVE;
            thread_lane_map[i] <= '0;
            thread_block[i] <= '0;
            thread_time_counter[i] <= '0;
        end
        
        // Reset lane outputs
        for (int l = 0; l < NUM_LANES; l++) begin
            tIdx[l] <= '0;
            bIdx[l] <= '0;
            FUNCT4[l] <= '0;
            IMM[l] <= '0;
            rs1[l] <= '0;
            rs2[l] <= '0;
            rd[l] <= '0;
            is_int[l] <= '0;
            is_float[l] <= '0;
            WE3[l] <= '0;
            pred_en[l] <= '0;
            pred_read[l] <= '0;
        end
        
        schedule_pointer <= 0;
    end else begin
        // Update time counters for running threads
        for (int t = 0; t < NUM_THREADS; t++) begin
            if (thread_state[t] == THREAD_RUNNING) begin
                thread_time_counter[t] <= thread_time_counter[t] + 1;
            end
        end

        // // Check for time quantum expiration or stalls? Worth doing?
        // for (int t = 0; t < NUM_THREADS; t++) begin
        //     if (thread_state[t] == THREAD_RUNNING) begin
        //         // Check for time quantum expiration or stall
        //         if ((thread_time_counter[t] >= TIME_QUANTUM) || 
        //             ((thread_lane_map[t] & stall_in) != 0)) begin
        //             // Time quantum expired or thread stalled
        //             thread_state[t] <= ((thread_lane_map[t] & stall_in) != 0) ? 
        //                               THREAD_STALLED : THREAD_READY;
        //             thread_lane_map[t] <= '0;
        //             thread_time_counter[t] <= '0;
        //         end
        //     end else if (thread_state[t] == THREAD_STALLED) begin
        //         // Check if stall condition cleared
        //         if ((thread_lane_map[t] & stall_in) == 0) begin
        //             thread_state[t] <= THREAD_READY;
        //             thread_lane_map[t] <= '0;
        //         end
        //     end
        // end

        // Dispatch new threads to available lanes
        if (instr_valid) begin
            // Update thread block for new instruction
            thread_block[tIdx_in] <= bIdx_in;
            
            // Mark thread as ready if it's inactive
            if (thread_state[tIdx_in] == THREAD_INACTIVE) begin
                thread_state[tIdx_in] <= THREAD_READY;
            end
        end

        // Assign ready threads to available lanes (coarse-grain)
        for (int l = 0; l < NUM_LANES; l++) begin
            if (lane_available[l]) begin
                // Find next ready thread (round-robin)
                for (int i = 0; i < NUM_THREADS; i++) begin
                    int t = (schedule_pointer + i) % NUM_THREADS;
                    
                    if (thread_state[t] == THREAD_READY) begin
                        // Assign to lane
                        thread_lane_map[t][l] <= 1'b1;
                        thread_state[t] <= THREAD_RUNNING;
                        
                        // Connect thread context to lane
                        tIdx[l] <= t;
                        bIdx[l] <= thread_block[t];
                        FUNCT4[l] <= FUNCT4_in;
                        IMM[l] <= IMM_in;
                        rs1[l] <= rs1_in;
                        rs2[l] <= rs2_in;
                        rd[l] <= rd_in;
                        is_int[l] <= is_int_in;
                        is_float[l] <= is_float_in;
                        WE3[l] <= WE3_in;
                        pred_en[l] <= pred_en_in;
                        pred_read[l] <= pred_read_in;
                        
                        schedule_pointer <= (schedule_pointer + 1) % NUM_THREADS;
                        break;
                    end
                end
            end
        end
    end
end

// Ready signal when we can accept new instructions
assign ready_for_instr = (instr_valid ? 
                         (thread_state[tIdx_in] != THREAD_RUNNING && 
                          thread_state[tIdx_in] != THREAD_STALLED) : 
                         1'b1);

endmodule