typedef struct packed {
    logic [3:0] thread_num;
    logic [31:0] result;
    logic [3:0] rd;
} int_result_reg;