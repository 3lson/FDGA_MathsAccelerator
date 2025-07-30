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

#define NAME "gpu"
// NOTE: These parameters MUST match what you compile Verilog with.
#define DATA_MEM_NUM_CHANNELS 8
#define INSTRUCTION_MEM_NUM_CHANNELS 8
#define NUM_CORES 1
#define WARPS_PER_CORE 4
#define THREADS_PER_WARP 16

class GPUTestbench : public BaseTestbench {
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

            std::cout << "  PC[" << current_address << "]: 0x" << std::hex << instruction << std::dec << std::endl;

            // Store in our simulated instruction memory
            instr_mem[current_address] = instruction;

            // Increment address for the next instruction
            current_address++;
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
            
            // Increment address by 4 for the next word (byte-addressable memory)
            current_offset += 4;
        }
        std::cout << "Loaded data from " << hex_filepath << " (total data words: " << data_mem.size() << ")" << std::endl;
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
            // Handle instruction memory requests
            for (int ch = 0; ch < INSTRUCTION_MEM_NUM_CHANNELS; ++ch) {
                if (top->instruction_mem_read_valid & (1ULL << ch)) {
                    uint32_t addr = top->instruction_mem_read_address[ch];
                    top->instruction_mem_read_data[ch] = instr_mem.count(addr) ? instr_mem[addr] : 0;
                }
                // Set ready signal for instruction memory
                top->instruction_mem_read_ready |= (1ULL << ch);
            }

            // Handle data memory read requests
            for (int ch = 0; ch < DATA_MEM_NUM_CHANNELS; ++ch) {
                if (top->data_mem_read_valid & (1ULL << ch)) {
                    uint32_t addr = top->data_mem_read_address[ch];
                    top->data_mem_read_data[ch] = data_mem.count(addr) ? data_mem[addr] : 0xDEADBEEF;
                }
                // Set ready signals for data memory
                top->data_mem_read_ready |= (1ULL << ch);
                top->data_mem_write_ready |= (1ULL << ch);
            }

            // --- Clock Tick ---
            top->clk = 0;
            top->eval();
            top->clk = 1;
            top->eval();

            // --- After the clock edge ---
            // Handle data memory write requests
            for (int ch = 0; ch < DATA_MEM_NUM_CHANNELS; ++ch) {
                if (top->data_mem_write_valid & (1ULL << ch)) {
                    uint32_t addr = top->data_mem_write_address[ch];
                    uint32_t data = top->data_mem_write_data[ch];
                    data_mem[addr] = data;
                    std::cout << "  WRITE[0x" << std::hex << addr << "]: 0x" << data << std::dec << std::endl;
                }
            }
        }
    }

    void initializeInputs() override {
        // Initialize clock and reset
        top->clk = 1;
        top->reset = 1;
        top->execution_start = 0;
        
        // Initialize kernel configuration
        top->base_instr = 0;     // Base instruction address
        top->base_data = 0;      // Base data address  
        top->num_blocks = 1;     // Number of blocks to execute
        top->warps_per_block = WARPS_PER_CORE; // Number of warps per block
        
        // Initialize memory interface ready signals
        top->instruction_mem_read_ready = (1ULL << INSTRUCTION_MEM_NUM_CHANNELS) - 1; // All channels ready
        top->data_mem_read_ready = (1ULL << DATA_MEM_NUM_CHANNELS) - 1;  // All channels ready
        top->data_mem_write_ready = (1ULL << DATA_MEM_NUM_CHANNELS) - 1; // All channels ready
        
        // Run a few cycles with reset active
        runSimulation(2);
        
        // Release reset
        top->reset = 0;
        runSimulation(1);
    }

    void loadAndRun(const std::map<uint32_t, uint32_t>& program, uint32_t num_blocks = 1, uint32_t warps_per_block = WARPS_PER_CORE) {
        instr_mem = program;
        
        // Configure kernel parameters
        top->num_blocks = num_blocks;
        top->warps_per_block = warps_per_block;
        
        // Start execution
        top->execution_start = 1;
        tick();
        top->execution_start = 0;
        
        int max_cycles = 2000; // Increased timeout for GPU execution
        for (int i = 0; i < max_cycles; ++i) {
            if (top->execution_done) {
                std::cout << "GPU finished execution in " << i << " cycles." << std::endl;
                return;
            }
            runSimulation(1);
            
            // Print progress every 100 cycles
            if (i % 100 == 0 && i > 0) {
                std::cout << "  Cycle " << i << ": GPU still executing..." << std::endl;
            }
        }
        FAIL() << "GPU did not finish within the " << max_cycles << " cycle timeout.";
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

    void printMemoryRange(uint32_t start_addr, uint32_t end_addr) {
        std::cout << "Memory contents from 0x" << std::hex << start_addr 
                  << " to 0x" << end_addr << ":" << std::dec << std::endl;
        for (uint32_t addr = start_addr; addr <= end_addr; ++addr) {
            if (data_mem.count(addr)) {
                std::cout << "  [0x" << std::hex << addr << "]: 0x" << data_mem[addr] 
                          << " (" << std::dec << data_mem[addr] << ")" << std::endl;
            }
        }
    }
};

