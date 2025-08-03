#include "sync_testbench.h"
#include <verilated_cov.h>
#include <bitset>
#include <vector>

#define NAME "fetcher_wrapper"

class FetcherWrapperTestbench : public SyncTestbench {
protected:
    void initializeInputs() override {
        top->clk = 1;
        top->reset = 1;
        top->warp_state = 0;               // Assuming IDLE
        top->pc = 0;
        top->instruction_mem_read_ready = 0;
        top->instruction_mem_read_data = 0;
    }
};

TEST_F(FetcherWrapperTestbench, ResetTest) {
    runSimulation(1);
    EXPECT_EQ(top->fetcher_state, 0); // FETCHER_IDLE
    EXPECT_EQ(top->instruction_mem_read_valid, 0);
    EXPECT_EQ(top->instruction_mem_read_address, 0);
    EXPECT_EQ(top->instruction, 0);
}

// Test: Start fetch, then receive instruction
TEST_F(FetcherWrapperTestbench, FetchInstructionTest) {
    top->reset = 0;
    top->warp_state = 1; // WARP_FETCH
    top->pc = 0x10;

    runSimulation(1); // Should initiate read
    EXPECT_EQ(top->instruction_mem_read_valid, 1);
    EXPECT_EQ(top->instruction_mem_read_address, 0x10);

    top->instruction_mem_read_ready = 1;
    top->instruction_mem_read_data = 0x12345678;

    runSimulation(1); // Should latch instruction
    EXPECT_EQ(top->instruction, 0x12345678);
    EXPECT_EQ(top->instruction_mem_read_valid, 0);
    EXPECT_EQ(top->fetcher_state, 2); // FETCHER_DONE

    // Signal decode phase so it returns to IDLE
    top->warp_state = 2; // WARP_DECODE
    runSimulation(1);
    EXPECT_EQ(top->fetcher_state, 0); // Back to IDLE
}

// test illegal state - this will cause error which is expected 
// Comment out to see
// TEST_F(FetcherWrapperTestbench, InvalidStateTest) {
//     top->reset = 0;
//     top->warp_state = 1;
//     top->fetcher_state = 3; // Invalid state
//     runSimulation(1); // Should trigger $error in Verilog sim (not caught in C++)
// }

int main(int argc, char **argv)
{
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    auto res = RUN_ALL_TESTS();

    VerilatedCov::write(("logs/coverage_" + std::string(NAME) + ".dat").c_str());
    return res;
}
