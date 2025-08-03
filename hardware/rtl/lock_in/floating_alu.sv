`timescale 1ns/1ns

`include "common.svh"
// =================================================
// A 32-bit Floating Point ALU
// with ADD, SUB, MUL, DIV, EQ, NEQ, ABS and SLT
// =================================================
module floating_alu (
    input   logic       clk,
    input   logic       rst,
    output   logic       valid,
    input   logic [31:0] op1,
    input   logic [31:0] op2,
    input   alu_instruction_t instruction,
    output  logic [31:0] result
);

// =====================================================
// STAGE 1: EXPONENT ALIGNMENT - Pipeline Registers
// =====================================================
logic [31:0] s1_op1, s1_op2;
alu_instruction_t s1_instruction;
logic s1_valid;

// Stage 1 decoded signals
logic s1_op1_sign_bit, s1_op2_sign_bit;
logic [7:0] s1_op1_biased_exp, s1_op2_biased_exp;
logic [23:0] s1_op1_significand, s1_op2_significand;
logic [7:0] s1_exp_diff;
logic s1_op1_larger;
logic [7:0] s1_aligned_exp;
logic s1_result_sign_bit;

// =====================================================
// STAGE 2: MANTISSA OPERATION - Pipeline Registers
// =====================================================
logic [31:0] s2_op1, s2_op2;
logic [31:0] s3_op1, s3_op2;
logic [31:0] s4_op1, s4_op2;
alu_instruction_t s2_instruction;
logic s2_valid;

// Stage 2 signals from Stage 1
logic s2_op1_sign_bit, s2_op2_sign_bit;
logic [7:0] s2_op1_biased_exp, s2_op2_biased_exp;
logic [23:0] s2_op1_significand, s2_op2_significand;
logic [7:0] s2_exp_diff;
logic s2_op1_larger;
logic [7:0] s2_aligned_exp;
logic s2_result_sign_bit;

// Stage 2 operation results
logic [24:0] s2_mantissa_result;
logic [47:0] s2_product;
logic [47:0] s2_quotient;
logic [31:0] s2_int_result;
logic [31:0] s2_abs_op1;
logic [7:0] s2_final_exp;
logic s2_final_sign;
logic s2_overflow, s2_underflow;

// =====================================================
// STAGE 3: NORMALIZATION - Pipeline Registers
// =====================================================
alu_instruction_t s3_instruction;
logic s3_valid;

// Stage 3 signals from Stage 2
logic [24:0] s3_mantissa_result;
logic [47:0] s3_product;
logic [47:0] s3_quotient;
logic [31:0] s3_int_result;
logic [31:0] s3_abs_op1;
logic [7:0] s3_final_exp;
logic s3_final_sign;
logic s3_overflow, s3_underflow;

// Stage 3 normalization results
logic [23:0] s3_normalized_mantissa;
logic [7:0] s3_normalized_exp;
logic s3_normalized_sign;
logic s3_guard_bit, s3_round_bit, s3_sticky_bit;
logic s3_round_up;

// =====================================================
// STAGE 4: RESULT COMPOSITION - Pipeline Registers
// =====================================================
alu_instruction_t s4_instruction;
logic s4_valid;

// Stage 4 signals from Stage 3
logic [23:0] s4_normalized_mantissa;
logic [7:0] s4_normalized_exp;
logic s4_normalized_sign;
logic s4_guard_bit, s4_round_bit, s4_sticky_bit;
logic s4_round_up;
logic [31:0] shifted_mantissa;
int shift_amount;

// =====================================================
// STAGE 1: EXPONENT ALIGNMENT
// =====================================================
always_ff @(posedge clk) begin
    if (rst) begin
        s1_op1 <= 32'd0;
        s1_op2 <= 32'd0;
        s1_instruction <= FADD;
        s1_valid <= 1'b0;
    end else begin
        s1_op1 <= op1;
        s1_op2 <= op2;
        s1_instruction <= instruction;
        s1_valid <= 1'b1;
    end
end

