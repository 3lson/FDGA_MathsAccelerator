#include "base_testbench.h"
#include <verilated_cov.h>
#include <bitset>
#include <iostream>
#include <iomanip>

#define NAME "decoder"

// Opcode definitions (matching the SystemVerilog defines)
#define OPCODE_R    0b000
#define OPCODE_I    0b001
#define OPCODE_F    0b010
#define OPCODE_M    0b100
#define OPCODE_UP   0b011
#define OPCODE_J    0b111
#define OPCODE_HALT 0b110

// ALU instruction encodings (from common.sv)
#define ADD     0
#define SUB     1
#define MUL     2
#define DIV     3
#define SLT     4
#define SEQ     6
#define SNEZ    7
#define MIN     8
#define ABS     9
#define ADDI    10
#define MULI    11
#define DIVI    12
#define SLLI    13
#define FADD    14
#define FSUB    15
#define FMUL    16
#define FDIV    17
#define FLT     18
#define FNEG    19
#define FEQ     20
#define FMIN    21
#define FABS    22
#define FCVT_W_S 23
#define FCVT_S_W 24
#define BEQZ     25
#define JAL     26

// Reg input mux encodings
#define ALU_OUT         0
#define LSU_OUT         1
#define PC_PLUS_1       3
#define IMMEDIATE       2
#define VECTOR_TO_SCALAR 4

// Warp states
#define WARP_DECODE 2

class DecoderTestbench : public BaseTestbench {
protected:
    void initializeInputs() override {
        top->clk = 0;
        top->reset = 0;
        top->warp_state = 0;
        top->instruction = 0;
    }

    // Helper to create instruction with various fields
    uint32_t makeInstr(uint8_t opcode, uint8_t funct4 = 0, uint8_t funct3 = 0, 
                       uint8_t rd = 0, uint8_t rs1 = 0, uint8_t rs2 = 0,
                       uint32_t immediate = 0, bool scalar = false) {
        uint32_t instr = 0;
        instr |= (opcode & 0x7) << 29;        // bits 31-29
        instr |= (funct3 & 0x7) << 12;        // bits 14-12
        instr |= (funct4 & 0xF) << 10;        // bits 13-10
        instr |= (rd & 0x1F);                 // bits 4-0
        instr |= (rs1 & 0x1F) << 5;           // bits 9-5
        instr |= (rs2 & 0x1F) << 14;          // bits 18-14
        instr |= scalar ? (1 << 6) : 0;       // bit 6
        
        // Add immediate based on instruction type
        if (opcode == OPCODE_I) {
            instr |= (immediate & 0x3FFF) << 14; // I-type immediate bits 27-14
        } else if (opcode == OPCODE_M && funct4 == 0) {
            instr |= (immediate & 0x7FFF) << 14; // Load immediate bits 28-14
        } else if (opcode == OPCODE_M && funct4 == 1) {
            // Store immediate: bits 28-19 and 4-0
            instr |= ((immediate >> 5) & 0x3FF) << 19;
            instr |= (immediate & 0x1F);
        } else if (opcode == OPCODE_J && funct3 == 1) {
            // Branch immediate: bits 28-19, bit 13, bits 4-0, 2'b00
            instr |= ((immediate >> 7) & 0x3FF) << 19;
            instr |= ((immediate >> 6) & 0x1) << 13;
            instr |= ((immediate >> 2) & 0x1F);
        } else if (opcode == OPCODE_J && funct3 == 0) {
            // JAL immediate: bits 28-13, bits 9-0, 2'b00
            instr |= ((immediate >> 12) & 0xFFFF) << 13;
            instr |= ((immediate >> 2) & 0x3FF);
        } else if (opcode == OPCODE_UP) {
            // Upper immediate bits 28-9
            instr |= (immediate & 0xFFFFF) << 9;
        }
        
        return instr;
    }

    void clockCycle() {
        top->clk = 0;
        top->eval();
        top->clk = 1;
        top->eval();
    }

    void resetDecoder() {
        top->reset = 1;
        clockCycle();
        top->reset = 0;
    }

    void decodeInstruction(uint32_t instruction) {
        top->instruction = instruction;
        top->warp_state = WARP_DECODE;
        clockCycle();
    }
};

