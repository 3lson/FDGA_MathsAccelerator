#include "base_testbench.h"
#include <verilated_cov.h>
#include <gtest/gtest.h>
#include <bit>
#include <cmath>
#include <vector>

class LaneTestbench : public BaseTestbench {
protected:
    void initializeInputs() override {
        top->clk = 0;
        top->stall = 0;
        top->threads = 0;
        top->bIdx = 0;
        top->FUNCT4 = 0;
        top->IMM = 0;
        top->AD1 = 0;
        top->AD2 = 0;
        top->AD3 = 0;
        top->is_int = 0;
        top->is_float = 0;
        top->WE3 = 0;
    }

    void clock() {
        top->clk = 0;
        top->eval();
        top->clk = 1;
        top->eval();
        top->clk = 0;
        top->eval();
    }

    static uint32_t float_to_bits(float f) {
        uint32_t bits;
        std::memcpy(&bits, &f, sizeof(f));
        return bits;
    }

    static float bits_to_float(uint32_t bits) {
        float f;
        std::memcpy(&f, &bits, sizeof(bits));
        return f;
    }

    void setThread(uint8_t thread) {
        top->threads = thread;
    }

    void writeRegister(uint8_t addr, uint32_t value, uint8_t thread) {
        top->AD3 = addr;
        top->WD3 = value;
        top->WE3 = 1;
        setThread(thread);
        clock();
        top->WE3 = 0;
    }

    void checkRegister(uint8_t addr, uint32_t expected, uint8_t thread) {
        top->AD1 = addr;
        setThread(thread);
        top->eval();
        EXPECT_EQ(top->RD1[thread], expected);
    }
};

TEST_F(LaneTestbench, IntegerAddOperation) {
    // Initialize
    writeRegister(1, 10, 0);  // Write 10 to register 1 for thread 0
    writeRegister(2, 20, 0);  // Write 20 to register 2 for thread 0

    // Set up ADD operation (assuming FUNCT4=0 is ADD)
    top->AD1 = 1;
    top->AD2 = 2;
    top->FUNCT4 = 0b0000;  // ADD operation
    top->is_int = 1;
    top->eval();

    // Check result
    EXPECT_EQ(top->alu_result, 30);
    
    // Write back and verify
    writeRegister(3, top->alu_result, 0);
    checkRegister(3, 30, 0);
}

TEST_F(LaneTestbench, FloatingAddOperation) {
    float a = 3.5f;
    float b = 2.25f;
    
    // Initialize floating point values
    writeRegister(1, float_to_bits(a), 0);
    writeRegister(2, float_to_bits(b), 0);

    // Set up FADD operation (assuming FUNCT4=0 is FADD)
    top->AD1 = 1;
    top->AD2 = 2;
    top->FUNCT4 = 0b0000;  // FADD operation
    top->is_float = 1;
    top->eval();

    // Check result
    EXPECT_FLOAT_EQ(bits_to_float(top->floatingalu_result), a + b);
    
    // Write back and verify
    writeRegister(3, top->floatingalu_result, 0);
    checkRegister(3, float_to_bits(a + b), 0);
}

TEST_F(LaneTestbench, ThreadIsolation) {
    // Test that threads don't interfere with each other
    writeRegister(1, 100, 0);  // Thread 0
    writeRegister(1, 200, 1);  // Thread 1

    // Verify isolation
    top->AD1 = 1;
    
    top->threads = 0;
    top->eval();
    EXPECT_EQ(top->RD1[0], 100);
    
    top->threads = 1;
    top->eval();
    EXPECT_EQ(top->RD1[1], 200);
}

TEST_F(LaneTestbench, IntegerComparison) {
    // Test EQ operation
    writeRegister(1, 10, 0);
    writeRegister(2, 10, 0);

    top->AD1 = 1;
    top->AD2 = 2;
    top->FUNCT4 = 0b0110;  // EQ operation
    top->is_int = 1;
    top->eval();

    EXPECT_EQ(top->int_eq_shared, 1);
}

TEST_F(LaneTestbench, FloatingComparison) {
    // Test FEQ operation
    float a = 3.14f;
    float b = 3.14f;
    
    writeRegister(1, float_to_bits(a), 0);
    writeRegister(2, float_to_bits(b), 0);

    top->AD1 = 1;
    top->AD2 = 2;
    top->FUNCT4 = 0b0110;  // FEQ operation
    top->is_float = 1;
    top->eval();

    EXPECT_EQ(top->float_eq_shared, 1);
}

TEST_F(LaneTestbench, StallBehavior) {
    writeRegister(1, 42, 0);
    
    // Normal operation
    top->AD1 = 1;
    top->stall = 0;
    top->eval();
    EXPECT_EQ(top->RD1[0], 42);
    
    // Stalled operation
    top->stall = 1;
    top->eval();
    // Should maintain previous values when stalled
    // (Exact behavior depends on your implementation)
}

TEST_F(LaneTestbench, ResultSelection) {
    // Test that final_result selects the correct ALU output
    writeRegister(1, 10, 0);
    writeRegister(2, 20, 0);
    
    // Integer operation
    top->AD1 = 1;
    top->AD2 = 2;
    top->FUNCT4 = 0b0000;  // ADD
    top->is_int = 1;
    top->eval();
    EXPECT_EQ(top->final_result, top->alu_result);
    
    // Floating operation
    top->is_int = 0;
    top->is_float = 1;
    top->eval();
    EXPECT_EQ(top->final_result, top->floatingalu_result);
    
    // Invalid operation
    top->is_int = 0;
    top->is_float = 0;
    top->eval();
    EXPECT_EQ(top->final_result, 0xDEADBEEF);
}

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);

    Verilated::mkdir("logs");
    auto result = RUN_ALL_TESTS();
    VerilatedCov::write("logs/coverage.dat");

    return result;
}