always_comb begin
    // Decode operands
    s1_op1_sign_bit = s1_op1[31];
    s1_op2_sign_bit = s1_op2[31];
    s1_op1_biased_exp = s1_op1[30:23];
    s1_op2_biased_exp = s1_op2[30:23];
    s1_op1_significand = {1'b1, s1_op1[22:0]};
    s1_op2_significand = {1'b1, s1_op2[22:0]};
    
    // Handle FSUB by inverting op2 sign
    if (s1_instruction == FSUB) begin
        s1_op2_sign_bit = ~s1_op2_sign_bit;
    end
    
    // Exponent alignment
    if (s1_op1_biased_exp >= s1_op2_biased_exp) begin
        s1_exp_diff = s1_op1_biased_exp - s1_op2_biased_exp;
        s1_aligned_exp = s1_op1_biased_exp;
        s1_op1_larger = 1'b1;
        s1_result_sign_bit = s1_op1_sign_bit;
    end else begin
        s1_exp_diff = s1_op2_biased_exp - s1_op1_biased_exp;
        s1_aligned_exp = s1_op2_biased_exp;
        s1_op1_larger = 1'b0;
        s1_result_sign_bit = s1_op2_sign_bit;
    end
end

// =====================================================
// STAGE 2: MANTISSA OPERATION
// =====================================================
always_ff @(posedge clk) begin
    // $display("rst: ",rst);
    if (rst) begin
        s2_op1 <= 32'd0;
        s2_op2 <= 32'd0;
        s2_instruction <= FADD;
        s2_valid <= 1'b0;
        s2_op1_sign_bit <= 1'b0;
        s2_op2_sign_bit <= 1'b0;
        s2_op1_biased_exp <= 8'd0;
        s2_op2_biased_exp <= 8'd0;
        s2_op1_significand <= 24'd0;
        s2_op2_significand <= 24'd0;
        s2_exp_diff <= 8'd0;
        s2_op1_larger <= 1'b0;
        s2_aligned_exp <= 8'd0;
        s2_result_sign_bit <= 1'b0;
    end else begin
        s2_op1 <= s1_op1;
        s2_op2 <= s1_op2;
        // $display("s2_instruction: ", s2_instruction);
        s2_instruction <= s1_instruction;
        s2_valid <= s1_valid;
        s2_op1_sign_bit <= s1_op1_sign_bit;
        s2_op2_sign_bit <= s1_op2_sign_bit;
        s2_op1_biased_exp <= s1_op1_biased_exp;
        s2_op2_biased_exp <= s1_op2_biased_exp;
        s2_op1_significand <= s1_op1_significand;
        s2_op2_significand <= s1_op2_significand;
        s2_exp_diff <= s1_exp_diff;
        s2_op1_larger <= s1_op1_larger;
        s2_aligned_exp <= s1_aligned_exp;
        s2_result_sign_bit <= s1_result_sign_bit;
    end
end

always_comb begin
    // Default values
    s2_mantissa_result = 25'd0;
    s2_product = 48'd0;
    s2_quotient = 48'd0;
    s2_int_result = 32'd0;
    s2_abs_op1 = 32'd0;
    s2_final_exp = 8'd0;
    s2_final_sign = 1'b0;
    s2_overflow = 1'b0;
    s2_underflow = 1'b0;
    
    case (s2_instruction)
        FADD, FSUB: begin
            logic [24:0] aligned_op1, aligned_op2;

            // $display("s2_op1_biased_exp: ", s2_op1_biased_exp);
            // $display("s2_op2_biased_exp: ", s2_op2_biased_exp);
            //$display("s2_op1_significand: %b", s2_op1_significand);
            //$display("s2_op2_significand: %b", s2_op2_significand);
            
            // Align mantissas based on which operand has the larger exponent
            if (s2_op1_larger) begin
                aligned_op1 = {1'b0, s2_op1_significand};
                aligned_op2 = {1'b0, s2_op2_significand} >> s2_exp_diff;
            end else begin
                aligned_op1 = {1'b0, s2_op1_significand} >> s2_exp_diff;
                aligned_op2 = {1'b0, s2_op2_significand};
            end
            
            // The instruction has been decoded in Stage 1 (FSUB became an ADD with a flipped sign).
            // Now, we perform the correct operation based on the *effective* signs.
            if (s2_op1_sign_bit == s2_op2_sign_bit) begin
                // EFFECTIVE ADDITION: (A + B) or (-A + -B)
                s2_mantissa_result = aligned_op1 + aligned_op2;
                s2_final_sign = s2_op1_sign_bit; // Sign is the same as either operand
            end else begin
                // EFFECTIVE SUBTRACTION: (A + -B) or (-A + B)
                //$display("aligned_op1: %b", aligned_op1);
                //$display("aligned_op2: %b", aligned_op2);
                if (aligned_op1 >= aligned_op2) begin
                    s2_mantissa_result = aligned_op1 - aligned_op2;
                    s2_final_sign = s2_op1_sign_bit; // Result takes sign of the larger operand
                end else begin
                    s2_mantissa_result = aligned_op2 - aligned_op1;
                    s2_final_sign = s2_op2_sign_bit; // Result takes sign of the larger operand
                end
            end
            
            // The final exponent is the exponent of the larger-exponent operand
            s2_final_exp = s2_aligned_exp;
            //$display("s2_mantissa_result: ", s2_mantissa_result);
        end
        
        FMUL: begin
            s2_product = s2_op1_significand * s2_op2_significand;
            s2_final_exp = s2_op1_biased_exp + s2_op2_biased_exp - 8'd127;
            s2_final_sign = s2_op1_sign_bit ^ s2_op2_sign_bit;
        end
        
        FDIV: begin
            logic [47:0] numerator, denominator;
            numerator = {s2_op1_significand, 24'd0};
            denominator = {s2_op2_significand, 24'd0};
            s2_quotient = numerator / denominator;
            s2_final_exp = s2_op1_biased_exp - s2_op2_biased_exp + 8'd127;
            s2_final_sign = s2_op1_sign_bit ^ s2_op2_sign_bit;
        end
        
        FCVT_W_S: begin
            logic [7:0] unbiased_exp;
            unbiased_exp = s2_op1_biased_exp - 8'd127;
            
            if (s2_op1_biased_exp == 8'hFF) begin
                s2_int_result = s2_op1_sign_bit ? 32'h80000000 : 32'h7FFFFFFF;
            end else if (s2_op1_biased_exp == 0 || unbiased_exp > 8'd30) begin
                s2_int_result = 32'd0;
            end else begin
                if (unbiased_exp >= 8'd23) begin
                    s2_int_result = s2_op1_significand << (unbiased_exp - 8'd23);
                end else begin
                    s2_int_result = s2_op1_significand >> (8'd23 - unbiased_exp);
                end
                
                if (s2_op1_sign_bit) begin
                    s2_int_result = -s2_int_result;
                end
            end
        end
        
        FCVT_S_W: begin
            s2_abs_op1 = s2_op1[31] ? -s2_op1 : s2_op1;
            s2_final_sign = s2_op1[31];
        end
        
        default: begin
            // For simple operations like FNEG, FABS, FEQ
            s2_final_sign = s2_op1_sign_bit;
            s2_final_exp = s2_op1_biased_exp;
        end
    endcase
end

// =====================================================
// STAGE 3: NORMALIZATION
// =====================================================
always_ff @(posedge clk) begin
    if (rst) begin
        s3_instruction <= FADD;
        s3_valid <= 1'b0;
        s3_mantissa_result <= 25'd0;
        s3_product <= 48'd0;
        s3_quotient <= 48'd0;
        s3_int_result <= 32'd0;
        s3_abs_op1 <= 32'd0;
        s3_final_exp <= 8'd0;
        s3_final_sign <= 1'b0;
        s3_overflow <= 1'b0;
        s3_underflow <= 1'b0;
        s3_op1 <= 32'b0;
        s3_op2 <= 32'b0;
    end else begin
        s3_instruction <= s2_instruction;
        s3_valid <= s2_valid;
        s3_mantissa_result <= s2_mantissa_result;
        s3_product <= s2_product;
        s3_quotient <= s2_quotient;
        s3_int_result <= s2_int_result;
        s3_abs_op1 <= s2_abs_op1;
        s3_final_exp <= s2_final_exp;
        s3_final_sign <= s2_final_sign;
        s3_overflow <= s2_overflow;
        s3_underflow <= s2_underflow;
        s3_op1 <= s2_op1;
        s3_op2 <= s2_op2;
    end
end

always_comb begin
    // Default values
    s3_normalized_mantissa = 24'd0;
    s3_normalized_exp = 8'd0;
    s3_normalized_sign = 1'b0;
    s3_guard_bit = 1'b0;
    s3_round_bit = 1'b0;
    s3_sticky_bit = 1'b0;
    s3_round_up = 1'b0;
    
    case (s3_instruction)
        FADD, FSUB: begin
            logic [7:0] temp_exp;
            logic [24:0] temp_mantissa;
            
            temp_exp = s3_final_exp;
            temp_mantissa = s3_mantissa_result;
            
            if (temp_mantissa == 25'd0) begin
                s3_normalized_mantissa = 24'd0;
                s3_normalized_exp = 8'd0;
                s3_normalized_sign = 1'b0;
            end else if (temp_mantissa[24]) begin
                // Overflow, shift right
                s3_normalized_mantissa = temp_mantissa[24:1];
                s3_normalized_exp = temp_exp + 1;
                s3_normalized_sign = s3_final_sign;
                s3_guard_bit = temp_mantissa[1];
                s3_round_bit = temp_mantissa[0];
            end else begin
                // Normalize by shifting left
                int shift_count = 0;
                for (int i = 23; i >= 0; i--) begin
                    if (temp_mantissa[i] == 1'b1) begin
                        shift_count = 23 - i;
                        break;
                    end
                end
                
                if (shift_count > temp_exp) begin
                    // Underflow
                    s3_normalized_mantissa = 24'd0;
                    s3_normalized_exp = 8'd0;
                    s3_normalized_sign = 1'b0;
                end else begin
                    temp_mantissa = temp_mantissa << shift_count;
                    s3_normalized_mantissa = temp_mantissa[23:0];
                    s3_normalized_exp = temp_exp - shift_count;
                    s3_normalized_sign = s3_final_sign;
                    s3_guard_bit = temp_mantissa[1];
                    s3_round_bit = temp_mantissa[0];
                end
            end
        end
        
        FMUL: begin
            if (s3_product[47]) begin
                s3_normalized_mantissa = s3_product[47:24];
                s3_normalized_exp = s3_final_exp + 1;
                s3_guard_bit = s3_product[23];
                s3_round_bit = s3_product[22];
                s3_sticky_bit = |s3_product[21:0];
            end else begin
                s3_normalized_mantissa = s3_product[46:23];
                s3_normalized_exp = s3_final_exp;
                s3_guard_bit = s3_product[22];
                s3_round_bit = s3_product[21];
                s3_sticky_bit = |s3_product[20:0];
            end
            s3_normalized_sign = s3_final_sign;
        end
        
        FDIV: begin
            // The quotient sig1/sig2 is in (0.5, 2.0).
            // The fixed-point result s3_quotient = (sig1/sig2) * 2^24.
            // So the MSB of the result is at bit 24 or 23.

            if (s3_quotient[24]) begin // Result is of the form 1x.xxxxxxxx...
                // The quotient is >= 1.0. We need to shift right by 1 to normalize.
                // This is equivalent to taking the mantissa from a different position.
                s3_normalized_mantissa = s3_quotient[24:1];
                s3_normalized_exp = s3_final_exp + 1; // Exponent increases
                s3_guard_bit = s3_quotient[0];
                s3_round_bit = 1'b0; // No bits beyond the guard bit in this simple model
                s3_sticky_bit = 1'b0;
            end else begin // Result is of the form 01.xxxxxxxx... (MSB is at bit 23)
                // The quotient is < 1.0. It's already normalized.
                s3_normalized_mantissa = s3_quotient[23:0];
                s3_normalized_exp = s3_final_exp; // Exponent is unchanged
                // For a more precise model, you would need more bits from the divider
                // to calculate rounding bits. For now, we can assume they are zero.
                s3_guard_bit = 1'b0;
                s3_round_bit = 1'b0;
                s3_sticky_bit = 1'b0;
            end
            s3_normalized_sign = s3_final_sign;
        end
        
        FCVT_S_W: begin
            if (s3_abs_op1 == 32'd0) begin
                s3_normalized_mantissa = 24'd0;
                s3_normalized_exp = 8'd0;
                s3_normalized_sign = 1'b0;
            end else begin
                int leading_one_pos = 0;
                for (int i = 31; i >= 0; i--) begin
                    if (s3_abs_op1[i]) begin
                        leading_one_pos = i;
                        break;
                    end
                end
                
                // The exponent is based on the position of the leading one.
                s3_normalized_exp = 127 + leading_one_pos;
                s3_normalized_sign = s3_final_sign;

                // To get the mantissa, we need to shift the number so the leading one
                // is just to the left of the 23-bit mantissa field.
                // This is effectively a variable shift, which synthesizes to a barrel shifter.
                shift_amount = 23 - leading_one_pos;
                
                if (shift_amount > 0) begin
                    // If leading_one_pos < 23, we need to shift left.
                    shifted_mantissa = s3_abs_op1 << shift_amount;
                end else begin
                    // If leading_one_pos >= 23, we need to shift right.
                    shifted_mantissa = s3_abs_op1 >> -shift_amount;
                end
                
                // The final mantissa is the top 23 bits, excluding the implicit leading one.
                // We construct the 24-bit value (with implicit 1) for the next stage.
                s3_normalized_mantissa = {1'b1, shifted_mantissa[22:0]};
            end
        end
        
        default: begin
            s3_normalized_mantissa = {1'b1, s3_final_exp, 15'd0}; // Placeholder
            s3_normalized_exp = s3_final_exp;
            s3_normalized_sign = s3_final_sign;
        end
    endcase
    
    // Calculate rounding
    s3_round_up = s3_guard_bit & (s3_round_bit | s3_sticky_bit | s3_normalized_mantissa[0]);
end

// =====================================================
// STAGE 4: RESULT COMPOSITION
// =====================================================
always_ff @(posedge clk) begin
    if (rst) begin
        s4_instruction <= FADD;
        s4_valid <= 1'b0;
        s4_normalized_mantissa <= 24'd0;
        s4_normalized_exp <= 8'd0;
        s4_normalized_sign <= 1'b0;
        s4_guard_bit <= 1'b0;
        s4_round_bit <= 1'b0;
        s4_sticky_bit <= 1'b0;
        s4_round_up <= 1'b0;
        s4_op1 <= 32'b0;
        s4_op2 <= 32'b0;
    end else begin
        s4_instruction <= s3_instruction;
        s4_valid <= s3_valid;
        s4_normalized_mantissa <= s3_normalized_mantissa;
        s4_normalized_exp <= s3_normalized_exp;
        s4_normalized_sign <= s3_normalized_sign;
        s4_guard_bit <= s3_guard_bit;
        s4_round_bit <= s3_round_bit;
        s4_sticky_bit <= s3_sticky_bit;
        s4_round_up <= s3_round_up;
        s4_op1 <= s3_op1;
        s4_op2 <= s3_op2;
    end
end

// Final result composition
always_comb begin
    result = 32'd0;    
    case (s4_instruction)
        FADD, FSUB, FMUL, FDIV, FCVT_S_W: begin
            logic [23:0] final_mantissa;
            logic [7:0] final_exp;
            
            final_mantissa = s4_normalized_mantissa;
            final_exp = s4_normalized_exp;
            
            // Apply rounding
            if (s4_round_up) begin
                final_mantissa = final_mantissa + 1;
                if (final_mantissa[23]) begin
                    final_mantissa = final_mantissa >> 1;
                    final_exp = final_exp + 1;
                end
            end
            
            // Handle overflow/underflow
            if (final_exp >= 8'd255) begin
                result = {s4_normalized_sign, 8'hFF, 23'd0}; // Infinity
            end else if (final_exp == 8'd0) begin
                result = {s4_normalized_sign, 8'd0, 23'd0}; // Zero
            end else begin
                result = {s4_normalized_sign, final_exp, final_mantissa[22:0]};
            end
            //$display("final mantissa: %b", final_mantissa);
            //$display("final_exp: %b", final_exp);
        end
        
        FCVT_W_S: begin
            result = s3_int_result; // Pass through from stage 3
        end
        
        FNEG: begin
            result = {~s4_op1[31], s4_op1[30:0]};
        end
        
        FABS: begin
            result = {1'b0, s4_op1[30:0]};
        end
        
        FEQ: begin
            // Compare original operands from stage 2
                logic both_are_zero = (s4_op1[30:0] == 31'd0) && (s4_op2[30:0] == 31'd0);
            if (both_are_zero) begin
                result = 32'd1;
            end else begin
                result = (s4_op1 == s4_op2) ? 32'd1 : 32'd0;
            end
        end

        FSLT: begin
            // Compare original operands from stage 2
            logic a_sign = s4_op1[31];
            logic b_sign = s4_op2[31];
            logic [30:0] a_mag = s4_op1[30:0];
            logic [30:0] b_mag = s4_op2[30:0];

            logic is_less;
            if (a_sign != b_sign) begin
                is_less = a_sign; 
            end else begin
                if (!a_sign) begin // Both positive
                    is_less = (a_mag < b_mag);
                end else begin // Both negative
                    is_less = (a_mag > b_mag);
                end
            end
            
            if ((a_mag == 31'd0) && (b_mag == 31'd0)) begin
                result = 32'd0;
            end else begin
                result = is_less ? 32'd1 : 32'd0;
            end
        end
        
        default: begin
            result = 32'd0;
        end
    endcase
    //$display("result: %h", result);
end

// Valid signal follows the pipeline
assign valid = s4_valid;

endmodule
