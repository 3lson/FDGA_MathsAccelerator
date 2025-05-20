#include "base_testbench.h"
#include <verilated_cov.h>
#include <bitset>

#define NAME            "control_unit"

#define OPCODE_I1       0b001'0011          // addi, xori, ori... (ALU imm)
#define OPCODE_I2       0b000'0011          // lb, lh, lw... (load imm)
#define OPCODE_I3       0b110'0111          // jalr
#define OPCODE_I4       0b111'0011          // ecall, ebreak

#define OPCODE_U1       0b011'0111          // lui (load upper imm)
#define OPCODE_U2       0b001'0111          // auipc (add imm to pc)

#define OPCODE_S        0b010'0011          // sb, sh, sw (store)
#define OPCODE_B        0b110'0011          // beq, bne, blt (branch)
#define OPCODE_R        0b011'0011          // add, sub, xor... (register)
#define OPCODE_J        0b110'1111          // jal  (jump and link)

class ControlunitTestbench : public BaseTestbench
{
protected:
    void initializeInputs() override
    {
        top->instr = 0;
        // outputs: 
        // ALUctrl - controls the alu
        // ALUsrc - selects between rd2 (0) and ImmOp (1)
        // ImmSrc - selects between I (0), S (1) and B (2) type
        // RegWrite - enables writing of register
    }
};

TEST_F(ControlunitTestbench, ALUControl)
{
    // ADD (000)    - add, addi, lw, sw
    // SUB (001)    - sub
    // AND (010)    - and
    // OR  (011)    - or
    // SLT (100)    - slt

    // lw should always signify an ADD
    top->instr = OPCODE_I2;
    top->eval();
    EXPECT_EQ(top->ALUctrl, 0) << "Test 1";

    // sw should always signify an ADD
    top->instr = OPCODE_S;
    top->eval();
    EXPECT_EQ(top->ALUctrl, 0) << "Test 2";
    
    // beq should always signify a SUB
    top->instr = OPCODE_B + (0b000 << 12);
    top->eval();
    EXPECT_EQ(top->ALUctrl, 1) << "Test 3";

    // bne should always signify a SUB
    top->instr = OPCODE_B + (0b001 << 12);
    top->eval();
    EXPECT_EQ(top->ALUctrl, 1) << "Test 4";
}

TEST_F(ControlunitTestbench, RegWriteTest) {   
    // Special Case: JALR (opcode 1100111)
    
    // Test JALR with rd = 0 (RegWrite should be 0)
    top->instr = 0;                    // Clear the instruction
    top->instr |= (0b1100111);         // Set opcode to JALR (bits 6:0)
    top->instr |= (0b00000 << 7);      // Set rd = 0 (bits 11:7)
    top->eval();
    std::cout << "JALR with rd = 0, instr = " << std::bitset<32>(top->instr) 
              << ", rd = " << std::bitset<5>((top->instr >> 7) & 0b11111) 
              << ", RegWrite = " << top->RegWrite << std::endl;
    EXPECT_EQ(top->RegWrite, 0) << "JALR with rd = 0 should not write to register";

    // Test JALR with rd != 0 (RegWrite should be 1)
    top->instr = 0;                    // Clear the instruction
    top->instr |= (0b1100111);         // Set opcode to JALR (bits 6:0)
    top->instr |= (0b00001 << 7);      // Set rd = 1 (bits 11:7)
    top->eval();
    std::cout << "JALR with rd = 1, instr = " << std::bitset<32>(top->instr) 
              << ", rd = " << std::bitset<5>((top->instr >> 7) & 0b11111) 
              << ", RegWrite = " << top->RegWrite << std::endl;
    EXPECT_EQ(top->RegWrite, 1) << "JALR with rd != 0 should write to register";
}

TEST_F(ControlunitTestbench, ALUsrcTest)
{
    // ALUsrc = 1 (chooses immediate): I-type, U-type, J-type
    // ALUsrc = 0 (chooses rd2): R-type, S-type, B-type
    
    for (int opcode : { 
        OPCODE_I1, OPCODE_I2, OPCODE_I3, OPCODE_I4, 
        OPCODE_U1, OPCODE_U2, OPCODE_J, OPCODE_S
    }) {
        top->instr = OPCODE_R;
        top->eval();

        top->instr = opcode;
        top->eval();

        EXPECT_EQ(top->ALUsrc, 1) << "Opcode: " << std::bitset<7>(opcode);
    }

    for (int opcode : { OPCODE_R, OPCODE_B }) 
    {
        top->instr = OPCODE_I1;
        top->eval();
        
        top->instr = opcode;
        top->eval();

        EXPECT_EQ(top->ALUsrc, 0) << "Opcode: " << std::bitset<7>(opcode);
    }
}

TEST_F(ControlunitTestbench, ImmSrc0Test)
{   
    // Checks the I instructions
    for (int opcode : { OPCODE_I1, OPCODE_I2, OPCODE_I3, OPCODE_I4 })
    {
        top->instr = opcode;
        top->eval();

        EXPECT_EQ(top->ImmSrc, 0) << "Opcode: " << std::bitset<7>(opcode);
    }
}

TEST_F(ControlunitTestbench, ImmSrc1Test)
{   
    // Checks the S instructions
    top->instr = OPCODE_S;
    top->eval();

    EXPECT_EQ(top->ImmSrc, 1);
}

TEST_F(ControlunitTestbench, ImmSrc2Test)
{   
    // Checks the B instructions
    top->instr = OPCODE_B;
    top->eval();

    EXPECT_EQ(top->ImmSrc, 2);
}

int main(int argc, char **argv)
{
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    auto res = RUN_ALL_TESTS();
    VerilatedCov::write(
        ("logs/coverage_" + std::string(NAME) + ".dat").c_str()
    );

    return res;
}
