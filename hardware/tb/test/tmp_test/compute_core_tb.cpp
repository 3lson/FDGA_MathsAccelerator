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
    vluint64_t sim_time = 0;
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

            current_address++; // This assumes your assembler outputs instructions for PC 0, 1, 2...
        }
        
        std::cout << "Loaded " << instr_mem.size() << " instructions from " << hex_filepath << std::endl;
    }

    void loadDataFromHex(const std::string& hex_filepath, uint32_t base_address = 0) {
        std::ifstream hex_file(hex_filepath);
        ASSERT_TRUE(hex_file.is_open()) << "Could not open data hex file: " << hex_filepath;

        std::string line;
        uint32_t current_offset = 0;
        while (std::getline(hex_file, line)) {
            if (line.empty() || line[0] == '/' || line[0] == '#') continue;

            uint32_t data_word;
            std::stringstream ss;
            ss << std::hex << line;
            ss >> data_word;

            uint32_t address = base_address + current_offset;
            data_mem[address] = data_word;
            std::cout << "  DATA[0x" << std::hex << address << "]: 0x" << data_word << std::dec << std::endl;
            
            current_offset += 4;
        }
        std::cout << "Loaded " << data_mem.size() << " data words from " << hex_filepath << std::endl;
    }

    void tick() {
        top->clk = 0;
        top->eval();
        top->clk = 1;
        top->eval();
        // if (tfp){
        //     tfp->dump(sim_time++);
        // }
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

            // --- VCD Dump ---
            // if (tfp){
            //     tfp->dump(sim_time++);
            // }

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
        top->kernel_config[2] = 0; // base data memory address
        top->kernel_config[0] = 2;  // num_warps_per_block
        top->kernel_config[1] = 1; // num_blocks

        top->instruction_mem_read_ready = -1;
        top->data_mem_read_ready = -1;
        top->data_mem_write_ready = -1;
        
        runSimulation(2);
        top->reset = 0;
    }

    void loadAndRun(const std::map<uint32_t, uint32_t>& program) {
        instr_mem = program;
        //data_mem.clear();
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

TEST_F(ComputeCoreTestbench, ResetBehavior) {
    runSimulation(1);
    EXPECT_EQ(top->done, 0);
    EXPECT_EQ(top->instruction_mem_read_valid, 0);
    EXPECT_EQ(top->data_mem_read_valid, 0);
    EXPECT_EQ(top->data_mem_write_valid, 0);
}

TEST_F(ComputeCoreTestbench, SimplestExit) {
    std::map<uint32_t, uint32_t> program;

    uint32_t exit_instr = (OPCODE_C << 29) | (0b111 << 10);
    program[0] = exit_instr;
    
    loadAndRun(program);

    SUCCEED() << "Core successfully fetched, decoded, and executed an EXIT instruction.";
}

TEST_F(ComputeCoreTestbench, RScalarTest) {
    loadProgramFromHex("../../assembler/tests/expected_output/rscalar.instr.hex");

    loadAndRun(instr_mem);

    EXPECT_EQ(data_mem[42], 10) << "Scalar ALU/Store data path failed when loaded from hex.";
    EXPECT_EQ(data_mem[43], 20) << "Scalar ALU/Store data path failed when loaded from hex.";
    EXPECT_EQ(data_mem[44], 0) << "Scalar ALU/Store data path failed when loaded from hex.";
    EXPECT_EQ(data_mem[45], 100) << "Scalar ALU/Store data path failed when loaded from hex.";
    EXPECT_EQ(data_mem[46], 1) << "Scalar ALU/Store data path failed when loaded from hex.";
    EXPECT_EQ(data_mem[47], 0) << "Scalar ALU/Store data path failed when loaded from hex.";
    EXPECT_EQ(data_mem[48], 1) << "Scalar ALU/Store data path failed when loaded from hex.";
    EXPECT_EQ(data_mem[49], 1) << "Scalar ALU/Store data path failed when loaded from hex.";
    EXPECT_EQ(data_mem[50], 5) << "Scalar ALU/Store data path failed when loaded from hex.";
}


TEST_F(ComputeCoreTestbench, IScalarTest) {

    loadProgramFromHex("../../assembler/tests/expected_output/iscalar.instr.hex");
    loadAndRun(instr_mem);

    EXPECT_EQ(data_mem[44], 20) << "Scalar ADDI failed";
    EXPECT_EQ(data_mem[45], 100) << "Scalar MULI failed";
    EXPECT_EQ(data_mem[46], 1) << "Scalar DIVI failed";
    EXPECT_EQ(data_mem[47], 40) << "Scalar SLLI failed"; // failing
}


TEST_F(ComputeCoreTestbench, JumpTest) {

    loadProgramFromHex("../../assembler/tests/expected_output/jump.instr.hex");
    loadAndRun(instr_mem);

    EXPECT_EQ(data_mem[46], 0) << "Jump test failed to store the correct final value.";
    EXPECT_EQ(data_mem[42], 5) << "Jump test failed to store the correct final value.";
}

TEST_F(ComputeCoreTestbench, CScalarTest) {
    loadProgramFromHex("../../assembler/tests/expected_output/cscalar.instr.hex");
    loadAndRun(instr_mem);

    EXPECT_EQ(data_mem[43], 0) << "CScalar conditional branch test failed.";
    EXPECT_EQ(data_mem[44], 5) << "CScalar conditional branch test failed.";
}

TEST_F(ComputeCoreTestbench, FScalarTest) {
    data_mem.clear();

    loadProgramFromHex("../../assembler/tests/expected_output/fscalar.instr.hex");
    loadDataFromHex("../../assembler/tests/expected_output/fscalar.data.hex");

    loadAndRun(instr_mem);

    EXPECT_FLOAT_EQ(bits_to_float(data_mem[45]), 3.0f) << "Scalar FADD failed";
    EXPECT_FLOAT_EQ(bits_to_float(data_mem[46]), 1.0f) << "Scalar FSUB failed";
    EXPECT_FLOAT_EQ(bits_to_float(data_mem[47]), 2.0f) << "Scalar FMUL failed";
    EXPECT_FLOAT_EQ(bits_to_float(data_mem[48]), 2.0f) << "Scalar FDIV failed";
    EXPECT_EQ(data_mem[49], 1) << "Scalar FLT failed (2.0 < 1.0 is false)";
    EXPECT_FLOAT_EQ(bits_to_float(data_mem[50]), -1.0f) << "Scalar FNEG failed";
    EXPECT_EQ(data_mem[51], 0) << "Scalar FEQ failed (2.0 == 1.0 is false)";
    EXPECT_FLOAT_EQ(bits_to_float(data_mem[52]), 1.0f) << "Scalar FABS failed";
    EXPECT_EQ(data_mem[53], 1) << "Scalar FCVT.W.S (float to int) failed";
}

TEST_F(ComputeCoreTestbench, MScalarTest) {
    data_mem.clear();

    loadProgramFromHex("../../assembler/tests/expected_output/mscalar.instr.hex");
    loadDataFromHex("../../assembler/tests/expected_output/mscalar.data.hex");

    loadAndRun(instr_mem);

    EXPECT_EQ(bits_to_float(data_mem[42]), 1.0f) << "Scalar Store Failed";
    EXPECT_EQ(data_mem[43], 32) << "Scalar Load Failed";
    EXPECT_EQ(data_mem[44], 32) << "Scalar Load Failed";
}

TEST_F(ComputeCoreTestbench, RVectorTest) {
    loadProgramFromHex("../../assembler/tests/expected_output/rvector.instr.hex");

    loadAndRun(instr_mem);

    EXPECT_EQ(data_mem[42], 10) << "Scalar ALU/Store data path failed when loaded from hex.";
    EXPECT_EQ(data_mem[43], 20) << "Scalar ALU/Store data path failed when loaded from hex.";
    EXPECT_EQ(data_mem[44], 0) << "Scalar ALU/Store data path failed when loaded from hex.";
    EXPECT_EQ(data_mem[45], 100) << "Scalar ALU/Store data path failed when loaded from hex.";
    EXPECT_EQ(data_mem[46], 1) << "Scalar ALU/Store data path failed when loaded from hex.";
    EXPECT_EQ(data_mem[47], 0) << "Scalar ALU/Store data path failed when loaded from hex.";
    EXPECT_EQ(data_mem[48], 1) << "Scalar ALU/Store data path failed when loaded from hex.";
    EXPECT_EQ(data_mem[49], 1) << "Scalar ALU/Store data path failed when loaded from hex.";
    EXPECT_EQ(data_mem[50], 5) << "Scalar ALU/Store data path failed when loaded from hex.";
}

TEST_F(ComputeCoreTestbench, IVectorTest) {
    loadProgramFromHex("../../assembler/tests/expected_output/ivector.instr.hex");
    loadAndRun(instr_mem);

    EXPECT_EQ(data_mem[44], 20) << "Scalar ADDI failed";
    EXPECT_EQ(data_mem[45], 100) << "Scalar MULI failed";
    EXPECT_EQ(data_mem[46], 1) << "Scalar DIVI failed";
    EXPECT_EQ(data_mem[47], 40) << "Scalar SLLI failed"; // failing
}


TEST_F(ComputeCoreTestbench, MVectorTest) {
    data_mem.clear();

    loadProgramFromHex("../../assembler/tests/expected_output/mvector.instr.hex");
    loadDataFromHex("../../assembler/tests/expected_output/mvector.data.hex");

    loadAndRun(instr_mem);

    EXPECT_EQ(bits_to_float(data_mem[42]), 1.0f) << "Scalar Store Failed";
    EXPECT_EQ(data_mem[43], 32) << "Scalar Load Failed";
    EXPECT_EQ(data_mem[44], 32) << "Scalar Load Failed";
}

TEST_F(ComputeCoreTestbench, FVectorTest) {
    data_mem.clear();

    loadProgramFromHex("../../assembler/tests/expected_output/fvector.instr.hex");
    loadDataFromHex("../../assembler/tests/expected_output/fvector.data.hex");

    loadAndRun(instr_mem);

    EXPECT_FLOAT_EQ(bits_to_float(data_mem[45]), 3.0f) << "Vector FADD failed";
    EXPECT_FLOAT_EQ(bits_to_float(data_mem[46]), 1.0f) << "Vector FSUB failed";
    EXPECT_FLOAT_EQ(bits_to_float(data_mem[47]), 2.0f) << "Vector FMUL failed";
    EXPECT_FLOAT_EQ(bits_to_float(data_mem[48]), 2.0f) << "Vector FDIV failed";
    EXPECT_EQ(data_mem[49], 1) << "Vector FLT failed (2.0 < 1.0 is false)";
    EXPECT_FLOAT_EQ(bits_to_float(data_mem[50]), -1.0f) << "Vector FNEG failed";
    EXPECT_EQ(data_mem[51], 0) << "Vector FEQ failed (2.0 == 1.0 is false)";
    EXPECT_FLOAT_EQ(bits_to_float(data_mem[52]), 1.0f) << "Vector FABS failed";
    EXPECT_EQ(data_mem[53], 1) << "Vector FCVT.W.S (float to int) failed";
}

// TEST_F(ComputeCoreTestbench, SXSLTTest) {
//     data_mem.clear();

//     loadProgramFromHex("test/tmp_test/sx_slt.hex");

//     loadDataFromHex("test/tmp_test/data_sx_slt.hex"); // This will load data starting at 0x1000

//     loadAndRun(instr_mem);

//     EXPECT_FLOAT_EQ(data_mem[42], 0xFFFF) << "SX.SLT failed";
// }

// TEST_F(ComputeCoreTestbench, SXSLTTest_0) {
//     data_mem.clear();

//     loadProgramFromHex("test/tmp_test/sx_slt_0.hex");

//     loadDataFromHex("test/tmp_test/data_sx_slt_0.hex"); // This will load data starting at 0x1000

//     loadAndRun(instr_mem);

//     EXPECT_FLOAT_EQ(data_mem[42], 0) << "SX.SLT.0 failed";
// }



int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    auto result = RUN_ALL_TESTS();
    VerilatedCov::write(("logs/coverage_" + std::string(NAME) + ".dat").c_str());
    return result;
}