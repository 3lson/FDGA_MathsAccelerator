#include "base_testbench.h"
#include <verilated_cov.h>
#include <gtest/gtest.h>
#include <bit>

//ALUOps
#define ALU_ADD 0b0000 
#define ALU_SUB 0b0001 
#define ALU_MUL 0b0010  
#define ALU_DIV 0b0011 
#define ALU_SLT 0b0100 
#define ALU_SEQ 0b0101 
#define ALU_MIN 0b0110
#define ALU_ABS 0b0111  



class ALUTestbench : public BaseTestbench {  // Use template for signextension
protected:
    void initializeInputs() override {
        // Initialize ALU inputs to 0
        top->ALUop1 = 0;
        top->ALUop2 = 0;
        top->ALUctrl = 0;
    }
};

TEST_F(ALUTestbench,AddTest) {
    int op1 = 5;
    int op2 = 10;
    
    // Set inputs for addition operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = ALU_ADD;

    top->eval();

    // Check the ALU result and EQ signal for addition
    EXPECT_EQ(top->Result, op1 + op2);
    EXPECT_EQ(top->EQ, 0);
}

TEST_F(ALUTestbench, SubtractionTest)
{
    int op1 = 5;
    int op2 = 5;
    
    // Set inputs for subtraction operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = ALU_SUB;

    top->eval();

    // Check the ALU result and EQ signal for subtraction
    EXPECT_EQ(top->Result, op1 - op2);
    EXPECT_EQ(top->EQ, 0);
}

TEST_F(ALUTestbench, MulTest)
{
    int op1 = 5;
    int op2 = 5;
    
    // Set inputs for subtraction operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = ALU_MUL;

    top->eval();

    // Check the ALU result and EQ signal for subtraction
    EXPECT_EQ(top->Result, op1 * op2);
    EXPECT_EQ(top->EQ, 0);
}

TEST_F(ALUTestbench, DivTest){
    int op1 = 5;
    int op2 = 5;
    
    // Set inputs for subtraction operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = ALU_DIV;

    top->eval();

    // Check the ALU result and EQ signal for subtraction
    EXPECT_EQ(top->Result, op1 / op2);
    EXPECT_EQ(top->EQ, 0);
}

TEST_F(ALUTestbench, LessThanTestTrue){
    int op1 = 3;
    int op2 = 5;
    
    // Set inputs for subtraction operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = ALU_SLT;

    top->eval();

    // Check the ALU result and EQ signal for subtraction
    EXPECT_EQ(top->Result, 1);
    EXPECT_EQ(top->EQ, 0);
}

TEST_F(ALUTestbench, LessThanTestFalse){
    int op1 = 8;
    int op2 = 5;
    
    // Set inputs for subtraction operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = ALU_SLT;

    top->eval();

    // Check the ALU result and EQ signal for subtraction
    EXPECT_EQ(top->Result, 0);
    EXPECT_EQ(top->EQ, 0);
}

TEST_F(ALUTestbench, EqualToTestFalse){
    int op1 = 8;
    int op2 = 5;
    
    // Set inputs for subtraction operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = ALU_SEQ;

    top->eval();

    // Check the ALU result and EQ signal for subtraction
    EXPECT_EQ(top->Result, 0);
    EXPECT_EQ(top->EQ, 0);
}

TEST_F(ALUTestbench, EqualToTestTrue){
    int op1 = 5;
    int op2 = 5;
    
    // Set inputs for Equal to operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = ALU_SEQ;

    top->eval();

    // Check the ALU result and EQ signal for Equal to
    EXPECT_EQ(top->Result, 1);
    EXPECT_EQ(top->EQ, 1);
}

TEST_F(ALUTestbench, MinTestOp1){
    int op1 = 3;
    int op2 = 5;
    
    // Set inputs for Minimum operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = ALU_MIN;

    top->eval();

    // Check the ALU result and EQ signal for Minimum
    EXPECT_EQ(top->Result, op1);
    EXPECT_EQ(top->EQ, 0);
}

TEST_F(ALUTestbench, MinTestOp2){
    int op1 = 8;
    int op2 = 5;
    
    // Set inputs for Minimum operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = ALU_MIN;

    top->eval();

    // Check the ALU result and EQ signal for Minimum
    EXPECT_EQ(top->Result, op2);
    EXPECT_EQ(top->EQ, 0);
}

TEST_F(ALUTestbench, AbsTest){
    float op1 = -8.0f;
    int op2 = 5;
    
    // Set inputs for absolute operation
    top->ALUop1 = 0xc1000000;
    top->ALUop2 = op2;
    top->ALUctrl = ALU_ABS;

    top->eval();

    // Check the ALU result and EQ signal for absolute
    EXPECT_EQ(top->Result, 0x41000000);
    EXPECT_EQ(top->EQ, 0);
}



int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);

    Verilated::mkdir("logs");
    auto result = RUN_ALL_TESTS();
    VerilatedCov::write("logs/coverage.dat");

    return result;
}