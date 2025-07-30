#include "base_testbench.h" // Your provided base class
#include <verilated_cov.h> // For coverage
#include <vector>          // For convenience

// Define the name of the module being tested (for coverage reporting)
#define MODULE_NAME "reg_file"

// Constants for the design parameters (match Verilog compilation)
const int THREADS_PER_WARP = 16; // Must match Verilog parameter
const int DATA_WIDTH = 32;       // Must match Verilog parameter (`common.svh` `DATA_WIDTH`)

// C++ equivalents for Verilog enums (values must match Verilog definitions in common.svh)
enum class WarpState : uint8_t {
    IDLE = 0,
    FETCH = 1,
    DECODE = 2,
    REQUEST = 3,
    WAIT = 4,
    EXECUTE = 5,
    UPDATE = 6,
    DONE = 7
};

enum class RegInputMux : uint8_t {
    ALU_OUT = 0,
    LSU_OUT = 1,
    IMMEDIATE = 2,
    VECTOR_TO_SCALAR = 3
};

const int ZERO_REG = 0;
const int THREAD_ID_REG = 29;
const int BLOCK_ID_REG = 30;
const int BLOCK_SIZE_REG = 31;

class RegFileTestbench : public BaseTestbench {
protected:
    vluint64_t main_time = 0;

    void tick() {
        top->clk = 0;
        top->eval();
#ifndef __APPLE__
        if (tfp) tfp->dump(main_time);
#endif
        main_time++;

        top->clk = 1;
        top->eval();
#ifndef __APPLE__
        if (tfp) tfp->dump(main_time);
#endif
        main_time++;
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

        // --- MODIFIED ---
        top->thread_enable = 0; // Clear all bits

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

        top->eval();
        runSimulation(2);
        top->reset = 0;
        top->eval();
    }

    void setAllThreadEnable(bool val) {
        // --- MODIFIED ---
        if (val) {
            if (THREADS_PER_WARP == 32) { // Special case for full 32-bit mask
                 top->thread_enable = 0xFFFFFFFFU;
            } else {
                 top->thread_enable = (1U << THREADS_PER_WARP) - 1; // Set all bits up to THREADS_PER_WARP
            }
        } else {
            top->thread_enable = 0; // Clear all bits
        }
    }

public:
    uint32_t getExpectedThreadId(uint32_t warp_id_val, int thread_idx_in_warp) {
        return warp_id_val * THREADS_PER_WARP + thread_idx_in_warp;
    }
};

TEST_F(RegFileTestbench, ResetAndSpecialRegisterRead) {
    top->enable = 1;
    setAllThreadEnable(true);
    top->warp_state = static_cast<uint8_t>(WarpState::REQUEST);

    uint32_t test_warp_id = 2;
    uint32_t test_block_id = 5;
    uint32_t test_block_size = 256;
    top->warp_id = test_warp_id;
    top->block_id = test_block_id;
    top->block_size = test_block_size;

    runSimulation(1);

    top->decoded_rs1_address = ZERO_REG;
    runSimulation(1);
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        EXPECT_EQ(top->rs1[i], 0) << "Thread " << i << ", ZERO_REG value mismatch.";
    }

    top->decoded_rs1_address = THREAD_ID_REG;
    runSimulation(1);
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        EXPECT_EQ(top->rs1[i], getExpectedThreadId(test_warp_id, i))
            << "Thread " << i << ", THREAD_ID_REG value mismatch.";
    }

    top->decoded_rs1_address = BLOCK_ID_REG;
    runSimulation(1);
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        EXPECT_EQ(top->rs1[i], test_block_id) << "Thread " << i << ", BLOCK_ID_REG value mismatch.";
    }

    top->decoded_rs1_address = BLOCK_SIZE_REG;
    runSimulation(1);
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        EXPECT_EQ(top->rs1[i], test_block_size) << "Thread " << i << ", BLOCK_SIZE_REG value mismatch.";
    }

    top->decoded_rs1_address = 4;
    runSimulation(1);
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        EXPECT_EQ(top->rs1[i], 0) << "Thread " << i << ", R4 (GP reg) after reset mismatch.";
    }
}

