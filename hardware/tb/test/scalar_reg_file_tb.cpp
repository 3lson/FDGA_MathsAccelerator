#include "base_testbench.h" // Your provided base class
#include <verilated_cov.h> // For coverage
#include <vector>          // For convenience

// Define the name of the module being tested (for coverage reporting)
#define MODULE_NAME "scalar_reg_file"

// Constants for the design parameters (match Verilog compilation)
const int DATA_WIDTH = 32;       // Must match Verilog parameter (`common.svh` `DATA_WIDTH`)

// C++ equivalents for Verilog enums (values must match Verilog definitions in common.svh)
enum class WarpState : uint8_t {
    IDLE = 0,
    FETCH = 1,
    DECODE = 2,
    REQUEST = 3,
    WAIT = 4,
    EXECUTE = 5,
    ALU_WAIT = 6,
    INT_ALU_WAIT = 7,
    UPDATE = 8,
    SYNC_WAIT = 9,
    DONE = 10
};

enum class RegInputMux : uint8_t {
    ALU_OUT = 0,
    LSU_OUT = 1,
    IMMEDIATE = 2,
    PC_PLUS_1 = 3,
    VECTOR_TO_SCALAR = 4
};

const int ZERO_REG = 0;
const int EXECUTION_MASK_REG = 31;

class ScalarRegFileTestbench : public BaseTestbench {
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

        top->warp_state = static_cast<uint8_t>(WarpState::IDLE);
        top->decoded_reg_write_enable = 0;
        top->decoded_reg_input_mux = static_cast<uint8_t>(RegInputMux::ALU_OUT);
        top->decoded_immediate = 0;
        top->decoded_rd_address = 0;
        top->decoded_rs1_address = 0;
        top->decoded_rs2_address = 0;
        
        top->alu_out = 0;
        top->lsu_out = 0;
        top->pc = 0;
        top->vector_to_scalar_data = 0;

        top->eval();
        runSimulation(2);
        top->reset = 0;
        top->eval();
    }

public:
    // Helper methods for test cases
    void setWarpState(WarpState state) {
        top->warp_state = static_cast<uint8_t>(state);
    }
};

TEST_F(ScalarRegFileTestbench, ResetAndZeroRegister) {
    // After reset, all registers should be 0.
    top->enable = 1;

    // Test zero register
    top->decoded_rs1_address = ZERO_REG;
    tick();
    EXPECT_EQ(top->rs1, 0) << "ZERO_REG should always read 0";

    // Test a general-purpose register
    top->decoded_rs1_address = 4; // x4
    top->reset = 1;
    tick();
    EXPECT_EQ(top->rs1, 0) << "GP reg x4 should be 0 after reset";

    // Test another GP register, including the one that used to be the mask
    top->decoded_rs1_address = 31; // x31
    top->reset = 1;
    tick();
    EXPECT_EQ(top->rs1, 0) << "GP reg x31 should be 0 after reset";
}

TEST_F(ScalarRegFileTestbench, WriteReadGeneralPurpose) {
    top->enable = 1;
    
    // Test ALU_OUT write
    setWarpState(WarpState::UPDATE);
    top->decoded_reg_write_enable = 1;
    top->decoded_rd_address = 5; // R5
    top->decoded_reg_input_mux = static_cast<uint8_t>(RegInputMux::ALU_OUT);
    top->alu_out = 0x12345678;
    runSimulation(1);

    // Read back
    setWarpState(WarpState::REQUEST);
    top->decoded_reg_write_enable = 0;
    top->decoded_rs1_address = 5;
    top->decoded_rs2_address = 5;
    runSimulation(1);

    EXPECT_EQ(top->rs1, 0x12345678) << "rs1 read from R5 mismatch";
    EXPECT_EQ(top->rs2, 0x12345678) << "rs2 read from R5 mismatch";
}

