#include "base_testbench.h" // Your provided base class
#include <verilated_cov.h> // For coverage
#include <gtest/gtest.h>   // Google Test framework
#include <vector>          // For convenience
#include <cstdint>         // For standard integer types

// Define the name of the module being tested (for coverage reporting)
#define MODULE_NAME "reg_file"

// Constants for the design parameters (match Verilog compilation)
const int THREADS_PER_WARP = 16; // Must match Verilog parameter
const int DATA_WIDTH = 32;       // Must match Verilog parameter

// C++ equivalents for Verilog enums (values must match Verilog definitions in common.svh)
enum class WarpState : uint8_t {
    IDLE = 0,
    FETCH = 1,
    DECODE = 2,
    REQUEST = 3,
    REG_WAIT = 4,
    WAIT = 5,
    EXECUTE = 6,
    ALU_WAIT = 7,
    INT_ALU_WAIT = 8,
    UPDATE = 9,
    SYNC_WAIT = 10,
    DONE = 11
};
enum class RegInputMux : uint8_t {
    ALU_OUT = 0,
    LSU_OUT = 1,
    IMMEDIATE = 2,
    PC_PLUS_1 = 3,
    VECTOR_TO_SCALAR = 4
};

// Constants for special register addresses
const int ZERO_REG = 0;
const int THREAD_ID_REG = 29;
const int BLOCK_ID_REG = 30;
const int BLOCK_SIZE_REG = 31;

class RegFileTestbench : public BaseTestbench {
protected:
    void tick() {
        top->clk = 0;
        top->eval();
        top->clk = 1;
        top->eval();
    }

    void runSimulation(int cycles) {
        for (int i = 0; i < cycles; ++i) {
            tick();
        }
    }

    void initializeInputs() override {
        top->clk = 0;
        top->reset = 1;
        top->enable = 0;
        top->thread_enable = 0;

        for (int i = 0; i < THREADS_PER_WARP; ++i) {
            top->alu_out[i] = 0;
            top->lsu_out[i] = 0;
        }

        top->warp_id = 0;
        top->block_id = 0;
        top->block_size = 0;
        top->warp_state = static_cast<uint8_t>(WarpState::IDLE);
        top->decoded_reg_write_enable = 0;
        top->decoded_reg_input_mux = static_cast<uint8_t>(RegInputMux::ALU_OUT);
        top->decoded_immediate = 0;
        top->decoded_rd_address = 0;
        top->decoded_rs1_address = 0;
        top->decoded_rs2_address = 0;

        tick();
        top->reset = 0;
        tick();
    }

    void setAllThreadEnable(bool val) {
        if (val) {
            // Use 1ULL (unsigned long long) to avoid overflow if THREADS_PER_WARP >= 32
            top->thread_enable = (1ULL << THREADS_PER_WARP) - 1;
        } else {
            top->thread_enable = 0;
        }
    }

public:
    uint32_t getExpectedThreadId(uint32_t warp_id_val, int thread_idx_in_warp) {
        return warp_id_val * THREADS_PER_WARP + thread_idx_in_warp;
    }
};

// Test Case 1: Verifies that special registers are correctly initialized on reset and can be read.
TEST_F(RegFileTestbench, ResetAndSpecialRegisterRead) {
    top->enable = 1;
    setAllThreadEnable(true);

    // Set identifiers that will be loaded into special registers
    uint32_t test_warp_id = 2;
    uint32_t test_block_id = 5;
    uint32_t test_block_size = 256;
    top->warp_id = test_warp_id;
    top->block_id = test_block_id;
    top->block_size = test_block_size;

    // The DUT writes to special registers every cycle. Tick once to ensure values are set.
    runSimulation(1);

    // Test ZERO_REG
    top->decoded_rs1_address = ZERO_REG;
    runSimulation(1);
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        EXPECT_EQ(top->rs1[i], 0) << "Thread " << i << ", ZERO_REG value mismatch.";
    }

    // Test THREAD_ID_REG
    top->decoded_rs1_address = THREAD_ID_REG;
    runSimulation(1);
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        EXPECT_EQ(top->rs1[i], getExpectedThreadId(test_warp_id, i))
            << "Thread " << i << ", THREAD_ID_REG value mismatch.";
    }

    // Test BLOCK_ID_REG
    top->decoded_rs1_address = BLOCK_ID_REG;
    runSimulation(1);
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        EXPECT_EQ(top->rs1[i], test_block_id) << "Thread " << i << ", BLOCK_ID_REG value mismatch.";
    }

    // Test BLOCK_SIZE_REG
    top->decoded_rs1_address = BLOCK_SIZE_REG;
    runSimulation(1);
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        EXPECT_EQ(top->rs1[i], test_block_size) << "Thread " << i << ", BLOCK_SIZE_REG value mismatch.";
    }
}

