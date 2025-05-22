
#include "sync_testbench.h"
#include <gtest/gtest.h>

class TopTestbench : public SyncTestbench {
protected:
    void initializeInputs() override {
        // Start with clk=0, reset asserted
        top->clk = 0;
        top->rst = 1;

        // Let reset propagate for two full clock cycles
        top->rst = 0;
        runSimulation(1);
    }
};

// After one instruction (ADDI x1, x1, 10) we expect Result==10
TEST_F(TopTestbench, ExecutesAddi) {
    runSimulation(1);
    EXPECT_EQ(top->Result, 10)
        << "After the first cycle (ADDI), Result should be 10";
}

// After two instructions (ADD x2, x1, x1), we expect Result==20
TEST_F(TopTestbench, ExecutesAdd) {
    // We’ve already done 1 cycle in ExecutesAddi (but GTest runs each TEST in a fresh fixture),
    // so here we do two cycles from reset-release.
    runSimulation(2);
    EXPECT_EQ(top->Result, 20)
        << "After the second cycle (ADD), Result should be 20";
}

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    Verilated::traceEverOn(true);
    return RUN_ALL_TESTS();
}
