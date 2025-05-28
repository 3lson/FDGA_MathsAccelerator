#include "base_testbench.h"
#include <verilated_cov.h>
#include <bitset>
#include <iostream>
#include <iomanip>

#define NAME "controlunit"

#define RTYPE 0b000
#define ITYPE 0b001
#define PTYPE 0b010
#define MTYPE 0b100
#define XTYPE 0b101
#define CTYPE 0b111

#define ALU_ADD 0b0000
#define ALU_SUB 0b0001
#define ALU_MUL 0b0010
#define ALU_DIV 0b0011
#define ALU_SLT 0b0100
#define ALU_SEQ 0b0110
#define ALU_MIN 0b1000
#define ALU_ABS 0b1001

class ControlunitTestbench : public BaseTestbench {
protected:
    void initializeInputs() override {
        top->instr = 0;
    }

    // Helper to create instruction with op and funct4/funct3
    uint32_t makeInstr(uint8_t op, uint8_t funct = 0) {
        return (op << 29) | (funct << 10);
    }
};

// ------------------ R-TYPE TEST ------------------
TEST_F(ControlunitTestbench, RTypeALUOps) {
    struct TestCase {
        uint8_t funct4;
        uint8_t expectedALUctrl;
    };

    std::vector<TestCase> tests = {
        {ALU_ADD, ALU_ADD}, {ALU_SUB, ALU_SUB}, {ALU_MUL, ALU_MUL},
        {ALU_DIV, ALU_DIV}, {ALU_SLT, ALU_SLT}, {ALU_SEQ, ALU_SEQ},
        {ALU_MIN, ALU_MIN}, {ALU_ABS, ALU_ABS}
    };

    for (auto& test : tests) {
        top->instr = makeInstr(RTYPE, test.funct4);
        top->eval();
        EXPECT_EQ(top->ALUctrl, test.expectedALUctrl);
        EXPECT_EQ(top->RegWrite, 1);
        EXPECT_EQ(top->ALUsrc, 0);
    }
}

// ------------------ I-TYPE TEST ------------------
TEST_F(ControlunitTestbench, ItypeOps) {
    top->instr = makeInstr(ITYPE, ALU_MUL);
    top->eval();
    EXPECT_EQ(top->ALUctrl, ALU_MUL);
    EXPECT_EQ(top->RegWrite, 1);
    EXPECT_EQ(top->ALUsrc, 1);
    EXPECT_EQ(top->ImmSrc, 0);
}

// ------------------ M-TYPE LOAD/STORE TEST ------------------
TEST_F(ControlunitTestbench, MtypeLoadStore) {
    // Load
    top->instr = makeInstr(MTYPE, 0b0000);
    top->eval();
    EXPECT_EQ(top->ALUctrl, ALU_ADD);
    EXPECT_EQ(top->RegWrite, 1);
    EXPECT_EQ(top->ALUsrc, 1);
    EXPECT_EQ(top->ImmSrc, 1);
    EXPECT_EQ(top->ResultSrc, 1);

    // Store
    top->instr = makeInstr(MTYPE, 0b0001);
    top->eval();
    EXPECT_EQ(top->ALUctrl, ALU_ADD);
    EXPECT_EQ(top->RegWrite, 0);
    EXPECT_EQ(top->ALUsrc, 0);
    EXPECT_EQ(top->ImmSrc, 2);
}

// ------------------ DEFAULT CASE ------------------
TEST_F(ControlunitTestbench, DefaultCase) {
    top->instr = makeInstr(0b110); // Invalid op
    top->eval();
    EXPECT_EQ(top->ALUctrl, ALU_ADD);
    EXPECT_EQ(top->RegWrite, 0);
    EXPECT_EQ(top->ALUsrc, 0);
    EXPECT_EQ(top->ImmSrc, 0);
    EXPECT_EQ(top->Jump, 0);       // Replaces PCsrc
    EXPECT_EQ(top->branch, 0);
}

// ------------------ C-TYPE CONTROL TESTS ------------------
TEST_F(ControlunitTestbench, CType_Jump) {
    top->instr = makeInstr(CTYPE, 0b000); // JUMP
    top->eval();
    EXPECT_EQ(top->ImmSrc, 3);
    EXPECT_EQ(top->Jump, 2);  // Jump = 2'b10
    EXPECT_EQ(top->WD3Src, 1);
    EXPECT_EQ(top->RegWrite, 1);
}

TEST_F(ControlunitTestbench, CType_BranchTaken) {
    top->instr = makeInstr(CTYPE, 0b001);
    top->eval();
    EXPECT_EQ(top->ALUctrl, ALU_SEQ);
    EXPECT_EQ(top->branch, 1);
}

TEST_F(ControlunitTestbench, CType_BranchNotTaken) {
    top->instr = makeInstr(CTYPE, 0b001);
    top->eval();
    EXPECT_EQ(top->branch, 1); 
}

TEST_F(ControlunitTestbench, CType_Call) {
    top->instr = makeInstr(CTYPE, 0b010);
    top->eval();
    EXPECT_EQ(top->ImmSrc, 4);
    EXPECT_EQ(top->Jump, 2); // Jump = 2'b10
    EXPECT_EQ(top->RegWrite, 1);
    EXPECT_EQ(top->WD3Src, 1);
}

TEST_F(ControlunitTestbench, CType_Ret) {
    top->instr = makeInstr(CTYPE, 0b011);
    top->eval();
    EXPECT_EQ(top->Jump, 3); // Jump = 2'b11 (ret)
    EXPECT_EQ(top->RegWrite, 0);
}

TEST_F(ControlunitTestbench, CType_Exit) {
    top->instr = makeInstr(CTYPE, 0b111);
    top->eval();
    EXPECT_EQ(top->exit, 1);
    EXPECT_EQ(top->RegWrite, 0);
}

TEST_F(ControlunitTestbench, CType_Sync) {
    top->instr = makeInstr(CTYPE, 0b110);
    top->eval();
    EXPECT_EQ(top->RegWrite, 0);
    EXPECT_EQ(top->exit, 0);
}


// ------------------ MAIN ------------------
int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    auto res = RUN_ALL_TESTS();
    VerilatedCov::write(("logs/coverage_" + std::string(NAME) + ".dat").c_str());
    return res;
}