// ------------------ RESET TEST ------------------
TEST_F(DecoderTestbench, ResetBehavior) {
    resetDecoder();
    
    EXPECT_EQ(top->decoded_reg_write_enable, 0);
    EXPECT_EQ(top->decoded_mem_write_enable, 0);
    EXPECT_EQ(top->decoded_mem_read_enable, 0);
    EXPECT_EQ(top->decoded_branch, 0);
    EXPECT_EQ(top->decoded_reg_input_mux, ALU_OUT);
    EXPECT_EQ(top->decoded_immediate, 0);
    EXPECT_EQ(top->decoded_rd_address, 0);
    EXPECT_EQ(top->decoded_rs1_address, 0);
    EXPECT_EQ(top->decoded_rs2_address, 0);
    EXPECT_EQ(top->decoded_alu_instruction, ADDI);
    EXPECT_EQ(top->decoded_halt, 0);
    EXPECT_EQ(top->decoded_scalar_instruction, 0);
}

// ------------------ R-TYPE TESTS ------------------
TEST_F(DecoderTestbench, RTypeInstructions) {
    struct TestCase {
        uint8_t funct4;
        uint8_t expectedALU;
        const char* name;
    };

    std::vector<TestCase> tests = {
        {0b0000, ADD, "ADD"},
        {0b0001, SUB, "SUB"},
        {0b0010, MUL, "MUL"},
        {0b0011, DIV, "DIV"},
        {0b0100, SLT, "SLT"},
        {0b0110, SEQ, "SEQ"},
        {0b0111, SNEZ, "SNEZ"},
        {0b1000, MIN, "MIN"},
        {0b1001, ABS, "ABS"}
    };

    for (auto& test : tests) {
        resetDecoder();
        uint32_t instr = makeInstr(OPCODE_R, test.funct4, 0, 5, 10, 15); // rd=5, rs1=10, rs2=15
        decodeInstruction(instr);
        
        EXPECT_EQ(top->decoded_alu_instruction, test.expectedALU) << "Failed for " << test.name;
        EXPECT_EQ(top->decoded_reg_write_enable, 1) << "RegWrite failed for " << test.name;
        EXPECT_EQ(top->decoded_reg_input_mux, ALU_OUT) << "RegInputMux failed for " << test.name;
        EXPECT_EQ(top->decoded_rd_address, 5) << "RD address failed for " << test.name;
        EXPECT_EQ(top->decoded_rs1_address, 10) << "RS1 address failed for " << test.name;
        EXPECT_EQ(top->decoded_rs2_address, 15) << "RS2 address failed for " << test.name;
        EXPECT_EQ(top->decoded_scalar_instruction, 0) << "Scalar flag failed for " << test.name;
    }
}

// ------------------ I-TYPE TESTS ------------------
TEST_F(DecoderTestbench, ITypeInstructions) {
    struct TestCase {
        uint8_t funct4;
        uint8_t expectedALU;
        const char* name;
    };

    std::vector<TestCase> tests = {
        {0b0000, ADDI, "ADDI"},
        {0b0010, MULI, "MULI"},
        {0b0011, DIVI, "DIVI"},
        {0b1010, SLLI, "SLLI"}
    };

    for (auto& test : tests) {
        resetDecoder();
        uint32_t instr = makeInstr(OPCODE_I, test.funct4, 0, 7, 12, 0, 0x1234); // rd=7, rs1=12, imm=0x1234
        decodeInstruction(instr);
        
        EXPECT_EQ(top->decoded_alu_instruction, test.expectedALU) << "Failed for " << test.name;
        EXPECT_EQ(top->decoded_reg_write_enable, 1) << "RegWrite failed for " << test.name;
        EXPECT_EQ(top->decoded_reg_input_mux, ALU_OUT) << "RegInputMux failed for " << test.name;
        EXPECT_EQ(top->decoded_rd_address, 7) << "RD address failed for " << test.name;
        EXPECT_EQ(top->decoded_rs1_address, 12) << "RS1 address failed for " << test.name;
        EXPECT_EQ(top->decoded_scalar_instruction, 0) << "Scalar flag failed for " << test.name;
        
        // Check sign extension of 14-bit immediate
        int32_t expected_imm = (int32_t)(int16_t)((0x1234 << 2) >> 2); // Sign extend 14-bit
        EXPECT_EQ((int32_t)top->decoded_immediate, expected_imm) << "Immediate failed for " << test.name;
    }
}

