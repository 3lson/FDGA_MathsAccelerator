#include "base_testbench.h"
#include <gtest/gtest.h>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include <bit>
#include <cmath>

// ALU Operations
#define ALU_ADD  0b0000
#define ALU_SUB  0b0001
#define ALU_MUL  0b0010
#define ALU_DIV  0b0011
#define ALU_SLT  0b0100
#define ALU_SGT  0b0101
#define ALU_SEQ  0b0110
#define ALU_SNEZ 0b0111
#define ALU_MIN  0b1000
#define ALU_ABS  0b1001

class LaneTestbench : public BaseTestbench<Vlane> {
protected:
    void initializeInputs() override {
        dut->clk = 0;
        dut->stall = 0;
        dut->threads = 0b0001; // Start with thread 0 active
        dut->bIdx = 0;
        dut->FUNCT4 = 0;
        dut->IMM = 0;
        dut->AD1 = 0;
        dut->AD2 = 0;
        dut->AD3 = 0;
        dut->is_int = 0;
        dut->is_float = 0;
        dut->WE3 = 0;
    }

    void setRegister(uint8_t thread, uint8_t reg, uint32_t value) {
        dut->threads = 1 << thread;
        dut->AD3 = reg;
        dut->WD3 = value;
        dut->WE3 = 1;
        step();
        dut->WE3 = 0;
    }

    uint32_t getRegister(uint8_t thread, uint8_t reg) {
        dut->threads = 1 << thread;
        dut->AD1 = reg;
        step();
        return dut->RD1[thread];
    }

    void performOperation(uint8_t op, bool is_int, bool is_float, 
                         uint8_t src1, uint8_t src2, uint8_t dest) {
        dut->FUNCT4 = op;
        dut->is_int = is_int;
        dut->is_float = is_float;
        dut->AD1 = src1;
        dut->AD2 = src2;
        dut->AD3 = dest;
        dut->WE3 = 1;
        step();
        dut->WE3 = 0;
    }

    void switchThread(uint8_t thread) {
        dut->threads = 1 << thread;
        step();
    }
};

// Integer Operation Tests
TEST_F(LaneTestbench, IntegerAdd) {
    setRegister(0, 1, 10);
    setRegister(0, 2, 20);
    performOperation(ALU_ADD, true, false, 1, 2, 3);
    EXPECT_EQ(getRegister(0, 3), 30);
}

TEST_F(LaneTestbench, IntegerSub) {
    setRegister(0, 1, 50);
    setRegister(0, 2, 20);
    performOperation(ALU_SUB, true, false, 1, 2, 3);
    EXPECT_EQ(getRegister(0, 3), 30);
}

TEST_F(LaneTestbench, IntegerMul) {
    setRegister(0, 1, 6);
    setRegister(0, 2, 7);
    performOperation(ALU_MUL, true, false, 1, 2, 3);
    EXPECT_EQ(getRegister(0, 3), 42);
}

TEST_F(LaneTestbench, IntegerDiv) {
    setRegister(0, 1, 20);
    setRegister(0, 2, 5);
    performOperation(ALU_DIV, true, false, 1, 2, 3);
    EXPECT_EQ(getRegister(0, 3), 4);
}

TEST_F(LaneTestbench, IntegerLessThan) {
    setRegister(0, 1, 3);
    setRegister(0, 2, 5);
    performOperation(ALU_SLT, true, false, 1, 2, 3);
    EXPECT_EQ(getRegister(0, 3), 1);
}

// Floating Point Operation Tests
TEST_F(LaneTestbench, FloatAdd) {
    float a = 1.5f, b = 2.25f;
    setRegister(0, 1, std::bit_cast<uint32_t>(a));
    setRegister(0, 2, std::bit_cast<uint32_t>(b));
    performOperation(ALU_ADD, false, true, 1, 2, 3);
    float result = std::bit_cast<float>(getRegister(0, 3));
    EXPECT_NEAR(result, a + b, 1e-5);
}

TEST_F(LaneTestbench, FloatSub) {
    float a = 5.5f, b = 2.25f;
    setRegister(0, 1, std::bit_cast<uint32_t>(a));
    setRegister(0, 2, std::bit_cast<uint32_t>(b));
    performOperation(ALU_SUB, false, true, 1, 2, 3);
    float result = std::bit_cast<float>(getRegister(0, 3));
    EXPECT_NEAR(result, a - b, 1e-5);
}

TEST_F(LaneTestbench, FloatMul) {
    float a = 1.5f, b = 2.0f;
    setRegister(0, 1, std::bit_cast<uint32_t>(a));
    setRegister(0, 2, std::bit_cast<uint32_t>(b));
    performOperation(ALU_MUL, false, true, 1, 2, 3);
    float result = std::bit_cast<float>(getRegister(0, 3));
    EXPECT_NEAR(result, a * b, 1e-5);
}

TEST_F(LaneTestbench, FloatAbs) {
    float a = -3.75f;
    setRegister(0, 1, std::bit_cast<uint32_t>(a));
    performOperation(ALU_ABS, false, true, 1, 0, 2); // Second operand unused
    float result = std::bit_cast<float>(getRegister(0, 2));
    EXPECT_NEAR(result, std::abs(a), 1e-5);
}

// Threading Tests
TEST_F(LaneTestbench, ThreadIsolation) {
    // Set different values in same register of different threads
    setRegister(0, 1, 100);
    setRegister(1, 1, 200);
    
    // Verify values are isolated
    EXPECT_EQ(getRegister(0, 1), 100);
    EXPECT_EQ(getRegister(1, 1), 200);
}

TEST_F(LaneTestbench, ThreadSwitch) {
    setRegister(0, 1, 123);
    setRegister(1, 1, 456);
    
    switchThread(0);
    EXPECT_EQ(getRegister(0, 1), 123);
    
    switchThread(1);
    EXPECT_EQ(getRegister(1, 1), 456);
}

// Invalid Operation Test
TEST_F(LaneTestbench, InvalidOperation) {
    setRegister(0, 1, 123);
    setRegister(0, 2, 456);
    
    // Neither is_int nor is_float set
    performOperation(ALU_ADD, false, false, 1, 2, 3);
    EXPECT_EQ(getRegister(0, 3), 0xDEADBEEF);
}

// Stall Test
TEST_F(LaneTestbench, StallOperation) {
    setRegister(0, 1, 10);
    setRegister(0, 2, 20);
    
    // Perform operation with stall
    dut->stall = 1;
    performOperation(ALU_ADD, true, false, 1, 2, 3);
    EXPECT_EQ(getRegister(0, 3), 0); // Shouldn't have written
    
    // Release stall and verify operation completes
    dut->stall = 0;
    step();
    EXPECT_EQ(getRegister(0, 3), 30);
}

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::traceEverOn(true);
    Verilated::mkdir("logs");
    
    auto result = RUN_ALL_TESTS();
    VerilatedCov::write("logs/coverage.dat");
    
    return result;
}