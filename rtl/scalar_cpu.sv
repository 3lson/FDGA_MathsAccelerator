module scalar_cpu (
    input  logic       clk,
    input  logic       rst,

    output logic [31:0] pc,
    output logic [31:0] instruction,
    output logic [2:0]  opcode,
    output logic        predicate,
    output logic [4:0]  rd,
    output logic [4:0]  rs1,
    output logic [4:0]  rs2,
    output logic [13:0] imm,
    output logic [4:0]  funct5,
    output logic [2:0]  funct3,
    output logic [1:0]  funct2,

    // EXE outputs for debugging
    output logic [3:0]  alu_op_exe,
    output logic [31:0] alu_result_exe,
    output logic [4:0]  rd_exe
);

    // === Stage 1: IF signals ===
    logic [31:0] pc_r;
    logic [31:0] instruction_r;
    logic branch_taken = 1'b0;          // tied off
    logic [31:0] branch_target = 32'b0; // tied off
    logic [27:0] payload;

    // === Stage 2: DEC signals ===
    logic [2:0]  opcode_r;
    logic        predicate_r;
    logic [4:0]  rd_r, rs1_r, rs2_r;
    logic [13:0] imm_r;
    logic [4:0]  funct5_r;
    logic [2:0]  funct3_r;
    logic [1:0]  funct2_r;

    // === Control unit signals ===
    logic        is_rtype, is_itype, is_mem_load, is_mem_store, is_streamload, is_storevec;
    logic        is_x_type, is_branch, is_jump, is_call, is_ret, is_exit;
    logic [3:0]  alu_op;

    // === Pipeline registers between DEC and EXE ===
    logic [3:0]  alu_op_exe_r;
    logic [31:0] op1_exe_r, op2_exe_r;
    logic [4:0]  rd_exe_r;

    // === Pipeline registers between EXE and WB ===
    logic [31:0] alu_result_wb_r;
    logic [4:0]  rd_wb_r;
    logic        rf_we_wb_r;

    // === ALU output ===
    logic [31:0] alu_result;

    // === Register file signals ===
    logic        rf_we;
    logic [4:0]  rf_rd_addr;
    logic [31:0] rf_rd_data;
    logic [31:0] rs1_data, rs2_data;

    // === Stage 1: Instruction Fetch ===
    fetch fetch_stage (
        .clk(clk),
        .rst(rst),
        .branch_taken(branch_taken),
        .branch_target(branch_target),
        .instruction(instruction_r),
        .pc(pc_r)
    );

    // === Stage 2: Decode ===
    decode decode_stage (
        .instruction(instruction_r),
        .opcode(opcode_r),
        .predicate(predicate_r),
        .rd(rd_r),
        .rs1(rs1_r),
        .rs2(rs2_r),
        .imm(imm_r),
        .funct5(funct5_r),
        .funct3(funct3_r),
        .funct2(funct2_r),
        .payload(payload)
    );

    // === Control Unit ===
    control_unit cu (
        .opcode(opcode_r),
        .funct5(funct5_r),
        .funct3(funct3_r),
        .funct2(funct2_r),
        .is_rtype(is_rtype),
        .is_itype(is_itype),
        .is_mem_load(is_mem_load),
        .is_mem_store(is_mem_store),
        .is_streamload(is_streamload),
        .is_storevec(is_storevec),
        .is_x_type(is_x_type),
        .is_branch(is_branch),
        .is_jump(is_jump),
        .is_call(is_call),
        .is_ret(is_ret),
        .is_exit(is_exit),
        .alu_op(alu_op)
    );

    // === Register File ===
    register_file rf (
        .clk(clk),
        .we(rf_we_wb_r),
        .rs1_addr(rs1_r),
        .rs2_addr(rs2_r),
        .rd_addr(rd_wb_r),
        .rd_data_in(alu_result_wb_r),
        .rs1_data_out(rs1_data),
        .rs2_data_out(rs2_data)
    );

    // === Pipeline register DEC -> EXE ===
    always_ff @(posedge clk or posedge rst) begin
        if (rst) begin
            alu_op_exe_r <= 4'd0;
            op1_exe_r <= 32'd0;
            op2_exe_r <= 32'd0;
            rd_exe_r <= 5'd0;

            rf_we <= 1'b0;
            rf_rd_addr <= 5'd0;
            rf_rd_data <= 32'd0;
        end else begin
            alu_op_exe_r <= alu_op;

            // Use register file outputs as operands
            op1_exe_r <= rs1_data;
            op2_exe_r <= (is_itype) ? {{18{imm_r[13]}}, imm_r} : rs2_data;

            rd_exe_r <= rd_r;

            // Determine if this instruction writes to the register file
            if ((is_rtype || is_itype) && (rd_r != 5'd0)) begin
                rf_we <= 1'b1;
                rf_rd_addr <= rd_r;
                rf_rd_data <= 32'd0; // actual write data set in WB stage
            end else begin
                rf_we <= 1'b0;
                rf_rd_addr <= 5'd0;
                rf_rd_data <= 32'd0;
            end
        end
    end

    // === Pipeline register EXE -> WB ===
    always_ff @(posedge clk or posedge rst) begin
        if (rst) begin
            alu_result_wb_r <= 32'd0;
            rd_wb_r <= 5'd0;
            rf_we_wb_r <= 1'b0;
        end else begin
            alu_result_wb_r <= alu_result;
            rd_wb_r <= rd_exe_r;
            rf_we_wb_r <= rf_we; // pass write enable from EXE stage
        end
    end

    // === ALU ===
    alu alu_stage (
        .alu_op(alu_op_exe_r),
        .op1(op1_exe_r),
        .op2(op2_exe_r),
        .result(alu_result)
    );

    // === Connect outputs ===
    assign pc          = pc_r;
    assign instruction = instruction_r;
    assign opcode      = opcode_r;
    assign predicate   = predicate_r;
    assign rd          = rd_r;
    assign rs1         = rs1_r;
    assign rs2         = rs2_r;
    assign imm         = imm_r;
    assign funct5      = funct5_r;
    assign funct3      = funct3_r;
    assign funct2      = funct2_r;

    assign alu_op_exe  = alu_op_exe_r;
    assign alu_result_exe = alu_result;
    assign rd_exe      = rd_exe_r;

    // === Debug prints ===
    always_ff @(posedge clk or posedge rst) begin
        if (!rst) begin
            $display("PC: %h, Instr: %h, ALU_OP: %d, ALU Result: %d, RD: %d, RF_WE: %b", 
                     pc_r, instruction_r, alu_op_exe_r, alu_result, rd_exe_r, rf_we_wb_r);
        end
    end

endmodule