TEST_F(ScalarRegFileTestbench, WriteSourceMuxSelection) {
    top->enable = 1;
    int test_reg_addr = 6;
    
    // Test LSU_OUT write
    setWarpState(WarpState::UPDATE);
    top->decoded_reg_write_enable = 1;
    top->decoded_rd_address = test_reg_addr;
    top->decoded_reg_input_mux = static_cast<uint8_t>(RegInputMux::LSU_OUT);
    top->lsu_out = 0xDEADBEEF;
    runSimulation(1);

    // Test IMMEDIATE write
    top->decoded_reg_input_mux = static_cast<uint8_t>(RegInputMux::IMMEDIATE);
    top->decoded_immediate = 0xCAFEBABE;
    runSimulation(1);

    // Test PC_PLUS_1 write
    top->decoded_reg_input_mux = static_cast<uint8_t>(RegInputMux::PC_PLUS_1);
    top->pc = 0x1000;
    runSimulation(1);

    // Test VECTOR_TO_SCALAR write
    top->decoded_reg_input_mux = static_cast<uint8_t>(RegInputMux::VECTOR_TO_SCALAR);
    top->vector_to_scalar_data = 0xABCD1234;
    runSimulation(1);

    // Read back
    setWarpState(WarpState::REQUEST);
    top->decoded_reg_write_enable = 0;
    top->decoded_rs1_address = test_reg_addr;
    runSimulation(1);

    EXPECT_EQ(top->rs1, 0xABCD1234) << "rs1 read from R6 mismatch (should be last write)";
}

TEST_F(ScalarRegFileTestbench, WarpDisable) {
    int test_reg_addr = 7;
    uint32_t initial_write_val = 0x11112222;
    uint32_t attempt_overwrite_val = 0x33334444;

    top->enable = 1;
    setWarpState(WarpState::UPDATE);
    top->decoded_reg_write_enable = 1;
    top->decoded_rd_address = test_reg_addr;
    top->decoded_reg_input_mux = static_cast<uint8_t>(RegInputMux::IMMEDIATE);
    top->decoded_immediate = initial_write_val;
    runSimulation(1);

    top->enable = 0;
    top->decoded_immediate = attempt_overwrite_val;
    runSimulation(1);

    top->enable = 1;
    setWarpState(WarpState::REQUEST);
    top->decoded_reg_write_enable = 0;
    top->decoded_rs1_address = test_reg_addr;
    runSimulation(1);

    EXPECT_EQ(top->rs1, initial_write_val)
        << "R7 value should not change when warp is disabled";
}

TEST_F(ScalarRegFileTestbench, ExecutionMaskRegister) {
    top->enable = 1;
    
    // Check initial execution mask (should be 1 after reset)
    EXPECT_EQ(top->warp_execution_mask, 0xFFFFFFFF) << "Initial execution mask mismatch";

    // Write to execution mask register
    setWarpState(WarpState::UPDATE);
    top->decoded_reg_write_enable = 1;
    top->decoded_rd_address = EXECUTION_MASK_REG;
    top->decoded_reg_input_mux = static_cast<uint8_t>(RegInputMux::IMMEDIATE);
    top->decoded_immediate = 0xFFFFFFFF;
    runSimulation(1);

    // Check updated execution mask
    EXPECT_EQ(top->warp_execution_mask, 0xFFFFFFFF) << "Updated execution mask mismatch";

    // Try to write to zero register (should be ignored)
    top->decoded_rd_address = ZERO_REG;
    top->decoded_immediate = 0x12345678;
    runSimulation(1);

    // Verify zero register is still zero
    setWarpState(WarpState::REQUEST);
    top->decoded_reg_write_enable = 0;
    top->decoded_rs1_address = ZERO_REG;
    runSimulation(1);
    EXPECT_EQ(top->rs1, 0) << "ZERO_REG should still be zero";
}

TEST_F(ScalarRegFileTestbench, StateDependentOperations) {
    top->enable = 1;
    int test_reg_addr = 8;
    
    // Write should only happen in UPDATE state
    top->decoded_reg_write_enable = 1;
    top->decoded_rd_address = test_reg_addr;
    top->decoded_reg_input_mux = static_cast<uint8_t>(RegInputMux::IMMEDIATE);
    top->decoded_immediate = 0x55555555;
    
    // Try write in REQUEST state (should not work)
    setWarpState(WarpState::REQUEST);
    runSimulation(1);

    // Read in REQUEST state
    top->decoded_rs1_address = test_reg_addr;
    runSimulation(1);
    EXPECT_EQ(top->rs1, 0) << "Register should not be written in REQUEST state";

    // Now do write in UPDATE state
    setWarpState(WarpState::UPDATE);
    runSimulation(1);

    // Verify write worked
    setWarpState(WarpState::REQUEST);
    top->decoded_reg_write_enable = 0;
    runSimulation(1);
    EXPECT_EQ(top->rs1, 0x55555555) << "Register should be written in UPDATE state";
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