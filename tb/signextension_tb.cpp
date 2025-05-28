#include "base_testbench.h"
#include <verilated_cov.h>
#include <gtest/gtest.h>
#include <bitset>
#include <cstdint>
#include <iostream>

class SignExtensionTestbench : public BaseTestbench {
protected:
    void initializeInputs() override {
        top->instr = 0;
        top->ImmSrc = 0;
    }
};

// Helper to extract and sign-extend bits like the Verilog does
int32_t signExtend(uint32_t value, int bits) {
    int32_t mask = 1 << (bits - 1);
    return (value ^ mask) - mask;  // simple sign-extension trick
}

// ImmSrc 000: I-type (instr[27:14]), sign-extended from 14 bits
TEST_F(SignExtensionTestbench, ITypeImmediate) {
    top->ImmSrc = 0b000;
    top->instr = (0b1 << 27) | (0x123 << 14);  // Negative value: MSB=1

    top->eval();

    int32_t expected = signExtend((top->instr >> 14) & 0x3FFF, 14);
    EXPECT_EQ(top->ImmOp, expected) << "Expected: " << std::hex << expected;
}

// ImmSrc 001: M-type LOAD (instr[28:14]), 15-bit signed
TEST_F(SignExtensionTestbench, MTypeLoadImmediate) {
    top->ImmSrc = 0b001;
    top->instr = (0b1 << 28) | (0x2345 << 14);  // MSB=1 for sign-extension

    top->eval();

    int32_t expected = signExtend((top->instr >> 14) & 0x7FFF, 15);
    EXPECT_EQ(top->ImmOp, expected) << "Expected: " << std::hex << expected;
}

// ImmSrc 010: M-type STORE (instr[28:19] & instr[4:0]), 15-bit signed
TEST_F(SignExtensionTestbench, MTypeStoreImmediate) {
    top->ImmSrc = 0b010;
    top->instr = (0b1 << 28) | (0x1F << 19) | 0x1F;  // MSB=1 + split field

    top->eval();

    uint32_t imm15 = ((top->instr >> 19) & 0x3FF) << 5 | (top->instr & 0x1F);
    int32_t expected = signExtend(imm15, 15);
    EXPECT_EQ(top->ImmOp, expected) << "Expected: " << std::hex << expected;
}

// ImmSrc 011: C-type JUMP/BRANCH (instr[28:13], instr[9:0]), shifted left by 2
TEST_F(SignExtensionTestbench, CTypeBranchImmediate) {
    top->ImmSrc = 0b011;
    // Let's set MSB=1, instr[28:13] = 0x1234, instr[9:0] = 0x2A
    top->instr = (1 << 28) | (0x1234 << 13) | 0x2A;

    top->eval();

    uint32_t raw = ((top->instr >> 13) & 0xFFFF) << 10 | (top->instr & 0x3FF);
    int32_t extended = signExtend(raw, 26) << 2;
    EXPECT_EQ(top->ImmOp, extended) << "Expected: " << std::hex << extended;
}

// ImmSrc 100: C-type CALL (instr[28:13]), shifted left by 2
TEST_F(SignExtensionTestbench, CTypeCallImmediate) {
    top->ImmSrc = 0b100;
    top->instr = (1 << 28) | (0x2345 << 13);  // MSB=1 for sign-extend

    top->eval();

    uint32_t imm = (top->instr >> 13) & 0xFFFF;
    int32_t expected = signExtend(imm, 16) << 2;
    EXPECT_EQ(top->ImmOp, expected) << "Expected: " << std::hex << expected;
}

// Default/fallback case
TEST_F(SignExtensionTestbench, DefaultImmediateCase) {
    top->ImmSrc = 0b111;  // Invalid -> fallback to I-type
    top->instr = (1 << 27) | (0x321 << 14);  // MSB=1 for sign-extend

    top->eval();

    int32_t expected = signExtend((top->instr >> 14) & 0x3FFF, 14);
    EXPECT_EQ(top->ImmOp, expected) << "Expected: " << std::hex << expected;
}

// Main entry point
int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");

    auto result = RUN_ALL_TESTS();
    VerilatedCov::write("logs/signextension_coverage.dat");
    return result;
}
