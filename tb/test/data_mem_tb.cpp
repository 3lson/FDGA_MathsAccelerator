#include "sync_testbench.h"
#include <verilated_cov.h>
#include <gtest/gtest.h>

/*KEYNOTE: LOAD instructions depend on storing data: 0x12345678 and 0xFF88654 into data_memory locations
mem[3:0] = 0x12345678 and mem[7:4] = 0xFF886543, it will fail otherwise.
Whilst running other tests that require loading instructions into datamem failures are expected.
so dont worry */

#define NAME            "data_mem"

class DataMembench : public SyncTestbench {
protected:
    void initializeInputs() override {
        top->clk = 1;

        //stores word instructions
        top->AddrMode = 0b111;
        top->A = 0;
        top->WD = 0x12345678;
        runSimulation(1);
        top->AddrMode = 0b111;
        top->A = 4;
        top->WD = 0xFF886543;
        runSimulation(1);
    }
};

//tests load byte instruction: lb rd, imm(rs1)
TEST_F(DataMembench, AddrMode000Test) {
    top->AddrMode = 0b000;
    top->A = 0;

    top->eval();

    EXPECT_EQ(top->RD, 0x00000078);
}

//tests load half word instruction: lh rd, imm(rs1)
TEST_F(DataMembench, AddrMode001Test) {
    top->AddrMode = 0b001;
    top->A = 6;

    top->eval();

    EXPECT_EQ(top->RD, 0xFFFFFF88);
}


//tests load word instruction: lw rd, imm(rs1)
TEST_F(DataMembench, AddrMode010Test) {
    top->AddrMode = 0b010;
    top->A = 0;

    top->eval();

    EXPECT_EQ(top->RD, 0x12345678);
}

//tests load byte unsigned instruction: lbu rd, imm(rs1)
TEST_F(DataMembench, AddrMode011Test) {
    top->AddrMode = 0b011;
    top->A = 6;

    top->eval();

    EXPECT_EQ(top->RD, 0x00000088);
}

//tests load half word unsigned instruction: lhu rd, imm(rs1)
TEST_F(DataMembench, AddrMode100Test) {
    top->AddrMode = 0b100;
    top->A = 6;

    top->eval();

    EXPECT_EQ(top->RD, 0x0000FF88);
}

//tests store byte instruction: sb rs2, imm(rs1)
TEST_F(DataMembench, AddrMode101Test) {
    top->AddrMode = 0b101;
    top->A = 8;
    top->WD = 0x00000078;
    runSimulation(1);


    EXPECT_EQ(top->RD, 0x00000078);
}

//tests store half word instruction: sh rs2, imm(rs1)
TEST_F(DataMembench, AddrMode110Test) {
    top->AddrMode = 0b110;
    top->A = 12;
    top->WD = 0x00005678;
    runSimulation(1);
    
    EXPECT_EQ(top->RD, 0x00005678);
}

//tests store word instruction: sw rs2, imm(rs1)
TEST_F(DataMembench, AddrMode111Test) {
    top->AddrMode = 0b111;
    top->A = 16;
    top->WD = 0x12345678;
    runSimulation(1);

    EXPECT_EQ(top->RD, 0x12345678);
}

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    auto res = RUN_ALL_TESTS();
    //VerilatedCov::write(("logs/coverage_" + std::string(NAME) + ".dat").c_str());

    return res;
}