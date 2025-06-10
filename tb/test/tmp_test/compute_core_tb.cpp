#include "base_testbench.h"
#include <verilated_cov.h>
#include <gtest/gtest.h>
#include <cstdint>
#include <map>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

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
#define OPCODE_C    0b111

class ComputeCoreTestbench : public BaseTestbench {
protected:
    std::map<uint32_t, uint32_t> instr_mem;
    std::map<uint32_t, uint32_t> data_mem;

    void loadProgramFromHex(const std::string& hex_filepath) {
        instr_mem.clear(); // Clear any previous program
        std::ifstream hex_file(hex_filepath);
        ASSERT_TRUE(hex_file.is_open()) << "Could not open hex file: " << hex_filepath;

        std::string line;
        uint32_t current_address = 0;
        while (std::getline(hex_file, line)) {
            // Skip empty lines or comments
            if (line.empty() || line[0] == '/' || line[0] == '#') {
                continue;
            }
            
            // Convert hex string to uint32_t
            uint32_t instruction;
            std::stringstream ss;
            ss << std::hex << line;
            ss >> instruction;

            std::cout << "  PC[" << current_address << "]: 0x" << std::hex << instruction << std::dec << std::endl; // ADD THIS LINE

            // Store in our simulated instruction memory
            instr_mem[current_address] = instruction;

            // Increment address for the next instruction (assuming word-addressable)
            // Your ISA is byte-addressable but instructions are 4 bytes long.
            current_address++; // This assumes your assembler outputs instructions for PC 0, 1, 2...
        }
        
        std::cout << "Loaded " << instr_mem.size() << " instructions from " << hex_filepath << std::endl;
    }

    void tick() {
        top->clk = 0;
        top->eval();
        top->clk = 1;
        top->eval();
    }

    void runSimulation(int cycles) {
        for (int i = 0; i < cycles; ++i) {
            // --- Before the clock edge ---
            // Combinational Memory Read Logic (DUT requests, TB provides)
            for (int w = 0; w < WARPS_PER_CORE; ++w) {
                if (top->instruction_mem_read_valid & (1ULL << w)) {
                    // Verilator flattens arrays: module.port[idx] -> module.port_idx
                    uint32_t addr = top->instruction_mem_read_address[w];
                    top->instruction_mem_read_data[w] = instr_mem.count(addr) ? instr_mem[addr] : 0;
                }
            }
            for (int t = 0; t < NUM_LSUS; ++t) {
                if (top->data_mem_read_valid & (1ULL << t)) {
                    uint32_t addr = top->data_mem_read_address[t];
                    top->data_mem_read_data[t] = data_mem.count(addr) ? data_mem[addr] : 0xDEADBEEF;
                }
            }

            // --- Clock Tick ---
            top->clk = 0;
            top->eval();
            top->clk = 1;
            top->eval();

            // --- After the clock edge ---
            for (int t = 0; t < NUM_LSUS; ++t) {
                if (top->data_mem_write_valid & (1ULL << t)) {
                    data_mem[top->data_mem_write_address[t]] = top->data_mem_write_data[t];
                }
            }
        }
    }

