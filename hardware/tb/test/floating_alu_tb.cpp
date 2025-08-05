#include "sync_testbench.h"
#include <verilated_cov.h>
#include <gtest/gtest.h>
#include <bit>

// Floating ALU Ops
#define FALU_ADD    12
#define FALU_SUB    13
#define FALU_MUL    14
#define FALU_SLT    15
#define FALU_NEG    16
#define FALU_EQ     17
#define FALU_ABS    19
#define FALU_FCVT_WS 20
#define FALU_FCVT_SW 21


class FloatingALUTestbench : public SyncTestbench {
protected:
    void initializeInputs() override {
        top->instruction = 0;
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

    top->instruction = FALU_ADD;
    top->op1 = float_to_bits(op1);
    top->op2 = float_to_bits(op2);

    runSimulation(5);

    EXPECT_FLOAT_EQ(bits_to_float(top->result), expected);
}

TEST_F(FloatingALUTestbench, SubTest) {
    float op1 = 10.0f;
    float op2 = 4.0f;
    float expected = op1 - op2;

    top->instruction = FALU_SUB;
    top->op1 = float_to_bits(op1);
    top->op2 = float_to_bits(op2);

    runSimulation(5);

    EXPECT_FLOAT_EQ(bits_to_float(top->result), expected);
}

TEST_F(FloatingALUTestbench, MulTest) {
    float op1 = 1.5f;
    float op2 = -2.0f;
    float expected = op1 * op2;

    top->instruction = FALU_MUL;
    top->op1 = float_to_bits(op1);
    top->op2 = float_to_bits(op2);

    runSimulation(5);

    EXPECT_FLOAT_EQ(bits_to_float(top->result), expected);
}

TEST_F(FloatingALUTestbench, NegTest) {
    float op1 = 7.25f;
    float expected = -7.25f;

    top->instruction = FALU_NEG;
    top->op1 = float_to_bits(op1);
    top->op2 = 0;

    runSimulation(5);

    EXPECT_FLOAT_EQ(bits_to_float(top->result), expected);
}

TEST_F(FloatingALUTestbench, AbsTest) {
    float op1 = -7.25f;
    float expected = std::abs(op1);

    top->instruction = FALU_ABS;
    top->op1 = float_to_bits(op1);
    top->op2 = 0;

    runSimulation(5);

    EXPECT_FLOAT_EQ(bits_to_float(top->result), expected);
}

TEST_F(FloatingALUTestbench, EqTestTrue) {
    float op1 = 4.0f;
    float op2 = 4.0f;

    top->instruction = FALU_EQ;
    top->op1 = float_to_bits(op1);
    top->op2 = float_to_bits(op2);

    runSimulation(5);

    EXPECT_FLOAT_EQ(top->result, 1);
}

TEST_F(FloatingALUTestbench, EqTestFalse) {
    float op1 = 4.0f;
    float op2 = 5.0f;

    top->instruction = FALU_EQ;
    top->op1 = float_to_bits(op1);
    top->op2 = float_to_bits(op2);

    runSimulation(5);

    EXPECT_FLOAT_EQ(bits_to_float(top->result), 0);
}


TEST_F(FloatingALUTestbench, SltTestFalse) {
    float op1 = 4.0f;
    float op2 = -2.0f;

    top->instruction = FALU_SLT;
    top->op1 = float_to_bits(op1);
    top->op2 = float_to_bits(op2);

    runSimulation(5);
    EXPECT_FLOAT_EQ(top->result, 0);
}

TEST_F(FloatingALUTestbench, SltTestTrue) {
    float op1 = -2.0f;
    float op2 = 4.0f;

    top->instruction = FALU_SLT;
    top->op1 = float_to_bits(op1);
    top->op2 = float_to_bits(op2);

    runSimulation(5);
    EXPECT_FLOAT_EQ(top->result, 1);
}

TEST_F(FloatingALUTestbench, ConvertFloatToInt) {
    struct TestCase {
        float input;
        int32_t expected;
    };

    std::vector<TestCase> test_cases = {
        {4.0f, 4},
        {-7.0f, -7},
        {0.0f, 0},
        {1.999f, 1},    // truncation
        {-1.999f, -1},  // truncation
        {8388608.0f, 8388608}, // 2^23, exact
        {8388607.0f, 8388607}, // max int representable exactly
        {1e-30f, 0},    // subnormal, rounds to 0
        {INFINITY, 0x7FFFFFFF},  // saturates
        {-INFINITY, INT32_MIN}, // saturates to min neg
        {NAN, 0x7FFFFFFF}        // treat NaN as saturated (or however your HDL handles it)
    };

    for (const auto& tc : test_cases) {
        top->instruction = FALU_FCVT_WS; // fixed macro name
        top->op1 = float_to_bits(tc.input);

        runSimulation(5);

        EXPECT_EQ(static_cast<int32_t>(top->result), tc.expected)
            << "Failed for input: " << tc.input;
    }
}

TEST_F(FloatingALUTestbench, ConvertIntToFloat) {
    struct TestCase {
        int32_t input;
        float expected;
    };

    std::vector<TestCase> test_cases = {
        {0, 0.0f},
        {1, 1.0f},
        {-1, -1.0f},
        {123456, 123456.0f},
        {-123456, -123456.0f},
        {INT32_MAX, static_cast<float>(INT32_MAX)},
        {INT32_MIN, static_cast<float>(INT32_MIN)},
    };

    for (const auto& tc : test_cases) {
        top->instruction = FALU_FCVT_SW; // Integer to float conversion opcode
        top->op1 = static_cast<uint32_t>(tc.input);

        runSimulation(5);

        float result = bits_to_float(top->result);
        EXPECT_FLOAT_EQ(result, tc.expected)
            << "Failed for input: " << tc.input;
    }
}


int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);

    Verilated::mkdir("logs");
    auto result = RUN_ALL_TESTS();
    VerilatedCov::write("logs/coverage.dat");

    return result;
}