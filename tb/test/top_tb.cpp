#include "sync_testbench.h"
#include <verilated_cov.h>

#define NAME "top"

class TopTestbench : public SyncTestbench {
protected:
    bool resetReleased = false;

    void initializeInputs() override {
        top->clk = 1;
        top->rst = 0;
    }

};

TEST_F(TopTestbench, ExecuteSimpleProgram) {
    runSimulation(2);  // Run enough cycles for instructions to complete
    // We are starting at cycle 0 -> 1 -> 2 so right result in 3 cycles
    EXPECT_EQ(top->Result, 40);
}

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");

    int result = RUN_ALL_TESTS();
    VerilatedCov::write(("logs/coverage_" + std::string(NAME) + ".dat").c_str());
    return result;
}