    void initializeInputs() override {
        top->clk = 1; top->reset = 1; top->start = 0; top->block_id = 0;
        top->kernel_config[3] = 0; // base instruction address
        top->kernel_config[2] = 0x1000; // base data memory address
        top->kernel_config[0] = 1;  // num_blocks
        top->kernel_config[1] = 1; // num_warps_per_block

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

// TEST_F(ComputeCoreTestbench, ResetBehavior) {
//     runSimulation(1);
//     EXPECT_EQ(top->done, 0);
//     EXPECT_EQ(top->instruction_mem_read_valid, 0);
//     EXPECT_EQ(top->data_mem_read_valid, 0);
//     EXPECT_EQ(top->data_mem_write_valid, 0);
// }

// TEST_F(ComputeCoreTestbench, SimplestExit) {
//     // Program:
//     // 0: exit
//     std::map<uint32_t, uint32_t> program;

//     // Construct the 'exit' instruction manually.
//     // According to the ISA: opcode=111, funct3=111. Other bits are don't-care.
//     uint32_t exit_instr = (OPCODE_C << 29) | (0b111 << 10);
//     program[0] = exit_instr;
    
//     // Configured in Initialise inputs
    
//     loadAndRun(program);

//     // The `loadAndRun` function will fail if it times out.
//     // If we reach this point, it means `top->done` was asserted.
//     SUCCEED() << "Core successfully fetched, decoded, and executed an EXIT instruction.";
// }

// TEST_F(ComputeCoreTestbench, ScalarALUAndStore_FromHex) {
//     // 1. Load the program from the hex file
//     loadProgramFromHex("test/tmp_test/program.hex"); // Assumes the file is in the build/run directory

//     // 2. Configure the core for the test
//     // Core configured in Initialise Input helper function
    
//     // 3. Run the simulation
//     loadAndRun(instr_mem); // Pass the populated map to the existing runner

//     // 4. Check the result
//     EXPECT_EQ(data_mem[42], 32) << "Scalar ALU/Store data path failed when loaded from hex.";
// }

TEST_F(ComputeCoreTestbench, VectorALUAndStore_FromHex) {
    // 1. Load the program from the hex file
    loadProgramFromHex("test/tmp_test/program.hex"); // Assumes the file is in the build/run directory

//     // 2. Configure the core for the test
//     // Core configured in Initialise Input helper function
    
//     // 3. Run the simulation
//     loadAndRun(instr_mem); // Pass the populated map to the existing runner

    // 4. Check the result
    EXPECT_EQ(data_mem[42], 32) << "Vector ALU/Store data path failed when loaded from hex.";
}


TEST_F(ComputeCoreTestbench, IScalarTest) {
    // Assembly Intent:
    // s.li sp, 42
    // s.li s1, 10
    // s.sw s1, 0(sp)     // mem[42] = 10
    // s.lw s2, 0(sp)     // s2 = 10
    // s.addi s3, s2, 10  // s3 = 20
    // s.muli s3, s2, 10  // s3 = 100
    // s.divi s3, s2, 10  // s3 = 1
    // s.slli s3, s2, 2   // s3 = 40
    // exit
    // We assume the program is written to store the final result of s3 (40)
    // back into memory at address 42 for verification.

    loadProgramFromHex("test/tmp_test/iscalar.hex");
    loadAndRun(instr_mem);

    // Check that the final result (40) was stored at the stack address.
    EXPECT_EQ(data_mem[44], 20) << "Scalar ADDI failed";
    EXPECT_EQ(data_mem[45], 100) << "Scalar MULI failed";
    EXPECT_EQ(data_mem[46], 1) << "Scalar DIVI failed";
    EXPECT_EQ(data_mem[47], 40) << "Scalar SLLI failed"; // failing
}

// TEST_F(ComputeCoreTestbench, JumpTest) {
//     // Assembly Intent:
//     // s.li s1, 4         // i = 4
//     // s.slt s4, s2, s3   // s2=i=4, s3=3 -> s4 = (4 < 3) = 0
//     // j for_end1         // Unconditionally jump
//     // for_end1:
//     // s.li s5, 5
//     // exit
//     // The program should execute the jump and then load 5 into s5.
//     // We assume the program stores the final value of s5 (5) at address 42.

//     loadProgramFromHex("test/tmp_test/jump.hex");
//     loadAndRun(instr_mem);

//     // The program should store 5 at the end.
//     EXPECT_EQ(data_mem[42], 5) << "Jump test failed to store the correct final value.";
// }


// TEST_F(ComputeCoreTestbench, CScalarTest) {
//     // Assembly Intent:
//     // s.li s1, 4         // i = 4
//     // ...
//     // s.slt s4, s2, s3   // s2=i=4, s3=3 -> s4 = (4 < 3) = 0
//     // s.beqz s4, for_end1// Branch if s4 is zero. It is, so branch is taken.
//     // for_end1:
//     // s.li s5, 5
//     // exit
//     // The program should take the branch and load 5 into s5.
//     // We assume the program stores the final value of s5 (5) at address 42.

//     loadProgramFromHex("test/tmp_test/cscalar.hex");
//     loadAndRun(instr_mem);

//     // The program should branch and store 5 at the end.
//     EXPECT_EQ(data_mem[43], 0) << "CScalar conditional branch test failed.";
// }

TEST_F(ComputeCoreTestbench, FScalarTest) {
    // Assembly Intent: Test various floating point operations.
    // The program needs to load float constants (1.0f) from a data section.
    // The testbench sets the base data memory address to 0x1000. We will assume
    // the linker places the constants there.
    // .LC0: .word 1065353216 (1.0f) at address 0x1000
    // .LC1: .word 1065353216 (1.0f) at address 0x1004

    loadProgramFromHex("test/tmp_test/fscalar.hex");
    loadAndRun(instr_mem);

    // Verify results based on the sequence of operations in the assembly snippet.
    // Note: Addresses are calculated as `base_addr + offset`.
    // fs1 and fs2 are both loaded with 1.0f.
    
    EXPECT_EQ(data_mem[44], 2.0) << "Scalar Floating ADD failed";
    EXPECT_EQ(data_mem[45], 0.0) << "Scalar Floating SUB failed";
    EXPECT_EQ(data_mem[46], 1.0) << "Scalar Floating MUL failed";
    EXPECT_EQ(data_mem[47], 1.0) << "Scalar Floating DIV failed";
    EXPECT_EQ(data_mem[48], 0) << "Scalar Floating FLT failed";
    EXPECT_EQ(data_mem[49], -1.0) << "Scalar Floating FNEG failed";
    EXPECT_EQ(data_mem[50], 1) << "Scalar Floating FEQ failed";
    EXPECT_EQ(data_mem[51], 1.0) << "Scalar Floating FABS failed";
    EXPECT_EQ(data_mem[52], 1) << "Scalar Floating FCVT.W.S failed";
}

// // NOTE: mscalar.hex seems incomplete. Assuming it's meant to test memory ops.
// // Here is a hypothetical test based on the provided snippet.
// TEST_F(ComputeCoreTestbench, MScalarTest) {
//     // Assembly Intent: Load a float, store it back to a different location.
//     // lui s1, %hi(.LC0)
//     // s.flw fs1, %lo(.LC0)(s1)  # Load 1.0f into fs1
//     // s.fsw fs1, -108(s0)       # Store fs1 somewhere

//     // The store address is `-108(s0)`. Let's assume s0 is set to a base address
//     // like 200 to make the final address positive and verifiable (200 - 108 = 92).
//     loadProgramFromHex("test/tmp_test/mscalar.hex");
//     loadAndRun(instr_mem);

//     // Verify that the value 1.0f was loaded from 0x1000 and stored at address 92.
//     EXPECT_EQ(data_mem[42], 10) << "Scalar Store Failed";
//     EXPECT_EQ(data_mem[43], 1.0) << "Scalar Load Failed";
// }



int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    auto result = RUN_ALL_TESTS();
    VerilatedCov::write(("logs/coverage_" + std::string(NAME) + ".dat").c_str());
    return result;
}