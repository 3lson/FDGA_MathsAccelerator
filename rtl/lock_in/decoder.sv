`default_nettype none
`timescale 1ns/1ns

`include "common.sv"

module decoder(
    input   wire                clk,
    input   wire                reset,
    input   warp_state_t        warp_state,

    input   instruction_t       instruction,

    output  reg                 decoded_reg_write_enable,
    output  reg                 decoded_mem_write_enable,
    output  reg                 decoded_mem_read_enable,
    output  reg                 decoded_branch,
    output  reg_input_mux_t     decoded_reg_input_mux,
    output  reg                 decoded_scalar_instruction,
    output  data_t              decoded_immediate,
    output  reg [4:0]           decoded_rd_address,
    output  reg [4:0]           decoded_rs1_address,
    output  reg [4:0]           decoded_rs2_address,
    output  alu_instruction_t   decoded_alu_instruction,

    output  reg                 decoded_halt
);
    // Extract fields from instruction
    wire         scalar     = instruction[6];
    wire [5:0]   inst       = instruction[5:0];
    wire [2:0]   opcode     = instruction[31:29];
    wire [4:0]   rd         = instruction[4:0];
    wire [2:0]   funct3     = instruction[14:12];
    wire [4:0]   rs1        = instruction[9:5];
    wire [3:0]   funct4     = instruction[13:10];
    wire [4:0]   rs2        = instruction[18:14];

    wire [13:0]  imm_i      = instruction[27:14];
    wire [14:0]  imm_load   = instruction[28:14];
    wire [14:0]  imm_s      = {instruction[28:19], instruction[4:0]};
    wire [17:0]  imm_b      = {instruction[28:19], instruction[13], instruction[4:0], 2'b00};
    wire [31:12] imm_u      = instruction[28:9];
    wire [27:0]  imm_j      = {instruction[28:13], instruction[9:0], 2'b00};

    always @(posedge clk) begin
        if (reset) begin
            // Set outputs to default values
            decoded_reg_write_enable <= 0;
            decoded_mem_write_enable <= 0;
            decoded_mem_read_enable <= 0;
            decoded_branch <= 0;
            decoded_reg_input_mux <= ALU_OUT;
            decoded_immediate <= {`DATA_WIDTH{1'b0}};
            decoded_rd_address <= 5'b0;
            decoded_rs1_address <= 5'b0;
            decoded_rs2_address <= 5'b0;
            decoded_alu_instruction <= ADDI;
            decoded_halt <= 0;
            decoded_scalar_instruction <= 0;
        end else if (warp_state == WARP_DECODE) begin
            // Default assignments for new decode
            decoded_reg_write_enable <= 0;
            decoded_reg_input_mux <= ALU_OUT;
            decoded_immediate <= {`DATA_WIDTH{1'b0}};
            decoded_rd_address <= 5'b0;
            decoded_rs1_address <= 5'b0;
            decoded_rs2_address <= 5'b0;
            decoded_alu_instruction <= ADDI;
            decoded_mem_read_enable <= 0;
            decoded_mem_write_enable <= 0;
            decoded_branch <= 0;
            decoded_halt <= 0;
            decoded_scalar_instruction <= 0;

            if (opcode == `OPCODE_HALT) begin
                decoded_halt <= 1;
            // end else if (opcode == `OPCODE_SX_SLT) begin
            //     decoded_scalar_instruction  <= 0; // This is a vector-scalar instruction
            //     decoded_rd_address          <= rd;
            //     decoded_rs1_address         <= rs1;
            //     decoded_rs2_address         <= rs2;
            //     decoded_reg_write_enable    <= 1;
            //     decoded_reg_input_mux       <= VECTOR_TO_SCALAR;
            //     decoded_alu_instruction     <= SLT;
            // end else if (opcode == `OPCODE_SX_SLTI) begin
            //     decoded_scalar_instruction  <= 0; // This is a vector-scalar instruction
            //     decoded_rd_address          <= rd;
            //     decoded_rs1_address         <= rs1;
            //     decoded_immediate           <= sign_extend_14(imm_i);
            //     decoded_reg_write_enable    <= 1;
            //     decoded_reg_input_mux       <= VECTOR_TO_SCALAR;
            //     decoded_alu_instruction     <= SLTI;                // not implemented yet
            end else if (opcode == `OPCODE_J) begin
                unique case (funct3) 
                    3'b000: begin
                        
                        decoded_alu_instruction     <= JAL;
                        $display("Decoding instruction 0b%32b", instruction);
                        decoded_immediate           <= sign_extend_28(imm_j);
                        
                    end

                    3'b001: begin
                        
                        // Branch instructions (e.g., BEQ, BNE)
                        decoded_rs1_address         <= rs1;
                        decoded_rs2_address         <= rs2;
                        decoded_immediate           <= sign_extend_18(imm_b);
                        decoded_branch              <= 1;
                        decoded_alu_instruction     <= BEQZ;

                    end

                    default: $error("Invalid R-type instruction with funct3 %b", funct3);
                endcase                
            end else begin
                case (opcode)
                    `OPCODE_R: begin
                        // Vector R-type instructions
                        decoded_rd_address          <= rd;
                        decoded_rs1_address         <= rs1;
                        decoded_rs2_address         <= rs2;
                        decoded_reg_write_enable    <= 1;
                        decoded_reg_input_mux       <= ALU_OUT;

                        // Determine the ALU instruction
                        unique case (funct4)
                            4'b0000: decoded_alu_instruction <= ADD;
                            4'b0001: decoded_alu_instruction <= SUB;
                            4'b0010: decoded_alu_instruction <= MUL;
                            4'b0011: decoded_alu_instruction <= DIV;
                            4'b0100: decoded_alu_instruction <= SLT;
                            4'b0110: decoded_alu_instruction <= SEQ;
                            4'b0111: decoded_alu_instruction <= SNEZ;
                            4'b1000: decoded_alu_instruction <= MIN;
                            4'b1001: decoded_alu_instruction <= ABS;
                            default: $error("Invalid R-type instruction with funct4 %b", funct4);
                        endcase
                    end
                    `OPCODE_I: begin
                        // Vector I-type instructions
                        decoded_rd_address <= rd;
                        decoded_rs1_address <= rs1;
                        decoded_reg_write_enable <= 1;
                        decoded_reg_input_mux <= ALU_OUT;
                        decoded_immediate <= sign_extend_14(imm_i);

                        unique case (funct4)
                            4'b0000: decoded_alu_instruction <= ADDI;
                            4'b0010: decoded_alu_instruction <= MULI;
                            4'b0011: decoded_alu_instruction <= DIVI;
                            4'b1010: decoded_alu_instruction <= SLLI;
                            default: $error("Invalid I-type instruction with funct4 %b", funct4);
                        endcase
                    end

                    `OPCODE_F: begin
                        // Vector I-type instructions
                        decoded_rd_address          <= rd;
                        decoded_rs1_address         <= rs1;
                        decoded_reg_write_enable    <= 1;
                        decoded_reg_input_mux       <= ALU_OUT;

                        unique case (funct4)
                            4'b0000: decoded_alu_instruction <= FADD;
                            4'b0001: decoded_alu_instruction <= FSUB;
                            4'b0010: decoded_alu_instruction <= FMUL;
                            4'b0011: decoded_alu_instruction <= FDIV;
                            4'b0100: decoded_alu_instruction <= FLT;
                            4'b0101: decoded_alu_instruction <= FNEG;
                            4'b0110: decoded_alu_instruction <= FEQ;
                            4'b0111: decoded_alu_instruction <= FMIN;
                            4'b1000: decoded_alu_instruction <= FABS;
                            4'b1001: decoded_alu_instruction <= FCVT_W_S;
                            4'b1010: decoded_alu_instruction <= FCVT_S_W;
                            default: $error("Invalid I-type instruction with funct4 %b", funct4);
                        endcase
                    end

                    `OPCODE_M: begin
                        unique case (funct4) 
                            4'b0000: begin
                                // Load instructions (e.g., LW)
                                decoded_rd_address          <= rd;
                                decoded_rs1_address         <= rs1;
                                decoded_reg_write_enable    <= 1;
                                decoded_reg_input_mux       <= LSU_OUT;
                                decoded_immediate           <= sign_extend_15(imm_load);
                                decoded_mem_read_enable     <= 1;
                                decoded_alu_instruction     <= ADDI; // For computing effective address
                            end
                            4'b0001: begin
                                decoded_rs1_address         <= rs1;
                                decoded_rs2_address         <= rs2;
                                decoded_immediate           <= sign_extend_15(imm_s);
                                decoded_mem_write_enable    <= 1;
                                decoded_alu_instruction     <= ADDI; // For computing effective address
                            end
                            default: $error("Invalid M-type instruction with funct4 %b", funct4);
                        endcase
                    end
                    `OPCODE_UP: begin
                        // LUI instruction
                        decoded_rd_address          <= rd;
                        decoded_immediate           <= {imm_u, 12'b0}; // Immediate value shifted left 12 bits
                        decoded_reg_write_enable    <= 1;
                        decoded_reg_input_mux       <= IMMEDIATE;
                    end
                    default: begin
                        // No operation or unrecognized opcode
                    end
                endcase
            end
        end
    end
endmodule