TEST_F(RegFileTestbench, WriteReadGeneralPurpose) {
    top->enable = 1;
    setAllThreadEnable(true);
    int test_reg_addr = 5;
    std::vector<uint32_t> test_values(THREADS_PER_WARP);

    top->warp_state = static_cast<uint8_t>(WarpState::UPDATE);
    top->decoded_reg_write_enable = 1;
    top->decoded_rd_address = test_reg_addr;
    top->decoded_reg_input_mux = static_cast<uint8_t>(RegInputMux::ALU_OUT);

    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        test_values[i] = 0x12340000 + i;
        top->alu_out[i] = test_values[i];
    }
    runSimulation(1);

    top->warp_state = static_cast<uint8_t>(WarpState::REQUEST);
    top->decoded_reg_write_enable = 0;
    top->decoded_rs1_address = test_reg_addr;
    top->decoded_rs2_address = test_reg_addr;
    runSimulation(1);

    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        EXPECT_EQ(top->rs1[i], test_values[i]) << "Thread " << i << " rs1 read from r" << test_reg_addr;
        EXPECT_EQ(top->rs2[i], test_values[i]) << "Thread " << i << " rs2 read from r" << test_reg_addr;
    }
}

TEST_F(RegFileTestbench, ThreadEnableMasking) {
    top->enable = 1;
    int test_reg_addr = 6;
    uint32_t write_value = 0xAABBCCDD;
    uint32_t initial_value = 0;

    top->warp_state = static_cast<uint8_t>(WarpState::UPDATE);
    top->decoded_reg_write_enable = 1;
    top->decoded_rd_address = test_reg_addr;
    top->decoded_reg_input_mux = static_cast<uint8_t>(RegInputMux::IMMEDIATE);
    top->decoded_immediate = write_value;

    // --- MODIFIED ---
    uint32_t thread_enable_mask_write = 0;
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        if (i % 2 == 0) { // Enable even threads
            thread_enable_mask_write |= (1U << i);
        }
    }
    top->thread_enable = thread_enable_mask_write;
    runSimulation(1);

    setAllThreadEnable(true);
    top->warp_state = static_cast<uint8_t>(WarpState::REQUEST);
    top->decoded_reg_write_enable = 0;
    top->decoded_rs1_address = test_reg_addr;
    runSimulation(1);

    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        if (i % 2 == 0) {
            EXPECT_EQ(top->rs1[i], write_value) << "Thread " << i << " (enabled) value mismatch.";
        } else {
            EXPECT_EQ(top->rs1[i], initial_value) << "Thread " << i << " (disabled) value should be initial.";
        }
    }

    // --- Test Read Masking ---
    // --- MODIFIED ---
    uint32_t current_thread_enable = top->thread_enable; // Save current full mask
    current_thread_enable |= (1U << 0);   // Ensure thread 0 is enabled
    current_thread_enable &= ~(1U << 1);  // Ensure thread 1 is disabled
    top->thread_enable = current_thread_enable;

    top->decoded_rs1_address = test_reg_addr;
    runSimulation(1);
    uint32_t rs1_t0_val = top->rs1[0];
    EXPECT_EQ(rs1_t0_val, write_value) << "Thread 0 initial read of r6";

    // --- MODIFIED ---
    top->thread_enable &= ~(1U << 0); // Disable thread 0
    runSimulation(1);
    EXPECT_EQ(top->rs1[0], rs1_t0_val) << "Thread 0 rs1 output should hold when thread_enable[0]=0 during read";
}

