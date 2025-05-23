// =================================================
// A 32-bit Floating Point ALU
// with ADD, SUB, MUL, DIV, EQ, NEQ, ABS and SLT
// =================================================
module floating_alu (
    input  logic [3:0]  alu_op,
    input  logic [31:0] op1,
    input  logic [31:0] op2,
    output logic [31:0] result,
    output logic        cmp
);

// 1 sign bit, 8 exponent bits and 23 mantissa bits
logic op1_sign_bit, op2_sign_bit;
logic [7:0] op1_biased_exponent, op2_biased_exponent;
// 1 implicit leading 1 + 23 fraction bits
logic [23:0] op1_significand, op2_significand;

logic [7:0] exp_diff, result_exp;
logic [24:0] sum, mantissa_shift_op1, mantissa_shift_op2;
logic result_sign_bit;
logic guard_bit, round_bit, round_up;
logic [23:0] rounded_sum;
/* verilator lint_off UNUSED */
logic [47:0] product, quotient, numerator, denominator;
int shift_count;
logic sticky_bit;
/* verilator lint_on UNUSED */

always_comb begin
    result = 32'd0;
    cmp = 0;

    op1_sign_bit = op1[31];
    // If instruction is sub, invert the first bit to make it a negative number to use
    // ADD instead
    op2_sign_bit = (alu_op == 4'd2) ? ~op2[31] : op2[31];
    op1_biased_exponent = op1[30:23];
    op2_biased_exponent = op2[30:23];
    op1_significand = {1'b1, op1[22:0]};
    op2_significand = {1'b1, op2[22:0]};

    exp_diff = 0;
    mantissa_shift_op1 = 0;
    mantissa_shift_op2 = 0;
    sum = 0;
    round_up = 0;
    rounded_sum = 0;
    product = 0;
    quotient = 0;
    result_exp = 0;
    result_sign_bit = 0;
    guard_bit = 0;
    round_bit = 0;
    shift_count = 0;
    numerator = 0;
    denominator = 0;
    sticky_bit = 0;
    shift_count = 0;

    case (alu_op)
        4'd1, 4'd2: begin
            if (op1_biased_exponent > op2_biased_exponent) begin
                exp_diff = op1_biased_exponent - op2_biased_exponent;
                // add 1'b0 to align mantissas after right shift
                mantissa_shift_op1 = {1'b0, op1_significand};
                mantissa_shift_op2 = {1'b0, op2_significand} >> exp_diff;
                result_exp = op1_biased_exponent;
                result_sign_bit = op1_sign_bit;
            end else begin
                exp_diff = op2_biased_exponent - op1_biased_exponent;
                mantissa_shift_op1 = {1'b0, op1_significand} >> exp_diff;
                mantissa_shift_op2 = {1'b0, op2_significand};
                result_exp = op2_biased_exponent;
                result_sign_bit = op2_sign_bit;
            end

            if (op1_sign_bit == op2_sign_bit) begin
                // Addition part
                sum = mantissa_shift_op1 + mantissa_shift_op2;

                // Normalisation
                if (sum[24]) begin // overflow so shift right
                    sum = sum >> 1;
                    result_exp = result_exp + 1;
                end else begin
                    while (sum[23] == 0 && result_exp > 0) begin
                        sum = sum << 1;
                        result_exp = result_exp - 1;
                    end
                end

                // Rounding to nearest even - IEEE standard 
                guard_bit = sum[1];
                round_bit = sum[0];
                round_up = guard_bit & (round_bit | sum[2]);

                if (round_up) begin
                    rounded_sum = sum[23:0] + 1;
                    if (rounded_sum[23]) begin
                        result = {result_sign_bit, result_exp, rounded_sum[22:0]};
                    end else begin
                        result_exp = result_exp + 1;
                        result = {result_sign_bit, result_exp, rounded_sum[23:1]};
                    end
                end else begin
                    result = {result_sign_bit, result_exp, sum[22:0]};
                end

            end else begin
                // Subtraction part
                if (mantissa_shift_op1 >= mantissa_shift_op2) begin
                    sum = mantissa_shift_op1 - mantissa_shift_op2;
                    result_sign_bit = op1_sign_bit;
                end else begin
                    sum = mantissa_shift_op2 - mantissa_shift_op1;
                    result_sign_bit = op2_sign_bit;
                end

                if (sum == 0) begin
                    result = 32'd0;
                end else begin
                    while (sum[23] == 0 && result_exp > 0) begin
                        sum = sum << 1;
                        result_exp = result_exp - 1;
                    end

                    guard_bit = sum[1];
                    round_bit = sum[0];
                    round_up = guard_bit & (round_bit | sum[2]);

                    if (round_up) begin
                        rounded_sum = sum[23:0] + 1;
                        if (rounded_sum[23]) begin
                            result = {result_sign_bit, result_exp, rounded_sum[22:0]};
                        end else begin
                            result_exp = result_exp + 1;
                            result = {result_sign_bit, result_exp, rounded_sum[23:1]};
                        end
                    end else begin
                        result = {result_sign_bit, result_exp, sum[22:0]};
                    end
                end
            end
        end

        4'd3: begin
            product = op1_significand * op2_significand;
            if (product[47]) begin
                result_exp = op1_biased_exponent + op2_biased_exponent - 127 + 1;
                rounded_sum = product[47:24];
                guard_bit = product[23];
                round_bit = product[22];
                round_up = guard_bit & (round_bit | product[21]);
            end else begin
                result_exp = op1_biased_exponent + op2_biased_exponent - 127;
                rounded_sum = product[46:23];
                guard_bit = product[22];
                round_bit = product[21];
                round_up = guard_bit & (round_bit | product[20]);
            end

            result_sign_bit = op1_sign_bit ^ op2_sign_bit;
            if (round_up) begin
                rounded_sum = rounded_sum + 1;
                if (rounded_sum[23]) begin
                    result = {result_sign_bit, result_exp, rounded_sum[22:0]};
                end else begin
                    result_exp = result_exp + 1;
                    result = {result_sign_bit, result_exp, rounded_sum[23:1]};
                end
            end else begin
                result = {result_sign_bit, result_exp, rounded_sum[22:0]};
            end
        end

        4'd4: begin
            numerator = {op1_significand, 24'd0};
            denominator = {24'd0, op2_significand};
            quotient = numerator / denominator;
            result_exp = op1_biased_exponent - op2_biased_exponent + 127;
            result_sign_bit = op1_sign_bit ^ op2_sign_bit;
            result = {result_sign_bit, result_exp, quotient[22:0]};
            // if (quotient[47]) begin
            //     quotient = quotient >> 1;
            //     //result_exp = result_exp + 1;
            // end else begin
            //     shift_count = 0;
            //     while (!quotient[46] && result_exp > 0 && shift_count < 24) begin
            //         quotient = quotient >> 1;
            //         result_exp = result_exp - 1;
            //         shift_count = shift_count + 1;
            //     end
            // end
            // sticky_bit = |quotient[20:0];
            // guard_bit = quotient[22];
            // round_bit = quotient[21];

            // round_up = guard_bit & (round_bit | sticky_bit);

            // rounded_sum = quotient[46:23];
            // if (round_up) begin
            //     rounded_sum = rounded_sum + 1;
            //     if (rounded_sum == 24'h100000) begin
            //         rounded_sum = rounded_sum >> 1;
            //         result_exp = result_exp + 1;
            //     end
            // end
            // if (result_exp >= 255) begin
            //     // Overflow → infinity
            //     result = {result_sign_bit, 8'hFF, 23'd0};
            // end else if (result_exp <= 0) begin
            //     // Underflow → zero (no subnormal handling here)
            //     result = 32'd0;
            // end else begin
            //     result = {result_sign_bit, result_exp[7:0], rounded_sum[22:0]};
            // end
            // $display("quotient = %h", quotient);
            // $display("exp = %d", result_exp);
            // $display("normalized quotient = %h", quotient);
            // $display("rounded_sum = %h", rounded_sum);
            // $display("final result = %h", result);

        end

        4'd5: result = {1'b0, op1[30:0]};

        4'd6: cmp = (op1 == op2);

        4'd7: cmp = (op1 != op2);

        4'd8: begin
            if (op1_sign_bit != op2_sign_bit) begin
                cmp = op1_sign_bit;
            end else if (op1_biased_exponent != op2_biased_exponent) begin
                cmp = (op1_biased_exponent < op2_biased_exponent) ^ op1_sign_bit;
            end else begin
                cmp = (op1_significand < op2_significand) ^ op1_sign_bit;
            end
        end

        default: begin
            result = 32'd0;
            cmp = 0;
        end
    endcase
end
endmodule
