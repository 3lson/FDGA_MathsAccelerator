#include "base_testbench.h"
#include "Vfloating_alu.h"
#include <verilated_cov.h>
#include <gtest/gtest.h>
#include <bit>
#include <cstring>
#include <cmath>
#include <vector>
#include <string>

// Floating ALU ops
#define FADD 1
#define FSUB 2
#define FMUL 3
#define FDIV 4
#define FABS 5
#define FEQ 6
#define FNE 7
#define FLT 8

uint32_t floatToBits(float f) {
    uint32_t bits;
    std::memcpy(&bits, &f, sizeof(float));
    return bits;
}

float bitsToFloat(uint32_t bits) {
    float f;
    std::memcpy(&f, &bits, sizeof(uint32_t));
    return f;
}

class FloatingALUTestbench : public BaseTestbench {
protected:
    void initializeInputs() override {
        top->alu_op = 0;
        top->op1 = 0;
        top->op2 = 0;
    }
};

struct FloatTestCase {
    float op1;
    float op2;
    uint8_t alu_op;
    float expected_result;
    bool expected_cmp;
    std::string description;
    float tolerance = 1e-5f; // floating point tolerance
};

class FloatingALUParameterizedTest : public FloatingALUTestbench, public ::testing::WithParamInterface<FloatTestCase> {};

TEST_P(FloatingALUParameterizedTest, RunTest) {
    const auto& test = GetParam();
    top->alu_op = test.alu_op;
    top->op1 = floatToBits(test.op1);
    top->op2 = floatToBits(test.op2);

    top->eval();

    float actual_result = bitsToFloat(top->result);
    EXPECT_NEAR(actual_result, test.expected_result, test.tolerance) << "FAIL: " << test.description;
    EXPECT_EQ(top->cmp, test.expected_cmp) << "FAIL CMP: " << test.description;
}

INSTANTIATE_TEST_SUITE_P(FloatingALUTests, FloatingALUParameterizedTest, ::testing::Values(
    // FADD
    FloatTestCase{1.0f, 1.0f, FADD, 2.0f, false, "1.0 + 1.0 = 2.0"},
    FloatTestCase{2.0f, 1.0f, FADD, 3.0f, false, "2.0 + 1.0 = 3.0"},
    FloatTestCase{1.0f, -1.0f, FADD, 0.0f, false, "1.0 + (-1.0) = 0.0"},

    // FSUB
    FloatTestCase{1.0f, 1.0f, FSUB, 0.0f, false, "1.0 - 1.0 = 0.0"},
    FloatTestCase{2.0f, 1.0f, FSUB, 1.0f, false, "2.0 - 1.0 = 1.0"},
    FloatTestCase{1.0f, 2.0f, FSUB, -1.0f, false, "1.0 - 2.0 = -1.0"},
    FloatTestCase{-1.0f, -1.0f, FSUB, 0.0f, false, "-1.0 - (-1.0) = 0.0"},

    // FMUL
    FloatTestCase{1.0f, 1.0f, FMUL, 1.0f, false, "1.0 * 1.0 = 1.0"},
    FloatTestCase{2.0f, 1.0f, FMUL, 2.0f, false, "2.0 * 1.0 = 2.0"},
    FloatTestCase{-2.0f, 2.0f, FMUL, -4.0f, false, "-2.0 * 2.0 = -4.0"},
    FloatTestCase{1.0f, 0.0f, FMUL, 0.0f, false, "1.0 * 0.0 = 0.0"},

    // FDIV
    FloatTestCase{2.0f, 1.0f, FDIV, 2.0f, false, "2.0 / 1.0 = 2.0"},
    FloatTestCase{1.0f, 2.0f, FDIV, 0.5f, false, "1.0 / 2.0 = 0.5"},
    FloatTestCase{-2.0f, 2.0f, FDIV, -1.0f, false, "-2.0 / 2.0 = -1.0"},
    FloatTestCase{1.0f, 1e-10f, FDIV, 1.0f / 1e-10f, false, "1.0 / small = large"},

    // FABS
    FloatTestCase{-1.0f, 0.0f, FABS, 1.0f, false, "|-1.0| = 1.0"},
    FloatTestCase{1.0f, 0.0f, FABS, 1.0f, false, "|1.0| = 1.0"},
    FloatTestCase{0.0f, 0.0f, FABS, 0.0f, false, "|0.0| = 0.0"},

    // FEQ
    FloatTestCase{1.0f, 1.0f, FEQ, 0.0f, true, "1.0 == 1.0"},
    FloatTestCase{1.0f, 2.0f, FEQ, 0.0f, false, "1.0 == 2.0"},
    FloatTestCase{-1.0f, -1.0f, FEQ, 0.0f, true, "-1.0 == -1.0"},

    // FNE
    FloatTestCase{1.0f, 2.0f, FNE, 0.0f, true, "1.0 != 2.0"},
    FloatTestCase{1.0f, 1.0f, FNE, 0.0f, false, "1.0 != 1.0"},

    // FLT
    FloatTestCase{1.0f, 2.0f, FLT, 0.0f, true, "1.0 < 2.0"},
    FloatTestCase{2.0f, 1.0f, FLT, 0.0f, false, "2.0 < 1.0"},
    FloatTestCase{-1.0f, 1.0f, FLT, 0.0f, true, "-1.0 < 1.0"},
    FloatTestCase{2.0f, 2.0f, FLT, 0.0f, false, "2.0 < 2.0"}
));

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    int result = RUN_ALL_TESTS();
    VerilatedCov::write("logs/coverage.dat");
    return result;
}