// ------------------ F-TYPE (Floating Point) TESTS ------------------
TEST_F(DecoderTestbench, FTypeInstructions) {
    struct TestCase {
        uint8_t funct4;
        uint8_t expectedALU;
        const char* name;
    };

    std::vector<TestCase> tests = {
        {0b0000, FADD, "FADD"},
        {0b0001, FSUB, "FSUB"},
        {0b0010, FMUL, "FMUL"},
        {0b0011, FDIV, "FDIV"},
        {0b0100, FLT, "FLT"},
        {0b0101, FNEG, "FNEG"},
        {0b0110, FEQ, "FEQ"},
        {0b0111, FMIN, "FMIN"},
        {0b1000, FABS, "FABS"},
        {0b1001, FCVT_W_S, "FCVT_W_S"},
        {0b1010, FCVT_S_W, "FCVT_S_W"}
    };

    for (auto& test : tests) {
        resetDecoder();
        uint32_t instr = makeInstr(OPCODE_F, test.funct4, 0, 3, 8, 13); // rd=3, rs1=8, rs2=13
        decodeInstruction(instr);
        
        EXPECT_EQ(top->decoded_alu_instruction, test.expectedALU) << "Failed for " << test.name;
        EXPECT_EQ(top->decoded_reg_write_enable, 1) << "RegWrite failed for " << test.name;
        EXPECT_EQ(top->decoded_reg_input_mux, ALU_OUT) << "RegInputMux failed for " << test.name;
        EXPECT_EQ(top->decoded_rd_address, 3) << "RD address failed for " << test.name;
        EXPECT_EQ(top->decoded_rs1_address, 8) << "RS1 address failed for " << test.name;
        EXPECT_EQ(top->decoded_scalar_instruction, 0) << "Scalar flag failed for " << test.name;
    }
}

// ------------------ M-TYPE LOAD TEST ------------------
TEST_F(DecoderTestbench, MTypeLoad) {
    resetDecoder();
    uint32_t instr = makeInstr(OPCODE_M, 0b0000, 0, 4, 9, 0, 0x2ABC); // Load: rd=4, rs1=9, imm=0x2ABC
    decodeInstruction(instr);
    
    EXPECT_EQ(top->decoded_alu_instruction, ADDI);
    EXPECT_EQ(top->decoded_reg_write_enable, 1);
    EXPECT_EQ(top->decoded_reg_input_mux, LSU_OUT);
    EXPECT_EQ(top->decoded_rd_address, 4);
    EXPECT_EQ(top->decoded_rs1_address, 9);
    EXPECT_EQ(top->decoded_mem_read_enable, 1);
    EXPECT_EQ(top->decoded_mem_write_enable, 0);
    
    // Check sign extension of 15-bit immediate
    int32_t expected_imm = (int32_t)(int16_t)((0x2ABC << 1) >> 1); // Sign extend 15-bit
    EXPECT_EQ((int32_t)top->decoded_immediate, expected_imm);
}

// ------------------ M-TYPE STORE TEST ------------------
TEST_F(DecoderTestbench, MTypeStore) {
    resetDecoder();
    uint32_t instr = makeInstr(OPCODE_M, 0b0001, 0, 0, 11, 16, 0x1DEF); // Store: rs1=11, rs2=16, imm=0x1DEF
    decodeInstruction(instr);
    
    EXPECT_EQ(top->decoded_alu_instruction, ADDI);
    EXPECT_EQ(top->decoded_reg_write_enable, 0);
    EXPECT_EQ(top->decoded_rs1_address, 11);
    EXPECT_EQ(top->decoded_rs2_address, 16);
    EXPECT_EQ(top->decoded_mem_read_enable, 0);
    EXPECT_EQ(top->decoded_mem_write_enable, 1);
    
    // Check sign extension of 15-bit immediate
    int32_t expected_imm = (int32_t)(int16_t)((0x1DEF << 1) >> 1); // Sign extend 15-bit
    EXPECT_EQ((int32_t)top->decoded_immediate, expected_imm);
}

// ------------------ UPPER IMMEDIATE TEST ------------------
TEST_F(DecoderTestbench, UpperImmediate) {
    resetDecoder();
    uint32_t instr = makeInstr(OPCODE_UP, 0, 0, 6, 0, 0, 0x12345); // LUI: rd=6, imm=0x12345
    decodeInstruction(instr);
    
    EXPECT_EQ(top->decoded_reg_write_enable, 1);
    EXPECT_EQ(top->decoded_reg_input_mux, IMMEDIATE);
    EXPECT_EQ(top->decoded_rd_address, 6);
    
    // Upper immediate should be shifted left 12 bits
    uint32_t expected_imm = (0x12345 & 0xFFFFF) << 12;
    EXPECT_EQ(top->decoded_immediate, expected_imm);
}