// Basic functionality test
TEST_F(GPUTestbench, ResetBehavior) {
    runSimulation(5);
    EXPECT_EQ(top->execution_done, 0);
    EXPECT_EQ(top->instruction_mem_read_valid, 0);
    EXPECT_EQ(top->data_mem_read_valid, 0);
    EXPECT_EQ(top->data_mem_write_valid, 0);
}

// Test simple program execution
TEST_F(GPUTestbench, SimpleKernelExecution) {
    // Clear any existing data
    data_mem.clear();
    
    // Load a simple test program (you'll need to create this hex file)
    loadProgramFromHex("test/tmp_test/simple_kernel.hex");
    
    // Run the kernel
    loadAndRun(instr_mem, 1, WARPS_PER_CORE);
    
    // Verify expected results - adjust these addresses and values based on your kernel
    EXPECT_TRUE(data_mem.count(100)) << "Kernel failed to write to expected memory location";
    std::cout << "Kernel execution completed successfully" << std::endl;
    
    // Print memory contents for debugging
    printMemoryRange(100, 120);
}

// Test vector addition kernel
TEST_F(GPUTestbench, VectorAdditionKernel) {
    data_mem.clear();
    
    // Setup input data for vector addition
    // Array A at addresses 0x1000-0x103C (16 elements)
    // Array B at addresses 0x1040-0x107C (16 elements)  
    // Result C at addresses 0x1080-0x10BC (16 elements)
    
    // Initialize input arrays
    for (int i = 0; i < 16; ++i) {
        data_mem[0x1000 + i*4] = i + 1;      // A[i] = i+1
        data_mem[0x1040 + i*4] = (i + 1) * 2; // B[i] = (i+1)*2
    }
    
    // Load vector addition kernel
    loadProgramFromHex("test/tmp_test/vector_add.hex");
    
    // Set base data address to point to our arrays
    top->base_data = 0x1000;
    
    // Run the kernel
    loadAndRun(instr_mem, 1, WARPS_PER_CORE);
    
    // Verify results: C[i] should equal A[i] + B[i] = (i+1) + (i+1)*2 = (i+1)*3
    for (int i = 0; i < 16; ++i) {
        uint32_t expected = (i + 1) * 3;
        uint32_t result_addr = 0x1080 + i*4;
        ASSERT_TRUE(data_mem.count(result_addr)) 
            << "Result array element " << i << " not written to memory";
        EXPECT_EQ(data_mem[result_addr], expected) 
            << "Vector addition failed at element " << i;
    }
    
    std::cout << "Vector addition kernel completed successfully" << std::endl;
    printMemoryRange(0x1080, 0x10BC);
}

