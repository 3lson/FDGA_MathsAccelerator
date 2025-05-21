module control_unit (
    input logic [2:0] opcode,
    input logic [4:0] funct5,
    input logic [2:0] funct3,
    input logic [1:0] funct2,
    output logic is_rtype,
    output logic is_itype,
    output logic is_mem_load,
    output logic is_mem_store,
    output logic is_streamload,
    output logic is_storevec,
    output logic is_x_type,
    output logic is_branch,
    output logic is_jump,
    output logic is_call,
    output logic is_ret,
    output logic is_exit,
    output logic [3:0] alu_op // Abstracted ALU op code (to drive ALU)
);

    always_comb begin
        is_rtype = 0;
        is_itype = 0;
        is_mem_load = 0;
        is_mem_store = 0;
        is_streamload = 0;
        is_storevec = 0;
        is_x_type = 0;
        is_branch = 0;
        is_jump = 0;
        is_call = 0;
        is_ret = 0;
        is_exit = 0;
        alu_op = 4'b0000;

        case (opcode)
            3'b000: begin // R-type
                is_rtype = 1;
                case (funct5)
                    5'b00000: alu_op = 4'd1; // ADD
                    5'b00001: alu_op = 4'd2; // SUB
                    5'b00010: alu_op = 4'd3; // MUL
                    5'b00011: alu_op = 4'd4; // MAC
                    5'b00100: alu_op = 4'd5; // DIV
                    5'b01010: alu_op = 4'd6; // SLL
                    5'b01011: alu_op = 4'd7; // SRL
                    5'b11100: alu_op = 4'd8; // SLT
                    default:  alu_op = 4'd0;
                endcase
            end
            3'b001: begin // I-type
                is_itype = 1;
                case (funct5)
                    5'b00000: alu_op = 4'd1; // ADDI
                    5'b00010: alu_op = 4'd3; // MULI
                    5'b00100: alu_op = 4'd5; // DIVI
                    5'b00110: alu_op = 4'd9; // CLAMP
                    default:  alu_op = 4'd0;
                endcase
            end
            3'b100: begin // Memory
                case (funct3)
                    3'b000: is_mem_load = 1;
                    3'b001: is_mem_store = 1;
                    3'b010: is_streamload = 1;
                    3'b011: is_storevec = 1;
                    default: ;
                endcase
            end
            3'b101: is_x_type = 1; // App-specific
            3'b111: begin // Control Flow 
                case (funct3)
                    3'b000: is_jump = 1;
                    3'b001: is_branch = 1;
                    3'b010: is_call = 1;
                    3'b011: is_ret = 1;
                    3'b111: is_exit = 1;
                    default: ;
                endcase
            end
            default: ;
        endcase
    end

endmodule