// ------------------ JUMP BRANCH TEST ------------------
TEST_F(DecoderTestbench, JumpBranch) {
    resetDecoder();
    /*
        uint32_t makeInstr(uint8_t opcode, uint8_t funct4 = 0, uint8_t funct3 = 0, 
                       uint8_t rd = 0, uint8_t rs1 = 0, uint8_t rs2 = 0,
                       uint32_t immediate = 0, bool scalar = false) {
        uint32_t instr = 0;
        instr |= (opcode & 0x7) << 29;        // bits 31-29
        instr |= (funct3 & 0x7) << 12;        // bits 14-12
        instr |= (funct4 & 0xF) << 10;        // bits 13-10
        instr |= (rd & 0x1F);                 // bits 4-0
        instr |= (rs1 & 0x1F) << 5;           // bits 9-5
        instr |= (rs2 & 0x1F) << 14;          // bits 18-14
        instr |= scalar ? (1 << 6) : 0;       // bit 6
    */
    uint32_t instr = makeInstr(OPCODE_J, 0, 0b001, 0, 7, 14, 0x0800); // Branch: rs1=7, rs2=14, imm=0x1000
    decodeInstruction(instr);
    
    EXPECT_EQ(top->decoded_alu_instruction, BEQ);
    EXPECT_EQ(top->decoded_rs1_address, 7);
    EXPECT_EQ(top->decoded_rs2_address, 14);
    EXPECT_EQ(top->decoded_branch, 1);
    EXPECT_EQ(top->decoded_reg_write_enable, 0);
    
    // Check sign extension of 18-bit branch immediate
    int32_t expected_imm = (int32_t)((0x1000 << 14) >> 14); // Sign extend 18-bit
    EXPECT_EQ((int32_t)top->decoded_immediate, expected_imm);
}

// ------------------ JUMP AND LINK TEST ------------------
TEST_F(DecoderTestbench, JumpAndLink) {
    resetDecoder();
    // OPCODE_R, test.funct4, 0, 5, 10, 15
    uint32_t instr = makeInstr(OPCODE_J, 0, 0b000, 0, 8, 0, 0);
    decodeInstruction(instr);
    
    EXPECT_EQ(top->decoded_alu_instruction, JAL);
    
    // Check sign extension of 28-bit jump immediate
    int32_t expected_imm = (int32_t)((0b100000000 << 6) >> 4); // Sign extend 28-bit
    EXPECT_EQ((int32_t)top->decoded_immediate, expected_imm);
}

// ------------------ HALT TEST ------------------
TEST_F(DecoderTestbench, HaltInstruction) {
    resetDecoder();
    uint32_t instr = makeInstr(OPCODE_HALT);
    decodeInstruction(instr);
    
    EXPECT_EQ(top->decoded_halt, 1);
    EXPECT_EQ(top->decoded_reg_write_enable, 0);
}

// Not yet implemented

// // ------------------ SCALAR FLAG TEST ------------------
// TEST_F(DecoderTestbench, ScalarFlagTest) {
//     resetDecoder();
//     uint32_t instr = makeInstr(OPCODE_R, 0b0000, 0, 1, 2, 3, 0, true); // Scalar R-type
//     decodeInstruction(instr);
    
//     EXPECT_EQ(top->decoded_scalar_instruction, 1);
//     EXPECT_EQ(top->decoded_alu_instruction, ADD);
//     EXPECT_EQ(top->decoded_reg_write_enable, 1);
// }

// ------------------ DEFAULT CASE TEST ------------------
TEST_F(DecoderTestbench, DefaultCase) {
    resetDecoder();
    uint32_t instr = makeInstr(0b101); // Invalid opcode
    decodeInstruction(instr);
    
    // Should maintain default values set during WARP_DECODE
    EXPECT_EQ(top->decoded_reg_write_enable, 0);
    EXPECT_EQ(top->decoded_mem_write_enable, 0);
    EXPECT_EQ(top->decoded_mem_read_enable, 0);
    EXPECT_EQ(top->decoded_branch, 0);
    EXPECT_EQ(top->decoded_halt, 0);
}

// ------------------ WARP STATE TEST ------------------
TEST_F(DecoderTestbench, WarpStateControl) {
    resetDecoder();
    
    // Test that decoder only responds during WARP_DECODE state
    top->instruction = makeInstr(OPCODE_R, 0b0000, 0, 1, 2, 3);
    top->warp_state = 0; // Not WARP_DECODE
    clockCycle();
    
    // Should maintain reset values
    EXPECT_EQ(top->decoded_reg_write_enable, 0);
    EXPECT_EQ(top->decoded_alu_instruction, ADDI);
    
    // Now test with correct warp state
    top->warp_state = WARP_DECODE;
    clockCycle();
    
    EXPECT_EQ(top->decoded_reg_write_enable, 1);
    EXPECT_EQ(top->decoded_alu_instruction, ADD);
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