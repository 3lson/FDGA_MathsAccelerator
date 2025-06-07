#include "sync_testbench.h"
#include <verilated_cov.h>
#include <bitset>

#define NAME "lsu"

#define WARP_IDLE        0
#define WARP_REQUEST     3
#define WARP_UPDATE      6

#define LSU_IDLE         0
#define LSU_REQUESTING   1
#define LSU_WAITING      2
#define LSU_DONE         3

class LSUTestbench : public SyncTestbench {
protected:
    void initializeInputs() override {
        top->clk = 1;
        top->reset = 1;
        top->enable = 1;
        top->decoded_mem_read_enable = 0;
        top->decoded_mem_write_enable = 0;
        top->rs1 = 0;
        top->rs2 = 0;
        top->imm = 0;
        top->warp_state = WARP_IDLE;
        top->mem_read_ready = 0;
        top->mem_read_data = 0;
        top->mem_write_ready = 0;
    }
};

TEST_F(LSUTestbench, ResetState) {
    runSimulation(1);
    EXPECT_EQ(top->lsu_state, LSU_IDLE);
    EXPECT_EQ(top->mem_read_valid, 0);
    EXPECT_EQ(top->mem_write_valid, 0);
}

TEST_F(LSUTestbench, MemoryLoadTest) {
    top->reset = 0;
    top->decoded_mem_read_enable = 1;
    top->rs1 = 0x100;
    top->imm = 0x20;
    top->warp_state = WARP_REQUEST;

    runSimulation(1);  // Transition to REQUESTING
    EXPECT_EQ(top->lsu_state, LSU_REQUESTING);

    runSimulation(1);  // Transition to WAITING
    EXPECT_EQ(top->mem_read_valid, 1);
    EXPECT_EQ(top->mem_read_address, 0x120);

    top->mem_read_ready = 1;
    top->mem_read_data = 0xDEADBEEF;
    runSimulation(1);  // Complete read

    EXPECT_EQ(top->lsu_out, 0xDEADBEEF);
    EXPECT_EQ(top->lsu_state, LSU_DONE);
}

TEST_F(LSUTestbench, MemoryStoreTest) {
    top->reset = 0;
    top->decoded_mem_write_enable = 1;
    top->rs1 = 0x100;
    top->imm = 0x10;
    top->rs2 = 0xCAFEBABE;
    top->warp_state = WARP_REQUEST;

    runSimulation(1);  // Transition to REQUESTING
    EXPECT_EQ(top->lsu_state, LSU_REQUESTING);

    runSimulation(1);  // Write signal active
    EXPECT_EQ(top->mem_write_valid, 1);
    EXPECT_EQ(top->mem_write_address, 0x110);
    EXPECT_EQ(top->mem_write_data, 0xCAFEBABE);

    top->mem_write_ready = 1;
    runSimulation(1);  // Complete write
    EXPECT_EQ(top->lsu_state, LSU_DONE);
}

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    auto res = RUN_ALL_TESTS();
    VerilatedCov::write(("logs/coverage_" + std::string(NAME) + ".dat").c_str());
    return res;
}
