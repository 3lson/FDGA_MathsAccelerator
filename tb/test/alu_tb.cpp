#include "base_testbench.h"
#include <verilated_cov.h>

#define NAME            "alu"

// Define operation codes for all 10 instructions
#define OPCODE_ADD      0b000
#define OPCODE_SUB      0b001
#define OPCODE_AND      0b010
#define OPCODE_OR       0b011
#define OPCODE_XOR      0b100
#define OPCODE_LSL      0b0101
#define OPCODE_LSR      0b0110
#define OPCODE_ASR      0b0111
#define OPCODE_SLT      0b1000
#define OPCODE_SLTU     0b1001

class ALUTestbench : public BaseTestbench
{
protected:
    void initializeInputs() override
    {
        // Initialize the ALU inputs to 0
        top->ALUop1 = 0;
        top->ALUop2 = 0;
        top->ALUctrl = 0;
    }
};

TEST_F(ALUTestbench, AdditionTest)
{
    int op1 = 5;
    int op2 = 10;

    // Set inputs for addition operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = OPCODE_ADD;

    top->eval();

    // Check the ALU result and EQ signal for addition
    EXPECT_EQ(top->Result, op1 + op2);
    EXPECT_EQ(top->EQ, (op1 + op2 == 0) ? 1 : 0);
}

TEST_F(ALUTestbench, SubtractionTest)
{
    int op1 = 5;
    int op2 = 5;

    // Set inputs for subtraction operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = OPCODE_SUB;

    top->eval();

    // Check the ALU result and EQ signal for subtraction
    EXPECT_EQ(top->Result, op1 - op2);
    EXPECT_EQ(top->EQ, (op1 - op2 == 0) ? 1 : 0);
}

TEST_F(ALUTestbench, BinaryAndTest)
{
    int op1 = 0b0110;
    int op2 = 0b0101;

    // Set inputs for binary AND operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = OPCODE_AND;

    top->eval();

    // Check the ALU result and EQ signal for AND operation
    EXPECT_EQ(top->Result, op1 & op2);
    EXPECT_EQ(top->EQ, ((op1 & op2) == 0) ? 1 : 0);
}

TEST_F(ALUTestbench, BinaryOrTest)
{
    int op1 = 0b0110;
    int op2 = 0b0101;

    // Set inputs for binary OR operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = OPCODE_OR;

    top->eval();

    // Check the ALU result and EQ signal for OR operation
    EXPECT_EQ(top->Result, op1 | op2);
    EXPECT_EQ(top->EQ, ((op1 | op2) == 0) ? 1 : 0);
}

TEST_F(ALUTestbench, BinaryXorTest)
{
    int op1 = 0b0110;
    int op2 = 0b0101;

    // Set inputs for binary XOR operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = OPCODE_XOR;

    top->eval();

    // Check the ALU result and EQ signal for XOR operation
    EXPECT_EQ(top->Result, op1 ^ op2);
    EXPECT_EQ(top->EQ, ((op1 ^ op2) == 0) ? 1 : 0);
}

TEST_F(ALUTestbench, LogicalShiftLeftTest)
{
    int op1 = 0b00000000000000000000000000001010; // 10 in binary
    int op2 = 2;

    // Set inputs for Logical Shift Left operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = OPCODE_LSL;

    top->eval();

    // Check the ALU result for LSL operation
    EXPECT_EQ(top->Result, op1 << op2);
}

TEST_F(ALUTestbench, LogicalShiftRightTest)
{
    int op1 = 0b00000000000000000000000000001010; // 10 in binary
    int op2 = 2;

    // Set inputs for Logical Shift Right operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = OPCODE_LSR;

    top->eval();

    // Check the ALU result for LSR operation
    EXPECT_EQ(top->Result, op1 >> op2);
}

TEST_F(ALUTestbench, ArithmeticShiftRightTest)
{
    int op1 = 0b11111111111111111111111111101010; // -22 in two's complement
    int op2 = 2;

    // Set inputs for Arithmetic Shift Right operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = OPCODE_ASR;

    top->eval();

    // Check the ALU result for ASR operation
    EXPECT_EQ(top->Result, op1 >> op2);
}

TEST_F(ALUTestbench, SetIfLessThanTest)
{
    int op1 = 0b0110; // 6
    int op2 = 0b0101; // 5

    // Set inputs for SLT (Set Less Than) operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = OPCODE_SLT;

    top->eval();

    // Check the ALU result for SLT operation
    EXPECT_EQ(top->Result, (op1 < op2) ? 0b1 : 0b0);
    EXPECT_EQ(top->EQ, ((op1 < op2) ? 0b1 : 0b0) == 0 ? 1 : 0);
}

TEST_F(ALUTestbench, SetIfLessThanUnsignedTest)
{
    int op1 = 0b0110; // 6
    int op2 = 0b0101; // 5

    // Set inputs for SLTU (Set Less Than Unsigned) operation
    top->ALUop1 = op1;
    top->ALUop2 = op2;
    top->ALUctrl = OPCODE_SLTU;

    top->eval();

    // Check the ALU result for SLTU operation
    EXPECT_EQ(top->Result, (op1 < op2) ? 0b1 : 0b0);
    EXPECT_EQ(top->EQ, ((op1 < op2) ? 0b1 : 0b0) == 0 ? 1 : 0);
}

int main(int argc, char **argv)
{
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    auto res = RUN_ALL_TESTS();
    VerilatedCov::write(("logs/coverage_" + std::string(NAME) + ".dat").c_str());

    return res;
}
