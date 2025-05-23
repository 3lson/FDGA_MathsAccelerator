#include "base_testbench.h"
#include <verilated_cov.h>
#include <gtest/gtest.h>
#include <bit>

// Floating ALU Ops
#define FALU_ADD 0b0001
#define FALU_SUB 0b0010
#define FALU_MUL 0b0011
#define FALU_DIV 0b0100
#define FALU_ABS 0b0101
#define FALU_EQ  0b0110
#define FALU_NEQ 0b0111
#define FALU_SLT 0b1000

class FloatingALUTestbench : public BaseTestbench {
protected:
    void initializeInputs() override {
        top->alu_op = 0;
        top->op1 = 0;
        top->op2 = 0;
    }

    static uint32_t float_to_bits(float f) {
        return *reinterpret_cast<uint32_t*>(&f);
    }

    static float bits_to_float(uint32_t bits) {
        return *reinterpret_cast<float*>(&bits);
    }
};

TEST_F(FloatingALUTestbench, AddTest) {
    float op1 = 3.5f;
    float op2 = 2.25f;
    float expected = op1 + op2;

    top->alu_op = FALU_ADD;
    top->op1 = float_to_bits(op1);
    top->op2 = float_to_bits(op2);

    top->eval();

    EXPECT_FLOAT_EQ(bits_to_float(top->result), expected);
    EXPECT_EQ(top->cmp, 0);
}

TEST_F(FloatingALUTestbench, SubTest) {
    float op1 = 10.0f;
    float op2 = 4.0f;
    float expected = op1 - op2;

    top->alu_op = FALU_SUB;
    top->op1 = float_to_bits(op1);
    top->op2 = float_to_bits(op2);

    top->eval();

    EXPECT_FLOAT_EQ(bits_to_float(top->result), expected);
    EXPECT_EQ(top->cmp, 0);
}

TEST_F(FloatingALUTestbench, MulTest) {
    float op1 = 1.5f;
    float op2 = -2.0f;
    float expected = op1 * op2;

    top->alu_op = FALU_MUL;
    top->op1 = float_to_bits(op1);
    top->op2 = float_to_bits(op2);

    top->eval();

    EXPECT_FLOAT_EQ(bits_to_float(top->result), expected);
    EXPECT_EQ(top->cmp, 0);
}

TEST_F(FloatingALUTestbench, DivTest) {
    float op1 = 6.0f;
    float op2 = 3.0f;
    float expected = op1 / op2;

    top->alu_op = FALU_DIV;
    top->op1 = float_to_bits(op1);
    top->op2 = float_to_bits(op2);

    top->eval();

    EXPECT_FLOAT_EQ(bits_to_float(top->result), expected);
    EXPECT_EQ(top->cmp, 0);
}

TEST_F(FloatingALUTestbench, AbsTest) {
    float op1 = -7.25f;
    float expected = std::abs(op1);

    top->alu_op = FALU_ABS;
    top->op1 = float_to_bits(op1);
    top->op2 = 0;

    top->eval();

    EXPECT_FLOAT_EQ(bits_to_float(top->result), expected);
    EXPECT_EQ(top->cmp, 0);
}

TEST_F(FloatingALUTestbench, EqTestTrue) {
    float op1 = 4.0f;
    float op2 = 4.0f;

    top->alu_op = FALU_EQ;
    top->op1 = float_to_bits(op1);
    top->op2 = float_to_bits(op2);

    top->eval();

    EXPECT_EQ(top->cmp, 1);
}

TEST_F(FloatingALUTestbench, EqTestFalse) {
    float op1 = 4.0f;
    float op2 = 5.0f;

    top->alu_op = FALU_EQ;
    top->op1 = float_to_bits(op1);
    top->op2 = float_to_bits(op2);

    top->eval();

    EXPECT_EQ(top->cmp, 0);
}

TEST_F(FloatingALUTestbench, NeqTestTrue) {
    float op1 = 1.0f;
    float op2 = 2.0f;

    top->alu_op = FALU_NEQ;
    top->op1 = float_to_bits(op1);
    top->op2 = float_to_bits(op2);

    top->eval();

    EXPECT_EQ(top->cmp, 1);
}

TEST_F(FloatingALUTestbench, NeqTestFalse) {
    float op1 = -3.0f;
    float op2 = -3.0f;

    top->alu_op = FALU_NEQ;
    top->op1 = float_to_bits(op1);
    top->op2 = float_to_bits(op2);

    top->eval();

    EXPECT_EQ(top->cmp, 0);
}

TEST_F(FloatingALUTestbench, SltTestTrue) {
    float op1 = 1.0f;
    float op2 = 2.0f;

    top->alu_op = FALU_SLT;
    top->op1 = float_to_bits(op1);
    top->op2 = float_to_bits(op2);

    top->eval();

    EXPECT_EQ(top->cmp, 1);
}

TEST_F(FloatingALUTestbench, SltTestFalse) {
    float op1 = 4.0f;
    float op2 = -2.0f;

    top->alu_op = FALU_SLT;
    top->op1 = float_to_bits(op1);
    top->op2 = float_to_bits(op2);

    top->eval();

    EXPECT_EQ(top->cmp, 0);
}


int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);

    Verilated::mkdir("logs");
    auto result = RUN_ALL_TESTS();
    VerilatedCov::write("logs/coverage.dat");

    return result;
}