TEST_F(RegFileTestbench, WriteToSpecialRegisters) {
    top->enable = 1;

    // --- MODIFIED ---
    top->thread_enable = (1U << 0); // Enable only thread 0

    uint32_t WID = 3; top->warp_id = WID;
    uint32_t BID = 7; top->block_id = BID;
    uint32_t BSZ = 64; top->block_size = BSZ;
    runSimulation(1);

    uint32_t attempt_write_value = 0xDEADBEEF;

    top->warp_state = static_cast<uint8_t>(WarpState::UPDATE);
    top->decoded_reg_write_enable = 1;
    top->decoded_reg_input_mux = 0b010;
    top->decoded_immediate = attempt_write_value;

    top->decoded_rd_address = ZERO_REG; runSimulation(1);
    top->decoded_rd_address = THREAD_ID_REG; runSimulation(1);
    top->decoded_rd_address = BLOCK_ID_REG; runSimulation(1);
    top->decoded_rd_address = BLOCK_SIZE_REG; runSimulation(1);

    setAllThreadEnable(true);
    top->warp_state = static_cast<uint8_t>(WarpState::REQUEST);
    top->decoded_reg_write_enable = 0;

    top->decoded_rs1_address = ZERO_REG;
    runSimulation(1);
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        EXPECT_EQ(top->rs1[i], 0) << "Thread " << i << " ZERO_REG value check.";
    }

    top->decoded_rs1_address = THREAD_ID_REG;
    runSimulation(1);
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        uint32_t expected_tid = getExpectedThreadId(WID, i);
        // if (i == 0) { // Thread 0 was enabled during the write to THREAD_ID_REG
        //     EXPECT_EQ(top->rs1[i], attempt_write_value) << "Thread " << i << " THREAD_ID_REG (written) check.";
        // } else {
            EXPECT_EQ(top->rs1[i], expected_tid) << "Thread " << i << " THREAD_ID_REG (original) check.";
        // }
    }

    top->decoded_rs1_address = BLOCK_ID_REG;
    runSimulation(1);
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        //  if (i == 0) {
        //     EXPECT_EQ(top->rs1[i], attempt_write_value) << "Thread " << i << " BLOCK_ID_REG (written) check.";
        // } else {
            EXPECT_EQ(top->rs1[i], BID) << "Thread " << i << " BLOCK_ID_REG (original) check.";
        // }
    }

    top->decoded_rs1_address = BLOCK_SIZE_REG;
    runSimulation(1);
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        // if (i == 0) {
        //     EXPECT_EQ(top->rs1[i], attempt_write_value) << "Thread " << i << " BLOCK_SIZE_REG (written) check.";
        // } else {
            EXPECT_EQ(top->rs1[i], BSZ) << "Thread " << i << " BLOCK_SIZE_REG (original) check.";
        // }
    }
}

TEST_F(RegFileTestbench, WarpDisable) {
    int test_reg_addr = 8;
    uint32_t initial_write_val = 0xCAFEBABE;
    uint32_t attempt_overwrite_val = 0xBADF00D;

    top->enable = 1;
    setAllThreadEnable(true);
    top->warp_state = static_cast<uint8_t>(WarpState::UPDATE);
    top->decoded_reg_write_enable = 1;
    top->decoded_rd_address = test_reg_addr;
    top->decoded_reg_input_mux = static_cast<uint8_t>(RegInputMux::IMMEDIATE);
    top->decoded_immediate = initial_write_val;
    runSimulation(1);

    top->enable = 0;
    top->decoded_immediate = attempt_overwrite_val;
    runSimulation(1);

    top->enable = 1;
    top->warp_state = static_cast<uint8_t>(WarpState::REQUEST);
    top->decoded_reg_write_enable = 0;
    top->decoded_rs1_address = test_reg_addr;
    runSimulation(1);

    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        EXPECT_EQ(top->rs1[i], initial_write_val)
            << "Thread " << i << ", r" << test_reg_addr << " value should not change when warp is disabled.";
    }
}

TEST_F(RegFileTestbench, WriteSourceLSU) {
    top->enable = 1;
    setAllThreadEnable(true);

    int test_reg_addr = 7;
    std::vector<uint32_t> test_lsu_values(THREADS_PER_WARP);

    top->warp_state = static_cast<uint8_t>(WarpState::UPDATE);
    top->decoded_reg_write_enable = 1;
    top->decoded_rd_address = test_reg_addr;
    top->decoded_reg_input_mux = static_cast<uint8_t>(RegInputMux::LSU_OUT);

    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        test_lsu_values[i] = 0xDEAD0000 + i;
        top->lsu_out[i] = test_lsu_values[i];
    }
    runSimulation(1);

    top->warp_state = static_cast<uint8_t>(WarpState::REQUEST);
    top->decoded_reg_write_enable = 0;
    top->decoded_rs1_address = test_reg_addr;
    runSimulation(1);

    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        EXPECT_EQ(top->rs1[i], test_lsu_values[i]) << "Thread " << i << " read from LSU write to r" << test_reg_addr;
    }
}

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
