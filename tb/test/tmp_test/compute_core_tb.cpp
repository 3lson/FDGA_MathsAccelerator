#include "base_testbench.h"
#include <verilated_cov.h>
#include <gtest/gtest.h>
#include <cstdint>
#include <map>
#include <functional>
#include <iostream>

#define NAME "compute_core"
// NOTE: These parameters MUST match what you compile Verilog with.
#define WARPS_PER_CORE 4
#define THREADS_PER_WARP 16
#define NUM_LSUS (THREADS_PER_WARP + 1)

// Opcode definitions
#define OPCODE_R    0b000
#define OPCODE_I    0b001
#define OPCODE_F    0b010
#define OPCODE_UP   0b011
#define OPCODE_M    0b100
#define OPCODE_HALT 0b110
#define OPCODE_J    0b111

class ComputeCoreTestbench : public BaseTestbench {
protected:
    std::map<uint32_t, uint32_t> instr_mem;
    std::map<uint32_t, uint32_t> data_mem;

    // Helper to create instruction with various fields
    uint32_t makeInstr(uint8_t opcode, uint8_t funct, 
                       uint8_t rd, uint8_t rs1, uint8_t rs2,
                       uint32_t immediate, bool scalar = false) {
        uint32_t instr = 0;
        instr |= (uint32_t)(opcode & 0x7) << 29;

        switch (opcode) {
            case OPCODE_R:
            case OPCODE_F:
                instr |= (uint32_t)scalar << 28;
                instr |= (uint32_t)(rs2 & 0x1F) << 14;
                instr |= (uint32_t)(funct & 0xF) << 10;
                instr |= (uint32_t)(rs1 & 0x1F) << 5;
                instr |= (uint32_t)(rd & 0x1F);
                break;
            case OPCODE_I:
                instr |= (uint32_t)scalar << 28;
                instr |= (immediate & 0x3FFF) << 14;
                instr |= (uint32_t)(funct & 0xF) << 10;
                instr |= (uint32_t)(rs1 & 0x1F) << 5;
                instr |= (uint32_t)(rd & 0x1F);
                break;
            case OPCODE_M:
                instr |= (uint32_t)scalar << 13;
                instr |= (uint32_t)(funct & 0x7) << 10; // funct is funct3
                instr |= (uint32_t)(rs1 & 0x1F) << 5;
                if (funct == 0b000 || funct == 0b010) { // LW, FLW
                    instr |= (uint32_t)(rd & 0x1F);
                    instr |= (immediate & 0x7FFF) << 14;
                } else { // SW, FSW
                    instr |= (uint32_t)(rs2 & 0x1F) << 14;
                    instr |= ((immediate >> 5) & 0x3FF) << 19;
                    instr |= (immediate & 0x1F);
                }
                break;
            case OPCODE_UP:
                instr |= (uint32_t)scalar << 5;
                instr |= (immediate & 0xFFFFF) << 9;
                instr |= (uint32_t)(rd & 0x1F);
                break;
            case OPCODE_J:
                 instr |= (uint32_t)(funct & 0x7) << 10; // funct is funct3
                 // ... Add logic for JAL, BEQZ if needed
                 break;
            case OPCODE_HALT:
                 // No other fields needed
                 break;
        }
        return instr;
    }

    void tick() {
        top->clk = 0;
        top->eval();
        top->clk = 1;
        top->eval();
    }

    void runSimulation(int cycles) {
        for (int i = 0; i < cycles; ++i) {
            // --- FIX: Use Verilator's flattened array access ---
            // Instruction Memory Read
            for (int w = 0; w < WARPS_PER_CORE; ++w) {
                if (top->instruction_mem_read_valid & (1ULL << w)) {
                    uint32_t addr = top->instruction_mem_read_address[w];
                    top->instruction_mem_read_data[w] = instr_mem.count(addr) ? instr_mem[addr] : 0;
                }
            }
            // Data Memory Read
            for (int t = 0; t < NUM_LSUS; ++t) {
                if (top->data_mem_read_valid & (1ULL << t)) {
                    uint32_t addr = top->data_mem_read_address[t];
                    top->data_mem_read_data[t] = data_mem.count(addr) ? data_mem[addr] : 0xDEADBEEF;
                }
            }

            tick();

            // Data Memory Write
            for (int t = 0; t < NUM_LSUS; ++t) {
                if (top->data_mem_write_valid & (1ULL << t)) {
                    data_mem[top->data_mem_write_address[t]] = top->data_mem_write_data[t];
                }
            }
        }
    }

