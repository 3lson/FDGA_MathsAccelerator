#include "base_testbench.h"
#include <verilated_cov.h>
#include <gtest/gtest.h>

class SignExtensionTestbench : public BaseTestbench {  // Use template for signextension
protected:
    void initializeInputs() override {
        // Initialize DUT inputs to default values
        top->instr = 0;
        top->ImmSrc = 0;
    }
};

TEST_F(SignExtensionTestbench, ImmSrc000Test) {
    top->instr = 0xFFF00013;  // Example I-type instruction: 0xFFF represents a negative immediate
    top->ImmSrc = 0b000;

    top->eval();

    // Expected output: Sign-extended 12-bit immediate field (0xFFFFFFFFFFF)
    EXPECT_EQ(top->ImmOp, 0xFFFFFFFF);
}

TEST_F(SignExtensionTestbench, ImmSrc001Test) {
    top->instr = 0xFE000FA3;  // Example S-type instruction
    top->ImmSrc = 0b001;

    top->eval();

    // Expected output: Sign-extended instruction for S-type immediate
    EXPECT_EQ(top->ImmOp, 0xFFFFFFFF);
}

TEST_F(SignExtensionTestbench, ImmSrc010Test) {
    top->ImmSrc = 0b010; 
    top->instr = 0xFE420AE3;  // Example branch instruction beq x4, x4, L7

    top->eval();    

    // Expected output: Sign-extended instruction for B-type immediate
    EXPECT_EQ(top->ImmOp, 0xFFFFFFF4);  // expected PC offset to enable branch
}

TEST_F(SignExtensionTestbench, ImmSrc011Test) {
    top->ImmSrc = 0b011;  
    top->instr = 0xFFFFF4B7;  // Example U-type instruction: lui s1 (x9) <constant> | constant = 1048575 (0xFFFFF)

    top->eval();    

    // Expected output: Sign-extended instruction for B-type immediate
    EXPECT_EQ(top->ImmOp, 0xFFFFF000);  // expected PC offset to enable branch
}

TEST_F(SignExtensionTestbench, ImmSrc100Test) {
    top->ImmSrc = 0b100;  
    top->instr = 0xFEDFF0EF;  // Example J-type instruction: JAL ra (x1) <label> | PC relative offset = -20

    top->eval();    

    // Expected output: Sign-extended instruction for B-type immediate
    EXPECT_EQ(top->ImmOp, 0xFFFFFFEC);  // expected PC offset to enable branch
}

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);

    Verilated::mkdir("logs");
    auto result = RUN_ALL_TESTS();
    VerilatedCov::write("logs/coverage.dat");

    return result;
}