// Test Case 2: Verifies basic write and read functionality for a general-purpose register.
TEST_F(RegFileTestbench, WriteReadGeneralPurpose) {
    top->enable = 1;
    setAllThreadEnable(true);
    int test_reg_addr = 5;
    std::vector<uint32_t> test_values(THREADS_PER_WARP);

    // --- WRITE PHASE ---
    top->warp_state = static_cast<uint8_t>(WarpState::UPDATE);
    top->decoded_reg_write_enable = 1;
    top->decoded_rd_address = test_reg_addr;
    top->decoded_reg_input_mux = static_cast<uint8_t>(RegInputMux::ALU_OUT);

    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        test_values[i] = 0x12340000 + i;
        top->alu_out[i] = test_values[i];
    }
    runSimulation(1); // Clock tick to latch the write

    // --- READ PHASE ---
    // Disable writes. Reading is combinational and does not depend on warp_state.
    top->decoded_reg_write_enable = 0;
    top->decoded_rs1_address = test_reg_addr;
    top->decoded_rs2_address = test_reg_addr;
    
    runSimulation(1);

    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        EXPECT_EQ(top->rs1[i], test_values[i]) << "Thread " << i << " rs1 read from r" << test_reg_addr;
        EXPECT_EQ(top->rs2[i], test_values[i]) << "Thread " << i << " rs2 read from r" << test_reg_addr;
    }
}

// Test Case 3: Verifies that writes are correctly masked by the thread_enable signal.
TEST_F(RegFileTestbench, ThreadEnableMasking) {
    top->enable = 1;
    int test_reg_addr = 6;
    uint32_t write_value = 0xAABBCCDD;
    uint32_t initial_value = 0; // Registers are 0 after reset

    // --- MASKED WRITE PHASE ---
    top->warp_state = static_cast<uint8_t>(WarpState::UPDATE);
    top->decoded_reg_write_enable = 1;
    top->decoded_rd_address = test_reg_addr;
    top->decoded_reg_input_mux = static_cast<uint8_t>(RegInputMux::IMMEDIATE);
    top->decoded_immediate = write_value;

    // Create a mask to only enable even-numbered threads
    uint32_t thread_enable_mask_write = 0;
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        if (i % 2 == 0) {
            thread_enable_mask_write |= (1U << i);
        }
    }
    top->thread_enable = thread_enable_mask_write;
    runSimulation(1); // Perform the masked write

    // --- VERIFICATION PHASE ---
    setAllThreadEnable(true); // Enable all threads for reading
    top->decoded_reg_write_enable = 0;
    top->decoded_rs1_address = test_reg_addr;
    
    runSimulation(1);

    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        if (i % 2 == 0) {
            EXPECT_EQ(top->rs1[i], write_value) << "Thread " << i << " (write enabled) value mismatch.";
        } else {
            EXPECT_EQ(top->rs1[i], initial_value) << "Thread " << i << " (write disabled) value should be initial.";
        }
    }
    
    // NOTE: A "read masking" test is no longer valid. The new hardware design features
    // always-on, combinational read ports that are not affected by the thread_enable signal.
}

// Test Case 4: Confirms that writes to read-only special registers are correctly ignored.
TEST_F(RegFileTestbench, WriteToSpecialRegisters) {
    top->enable = 1;
    setAllThreadEnable(true);

    // Capture initial special register values
    uint32_t WID = 3; top->warp_id = WID;
    uint32_t BID = 7; top->block_id = BID;
    uint32_t BSZ = 64; top->block_size = BSZ;
    runSimulation(1); // Clock to update special regs

    std::vector<uint32_t> initial_thread_ids(THREADS_PER_WARP);
    for(int i = 0; i < THREADS_PER_WARP; ++i) {
        initial_thread_ids[i] = getExpectedThreadId(WID, i);
    }
    
    // --- ATTEMPT WRITE PHASE ---
    uint32_t attempt_write_value = 0xDEADBEEF;
    top->warp_state = static_cast<uint8_t>(WarpState::UPDATE);
    top->decoded_reg_write_enable = 1;
    top->decoded_reg_input_mux = static_cast<uint8_t>(RegInputMux::IMMEDIATE);
    top->decoded_immediate = attempt_write_value;

    // Attempt to write to each special register. The hardware should ignore these.
    top->decoded_rd_address = ZERO_REG;       runSimulation(1);
    top->decoded_rd_address = THREAD_ID_REG;  runSimulation(1);
    top->decoded_rd_address = BLOCK_ID_REG;   runSimulation(1);
    top->decoded_rd_address = BLOCK_SIZE_REG; runSimulation(1);

    top->decoded_reg_write_enable = 0; // Disable writes for safety

    // Verify all special registers retained their original values
    top->decoded_rs1_address = ZERO_REG;
    runSimulation(1);
    EXPECT_EQ(top->rs1[0], 0) << "ZERO_REG should not be writable.";

    top->decoded_rs1_address = THREAD_ID_REG;
    runSimulation(1);
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        EXPECT_EQ(top->rs1[i], initial_thread_ids[i]) << "Thread " << i << " THREAD_ID_REG should not be writable.";
    }

    top->decoded_rs1_address = BLOCK_ID_REG;
    runSimulation(1);
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        EXPECT_EQ(top->rs1[i], BID) << "Thread " << i << " BLOCK_ID_REG should not be writable.";
    }

    top->decoded_rs1_address = BLOCK_SIZE_REG;
    runSimulation(1);
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        EXPECT_EQ(top->rs1[i], BSZ) << "Thread " << i << " BLOCK_SIZE_REG should not be writable.";
    }
}

// main function - standard GTest and Verilator setup
int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");

    auto res = RUN_ALL_TESTS();

#if VM_COVERAGE
    VerilatedCov::write(("logs/coverage_" + std::string(MODULE_NAME) + ".dat").c_str());
#endif

    return res;
}