    void initializeInputs() override {
        top->clk = 1; top->reset = 1; top->start = 0; top->block_id = 0;
        top->kernel_config[0] = 0; top->kernel_config[1] = 0x1000;
        top->kernel_config[2] = 1; top->kernel_config[3] = 1;

        top->instruction_mem_read_ready = -1; // All ready
        top->data_mem_read_ready = -1;
        top->data_mem_write_ready = -1;
        
        runSimulation(2);
        top->reset = 0;
    }

    void loadAndRun(const std::map<uint32_t, uint32_t>& program) {
        instr_mem = program;
        data_mem.clear();
        top->start = 1;
        tick();
        top->start = 0;
        
        int max_cycles = 500;
        for (int i = 0; i < max_cycles; ++i) {
            if (top->done) {
                std::cout << "Core finished in " << i << " cycles." << std::endl;
                return;
            }
            runSimulation(1);
        }
        FAIL() << "Core did not finish within the " << max_cycles << " cycle timeout.";
    }

    bool waitForCondition(std::function<bool()> condition, int maxCycles = 100) {
        for (int i = 0; i < maxCycles; ++i) {
            if (condition()) return true;
            runSimulation(1);
        }
        return false;
    }
    
    static uint32_t float_to_bits(float f) { return *reinterpret_cast<uint32_t*>(&f); }
    static float bits_to_float(uint32_t bits) { return *reinterpret_cast<float*>(&bits); }
};

// --- CORRECTED TESTS ---
TEST_F(ComputeCoreTestbench, ResetBehavior) {
    runSimulation(1);
    EXPECT_EQ(top->done, 0);
    EXPECT_EQ(top->instruction_mem_read_valid, 0);
    EXPECT_EQ(top->data_mem_read_valid, 0);
    EXPECT_EQ(top->data_mem_write_valid, 0);
}

TEST_F(ComputeCoreTestbench, StartSequenceAndFetch) {
    top->start = 1;
    tick();
    top->start = 0;

    bool fetchStarted = waitForCondition([this]() {
        return (top->instruction_mem_read_valid & 1) != 0;
    });

    ASSERT_TRUE(fetchStarted) << "Core did not start fetch";
    EXPECT_EQ(top->instruction_mem_read_address[0], 0);
}

TEST_F(ComputeCoreTestbench, ScalarIntegerALU) {
    std::map<uint32_t, uint32_t> program = {
        {0, makeInstr(OPCODE_I, 0b0000, 9, 0, 0, 10, true)},  // addi s1 (x9), x0, 10
        {1, makeInstr(OPCODE_I, 0b0000, 10, 0, 0, 20, true)}, // addi s2 (x10), x0, 20
        {2, makeInstr(OPCODE_R, 0b0000, 11, 9, 10, 0, true)}, // add s3 (x11), s1, s2
        {3, makeInstr(OPCODE_M, 0b001, 0, 0, 11, 0, true)},   // sw s3, 0(x0)
        {4, makeInstr(OPCODE_HALT, 0, 0, 0, 0, 0, true)}
    };
    
    loadAndRun(program);

    EXPECT_EQ(data_mem[0x1000], 30) << "Scalar integer ADD result is incorrect.";
}

TEST_F(ComputeCoreTestbench, VectorFloatALU) {
    data_mem[0x1000] = float_to_bits(1.5f);
    data_mem[0x1004] = float_to_bits(2.5f);
    
    std::map<uint32_t, uint32_t> program = {
        {0, makeInstr(OPCODE_M, 0b010, 1, 0, 0, 0, false)},       // flw fv1, 0(x0)
        {1, makeInstr(OPCODE_M, 0b010, 2, 0, 0, 4, false)},       // flw fv2, 4(x0)
        {2, makeInstr(OPCODE_F, 0b0000, 3, 1, 2, 0, false)}, // fadd.s fv3, fv1, fv2
        {3, makeInstr(OPCODE_M, 0b011, 0, 29, 3, 8, false)}, // fsw fv3, 8(x29)
        {4, makeInstr(OPCODE_HALT, 0, 0, 0, 0, 0, true)}
    };

    loadAndRun(program);
    
    for (int i = 0; i < THREADS_PER_WARP; ++i) {
        uint32_t addr = 0x1000 + 8 + (i * 4);
        EXPECT_FLOAT_EQ(bits_to_float(data_mem[addr]), 4.0f) 
            << "Vector float FADD result is incorrect for thread " << i;
    }
}

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    auto result = RUN_ALL_TESTS();
    VerilatedCov::write(("logs/coverage_" + std::string(NAME) + ".dat").c_str());
    return result;
}