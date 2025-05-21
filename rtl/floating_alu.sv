// =================================================
// An ALU that computes floating point instructions
// Currently has ADD
// =================================================
module floating_alu (
    input  logic [3:0] alu_op,
    input  logic [15:0] op1,
    input  logic [15:0] op2,
    output logic [15:0] result
);

logic op1_sign_bit;
logic op2_sign_bit;
logic [4:0] op1_biased_exponent;
logic [4:0] op2_biased_exponent;
logic [10:0] op1_significand;
logic [10:0] op2_significand;
logic [4:0] exp_diff;
logic [11:0] sum, mantissa_shift_op1, mantissa_shift_op2;
logic [4:0] result_exp;
logic result_sign_bit;
logic guard_bit, round_bit, round_up;
logic [10:0] rounded_sum;

always_comb begin
    
    result = 16'b0;
    op1_sign_bit = op1[15];
    // If SUB, op1 - op2 => op1 + (-op2), so sign bit can be inversed
    op2_sign_bit = (alu_op == 4'd2) ? ~op2[15] : op2[15];
    op1_biased_exponent = op1[14:10];
    op2_biased_exponent = op2[14:10];
    op1_significand = {1'b1, op1[9:0]};
    op2_significand = {1'b1, op2[9:0]};
    case (alu_op)
        4'd1, 4'd2:  begin
            if (op1_biased_exponent > op2_biased_exponent) begin
                exp_diff = op1_biased_exponent - op2_biased_exponent;
                mantissa_shift_op1 = {1'b0, op1_significand};
                mantissa_shift_op2 = {1'b0, op2_significand} >> exp_diff;
                result_exp = op1_biased_exponent;
                result_sign_bit = op1_sign_bit;
            end else begin
                exp_diff = op1_biased_exponent - op2_biased_exponent;
                mantissa_shift_op1 = {1'b0, op1_significand} >> exp_diff;
                mantissa_shift_op2 = {1'b0, op2_significand};
                result_exp = op2_biased_exponent;
                result_sign_bit = op2_sign_bit;
            end

            // Perform addition (same sign only in this version)
            if (op1_sign_bit == op2_sign_bit) begin
                sum = mantissa_shift_op1 + mantissa_shift_op2;

                // Normalize result (if carry)
                if (sum[11]) begin
                    result_exp = result_exp + 1;
                    sum = sum >> 1;
                end

                guard_bit  = sum[1];           
                round_bit  = sum[0];           

                // Round to nearest even
                round_up = guard_bit & (round_bit | sum[2]);

                if (round_up) begin
                    rounded_sum = sum[10:0] + 1;
                    // Check if rounding caused carry-out
                    if (rounded_sum[10]) begin
                        result = {result_sign_bit, result_exp, rounded_sum[9:0]};
                    end else begin
                        // Overflow — need to renormalize
                        result_exp = result_exp + 1;
                        result = {result_sign_bit, result_exp, rounded_sum[10:1]};
                    end
                end else begin
                    // No rounding
                    result = {result_sign_bit, result_exp, sum[9:0]};
                end
            end
            else begin
                // Subtraction case: opposite signs

                // Subtract in correct order
                if (mantissa_shift_op1 >= mantissa_shift_op2) begin
                    sum = mantissa_shift_op1 - mantissa_shift_op2;
                    result_sign_bit = op1_sign_bit;
                end else begin
                    sum = mantissa_shift_op2 - mantissa_shift_op1;
                    result_sign_bit = op2_sign_bit;
                end

                // Normalize result if leading bits are zero
                if (sum == 0) begin
                    result = 16'd0; // result is zero
                end else begin
                    int shift_count;
                    shift_count = 0;
                    while (sum[10] == 0 && result_exp > 0) begin
                        sum = sum << 1;
                        result_exp = result_exp - 1;
                        shift_count = shift_count + 1;
                    end

                    result = {result_sign_bit, result_exp, sum[9:0]};
                end
            end
        end
        default: result = 16'd0;
    endcase
end
endmodule
