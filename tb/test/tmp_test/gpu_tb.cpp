#include "sync_testbench.h"
#include <verilated.h>
#include <gtest/gtest.h>
#include <verilated_cov.h>

#define NAME "gpu"

class GPUTestbench : public SyncTestbench {
protected:
    void initializeInputs() override {
        top->clk = 1;
        top->reset = 1;
    }
};

TEST_F(GPUTestbench, ResetStateTest) {
    runSimulation(2);  // Apply reset
    EXPECT_EQ(top->reset, 1);
    top->reset = 0;
    runSimulation(1);
}

TEST_F(GPUTestbench, StartupTest) {
    top->reset = 0;
    runSimulation(10);  // Let the warp scheduler initialise
    SUCCEED(); // For now just check it doesn’t crash
}

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    int result = RUN_ALL_TESTS();
    VerilatedCov::write(("logs/coverage_" + std::string(NAME) + ".dat").c_str());
    return result;
}
