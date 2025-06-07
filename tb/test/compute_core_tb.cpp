#include "sync_testbench.h"
#include <verilated_cov.h>
#include <bitset>
#include <cstdint> // Required for uint32_t

#define NAME "compute_core"

// Define warp states for clarity in debugging
#define WARP_IDLE      0
#define WARP_FETCH     1
#define WARP_DECODE    2
#define WARP_REQUEST   3
#define WARP_WAIT      4
#define WARP_EXECUTE   5
#define WARP_UPDATE    6
#define WARP_DONE      7

// From your design parameters, used for loop bounds
#define WARPS_PER_CORE 4
#define NUM_LSUS       17

class ComputeCoreTestbench : public SyncTestbench {
protected:
    // This method is automatically called by the GTest framework before each test
    void initializeInputs() override {
        // Initialize all single-bit or scalar inputs to a known state
        top->clk = 1;
        top->reset = 1; // Start in reset
        top->start = 0;
        top->block_id = 0;

        // --- Correctly pack the kernel_config_t struct input ---
        // This port is a 128-bit bus, which Verilator splits into a C-array of 4 x 32-bit words.
        // The layout from common.sv is {num_warps_per_block, num_blocks, base_data_address, base_instructions_address}
        // Word 0 = bits [31:0], Word 1 = bits [63:32], etc.
        uint32_t base_instructions_address = 0;
        uint32_t base_data_address         = 0x1000; // Example value
        uint32_t num_blocks                = 1;      // Example value
        uint32_t num_warps_per_block       = 1;

        top->kernel_config[0] = base_instructions_address;
        top->kernel_config[1] = base_data_address;
        top->kernel_config[2] = num_blocks;
        top->kernel_config[3] = num_warps_per_block;

        // --- Correctly initialize packed vector (multi-bit) inputs ---
        // These are exposed as single integer-like variables. We set all bits to 0.
        top->instruction_mem_read_ready = 0;
        top->data_mem_read_ready = 0;
        top->data_mem_write_ready = 0;
        
        // --- Correctly initialize unpacked array inputs ---
        // Verilator exposes these as standard C-style arrays. We loop to set them all to 0.
        for (int i = 0; i < WARPS_PER_CORE; ++i) {
            top->instruction_mem_read_data[i] = 0;
        }

        for (int i = 0; i < NUM_LSUS; ++i) {
            top->data_mem_read_data[i] = 0;
        }
    }
};

// Test Case 1: Verifies the behavior of the core during and after reset.
TEST_F(ComputeCoreTestbench, ResetBehavior) {
    // Hold reset for 2 cycles to ensure all registers are reset
    runSimulation(2);

    // After reset, 'done' should be low
    EXPECT_EQ(top->done, 0);

    // All 'valid' signals (outputs from the core) should be low
    EXPECT_EQ(top->instruction_mem_read_valid, 0);
    EXPECT_EQ(top->data_mem_read_valid, 0);
    EXPECT_EQ(top->data_mem_write_valid, 0);
}

// Test Case 2: Verifies the startup sequence and the first instruction fetch.
TEST_F(ComputeCoreTestbench, StartExecutionTriggersWarpFetch) {
    // Step 1: Come out of reset. The core is now waiting for a start signal.
    top->reset = 0;
    runSimulation(1);

    // Step 2: Assert 'start' for exactly one cycle to kick off the core.
    top->start = 1;
    runSimulation(1);
    top->start = 0;

    // After the start pulse, the FSM in the DUT should transition warp 0 to the
    // WARP_FETCH state. The `fetcher` submodule for warp 0 will see this state
    // and should assert `instruction_mem_read_valid` for warp 0.

    // Step 3: Poll for the expected output with a timeout.
    // This is a robust way to test asynchronous handshakes. Instead of assuming
    // the signal will be high on a specific cycle, we wait for it.
    int timeout = 10; // Wait for a maximum of 10 cycles.
    while (timeout > 0) {
        // Check if bit 0 of the 'valid' signal is high
        if ((top->instruction_mem_read_valid >> 0) & 1) {
            break; // Exit the loop if the signal is asserted
        }
        runSimulation(1); // Otherwise, advance to the next cycle
        timeout--;
    }

    // Step 4: Verify the results.
    // The most important check: did we time out?
    EXPECT_GT(timeout, 0) << "TEST FAILED: Timeout waiting for instruction fetch. The core never requested an instruction.";
    
    // If we didn't time out, check that the signals are correct.
    EXPECT_EQ((top->instruction_mem_read_valid >> 0) & 1, 1) << "Instruction read valid for warp 0 was not asserted.";
    EXPECT_EQ(top->instruction_mem_read_address[0], 0) << "The fetch address for the first instruction should be 0.";
    
    // The core should certainly not be 'done' yet.
    EXPECT_EQ(top->done, 0);
}

// Main function to run all tests
int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    auto result = RUN_ALL_TESTS();
    VerilatedCov::write(("logs/coverage_" + std::string(NAME) + ".dat").c_str());
    return result;
}