// Test multi-block execution
TEST_F(GPUTestbench, MultiBlockExecution) {
    data_mem.clear();
    
    // Setup data for multiple blocks to process
    // Each block will process a different section of the array
    const uint32_t blocks = 2;
    const uint32_t elements_per_block = 8;
    
    // Initialize input data
    for (uint32_t i = 0; i < blocks * elements_per_block; ++i) {
        data_mem[0x2000 + i*4] = i * 10; // Input array
    }
    
    // Load multi-block kernel
    loadProgramFromHex("test/tmp_test/multi_block.hex");
    
    top->base_data = 0x2000;
    
    // Run with multiple blocks
    loadAndRun(instr_mem, blocks, WARPS_PER_CORE);
    
    // Verify each block processed its section correctly
    // (Verification logic depends on what your kernel does)
    for (uint32_t i = 0; i < blocks * elements_per_block; ++i) {
        uint32_t result_addr = 0x2100 + i*4; // Assuming results stored here
        if (data_mem.count(result_addr)) {
            std::cout << "Block processing result[" << i << "] = " << data_mem[result_addr] << std::endl;
        }
    }
    
    std::cout << "Multi-block execution completed" << std::endl;
}

// Test floating point operations
TEST_F(GPUTestbench, FloatingPointKernel) {
    data_mem.clear();
    
    // Setup floating point input data
    data_mem[0x3000] = float_to_bits(1.5f);
    data_mem[0x3004] = float_to_bits(2.5f);
    data_mem[0x3008] = float_to_bits(3.0f);
    data_mem[0x300C] = float_to_bits(4.0f);
    
    loadProgramFromHex("test/tmp_test/float_kernel.hex");
    loadDataFromHex("test/tmp_test/data_float.hex", 0x3000);
    
    top->base_data = 0x3000;
    
    loadAndRun(instr_mem, 1, WARPS_PER_CORE);
    
    // Verify floating point results
    if (data_mem.count(0x3100)) {
        float result = bits_to_float(data_mem[0x3100]);
        std::cout << "Floating point result: " << result << std::endl;
        // Add specific expectations based on your kernel
    }
    
    printMemoryRange(0x3100, 0x3120);
}

// Test memory access patterns
TEST_F(GPUTestbench, MemoryAccessPattern) {
    data_mem.clear();
    
    // Initialize a larger data set
    for (int i = 0; i < 64; ++i) {
        data_mem[0x4000 + i*4] = i;
    }
    
    loadProgramFromHex("test/tmp_test/memory_pattern.hex");
    
    top->base_data = 0x4000;
    
    loadAndRun(instr_mem, 1, WARPS_PER_CORE);
    
    // Verify memory access pattern results
    // Check that the kernel performed the expected memory operations
    bool found_writes = false;
    for (const auto& pair : data_mem) {
        if (pair.first >= 0x5000 && pair.first < 0x5100) {
            found_writes = true;
            std::cout << "Memory pattern result at 0x" << std::hex << pair.first 
                      << ": " << std::dec << pair.second << std::endl;
        }
    }
    
    EXPECT_TRUE(found_writes) << "No memory writes detected in expected range";
}

// Performance and stress test
TEST_F(GPUTestbench, StressTest) {
    data_mem.clear();
    
    // Large dataset
    const uint32_t data_size = 256;
    for (uint32_t i = 0; i < data_size; ++i) {
        data_mem[0x10000 + i*4] = i % 100;
    }
    
    loadProgramFromHex("test/tmp_test/stress_kernel.hex");
    
    top->base_data = 0x10000;
    
    // Run with maximum configuration
    loadAndRun(instr_mem, 4, WARPS_PER_CORE); // Multiple blocks
    
    // Basic verification that execution completed
    SUCCEED() << "Stress test completed without timeout";
    
    // Count how many memory locations were written
    uint32_t write_count = 0;
    for (const auto& pair : data_mem) {
        if (pair.first >= 0x20000) { // Assuming results written to this range
            write_count++;
        }
    }
    
    std::cout << "Stress test wrote to " << write_count << " memory locations" << std::endl;
}

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    auto result = RUN_ALL_TESTS();
    VerilatedCov::write(("logs/coverage_" + std::string(NAME) + ".dat").c_str());
    return result;
}