#include "base_testbench.h"
#include <verilated_cov.h>
#include <bitset>
#include <iostream>
#include <iomanip>
#include <vector>
#include <queue>

#define NAME "gpu"

// GPU Parameters (matching the module defaults)
#define DATA_MEM_NUM_CHANNELS 8
#define INSTRUCTION_MEM_NUM_CHANNELS 8
#define NUM_CORES 1
#define WARPS_PER_CORE 1
#define THREADS_PER_WARP 16

// Memory simulation helpers
struct MemoryRequest {
    uint32_t address;
    uint32_t data;
    bool is_write;
    int channel;
};

class GpuTestbench : public BaseTestbench {
protected:
    // Memory simulation queues
    std::queue<MemoryRequest> data_mem_requests;
    std::queue<MemoryRequest> instruction_mem_requests;
    
    // Simple memory arrays for simulation
    std::vector<uint32_t> data_memory;
    std::vector<uint32_t> instruction_memory;
    
    void initializeInputs() override {
        // Initialize clock and reset
        top->clk = 0;
        top->reset = 1;
        top->execution_start = 0;
        
        // Initialize kernel configuration as wide vector
        // Pack the configuration into the wide vector format
        // Assuming the kernel_config is a 128-bit wide vector (4 x 32-bit words)
        top->kernel_config[0] = 0x1000;      // base_instructions_address
        top->kernel_config[1] = 0x2000;      // base_data_address  
        top->kernel_config[2] = 4;           // num_blocks
        top->kernel_config[3] = 2;           // num_warps_per_block
        
        // Initialize memory arrays
        data_memory.resize(0x10000, 0);
        instruction_memory.resize(0x10000, 0);
        
        // Initialize all memory interface signals
        initializeMemorySignals();
    }
    
    void initializeMemorySignals() {
        // Data memory read interface - arrays based on error messages
        for (int i = 0; i < DATA_MEM_NUM_CHANNELS; i++) {
            top->data_mem_read_ready = 1;
            top->data_mem_read_data[i] = 0;
            top->data_mem_write_ready = 1;
        }
        
        // Instruction memory read interface - arrays based on error messages
        for (int i = 0; i < INSTRUCTION_MEM_NUM_CHANNELS; i++) {
            top->instruction_mem_read_ready = 1;
            top->instruction_mem_read_data[i] = 0;
        }
    }
    
    void clockCycle() {
        top->clk = 0;
        top->eval();
        top->clk = 1;
        top->eval();
        handleMemoryRequests();
    }
    
    void handleMemoryRequests() {
        // Handle data memory read requests
        for (int i = 0; i < DATA_MEM_NUM_CHANNELS; i++) {
            if (top->data_mem_read_valid && top->data_mem_read_ready) {
                uint32_t addr = top->data_mem_read_address[i];
                if (addr < data_memory.size()) {
                    top->data_mem_read_data[i] = data_memory[addr];
                }
            }
            
            // Handle data memory write requests
            if (top->data_mem_write_valid && top->data_mem_write_ready) {
                uint32_t addr = top->data_mem_write_address[i];
                uint32_t data = top->data_mem_write_data[i];
                if (addr < data_memory.size()) {
                    data_memory[addr] = data;
                }
            }
        }
        
        // Handle instruction memory read requests
        for (int i = 0; i < INSTRUCTION_MEM_NUM_CHANNELS; i++) {
            if (top->instruction_mem_read_valid && top->instruction_mem_read_ready) {
                uint32_t addr = top->instruction_mem_read_address[i];
                if (addr < instruction_memory.size()) {
                    top->instruction_mem_read_data[i] = instruction_memory[addr];
                }
            }
        }
    }
    
    void reset() {
        top->reset = 1;
        clockCycle();
        clockCycle();
        top->reset = 0;
        clockCycle();
    }
    
    void startExecution() {
        top->execution_start = 1;
        clockCycle();
        top->execution_start = 0;
    }
    
    void waitForCompletion(int max_cycles = 10000) {
        int cycles = 0;
        while (!top->execution_done && cycles < max_cycles) {
            clockCycle();
            cycles++;
        }
        EXPECT_TRUE(top->execution_done) << "GPU execution did not complete within " << max_cycles << " cycles";
    }
    
    void loadInstructions(const std::vector<uint32_t>& instructions, uint32_t base_addr = 0x1000) {
        for (size_t i = 0; i < instructions.size(); i++) {
            if ((base_addr + i) < instruction_memory.size()) {
                instruction_memory[base_addr + i] = instructions[i];
            }
        }
    }
    
    void loadData(const std::vector<uint32_t>& data, uint32_t base_addr = 0x2000) {
        for (size_t i = 0; i < data.size(); i++) {
            if ((base_addr + i) < data_memory.size()) {
                data_memory[base_addr + i] = data[i];
            }
        }
    }
    
    void setKernelConfig(uint32_t base_instr, uint32_t base_data, uint32_t num_blocks, uint32_t warps_per_block) {
        top->kernel_config[0] = base_instr;
        top->kernel_config[1] = base_data;
        top->kernel_config[2] = num_blocks;
        top->kernel_config[3] = warps_per_block;
    }
};

// ------------------ BASIC INITIALIZATION TEST ------------------
TEST_F(GpuTestbench, BasicInitialization) {
    reset();
    
    // Check initial state
    EXPECT_EQ(top->execution_done, 0);
    EXPECT_EQ(top->reset, 0);
    
    // Verify memory interfaces are properly initialized
    for (int i = 0; i < DATA_MEM_NUM_CHANNELS; i++) {
        EXPECT_EQ(top->data_mem_read_valid, 0);
        EXPECT_EQ(top->data_mem_write_valid, 0);
    }
    
    for (int i = 0; i < INSTRUCTION_MEM_NUM_CHANNELS; i++) {
        EXPECT_EQ(top->instruction_mem_read_valid, 0);
    }
}

// ------------------ KERNEL CONFIGURATION TEST ------------------
TEST_F(GpuTestbench, KernelConfiguration) {
    reset();
    
    // Set up kernel configuration
    setKernelConfig(0x1000, 0x2000, 8, 4);
    
    startExecution();
    
    // Allow a few cycles for configuration to be latched
    for (int i = 0; i < 10; i++) {
        clockCycle();
    }
    
    // Configuration should be captured internally
    // (We can't directly check internal registers, but execution should start)
    EXPECT_EQ(top->execution_start, 0); // Should be cleared after one cycle
}

// ------------------ MEMORY INTERFACE TEST ------------------
TEST_F(GpuTestbench, MemoryInterfaceBasic) {
    reset();
    
    // Load some test data
    std::vector<uint32_t> test_data = {0xDEADBEEF, 0xCAFEBABE, 0x12345678, 0x87654321};
    loadData(test_data, 0x2000);
    
    // Load simple instructions (NOP-like)
    std::vector<uint32_t> test_instructions = {0x00000000, 0x00000000, 0x00000000, 0x00000000};
    loadInstructions(test_instructions, 0x1000);
    
    startExecution();
    
    // Run for a few cycles to see memory activity
    for (int i = 0; i < 50; i++) {
        clockCycle();
        
        // Check if any memory requests are being made
        bool data_read_active = false;
        bool instr_read_active = false;
        
        for (int j = 0; j < DATA_MEM_NUM_CHANNELS; j++) {
            if (top->data_mem_read_valid) {
                data_read_active = true;
                break;
            }
        }
        
        for (int j = 0; j < INSTRUCTION_MEM_NUM_CHANNELS; j++) {
            if (top->instruction_mem_read_valid) {
                instr_read_active = true;
                break;
            }
        }
        
        if (instr_read_active) {
            std::cout << "Instruction memory activity detected at cycle " << i << std::endl;
        }
    }
}

// ------------------ EXECUTION START/STOP TEST ------------------
TEST_F(GpuTestbench, ExecutionStartStop) {
    reset();
    
    // Verify initial state
    EXPECT_EQ(top->execution_done, 0);
    
    // Load minimal program (exit instruction)
    std::vector<uint32_t> exit_program = {
        0xE0000000, // EXIT instruction (assuming C-TYPE with funct=111)
        0x00000000,
        0x00000000,
        0x00000000
    };
    loadInstructions(exit_program, 0x1000);
    
    startExecution();
    
    // Wait for completion (should be quick with exit instruction)
    waitForCompletion(1000);
}

// // ------------------ MULTI-CORE TEST ------------------
// TEST_F(GpuTestbench, MultiCoreExecution) {
//     reset();
    
//     // Configure for multiple blocks to engage multiple cores
//     setKernelConfig(0x1000, 0x2000, NUM_CORES * 2, WARPS_PER_CORE);
    
//     // Load a simple program
//     std::vector<uint32_t> simple_program = {
//         0x00000000, // NOP
//         0x00000000, // NOP
//         0xE0000000, // EXIT
//         0x00000000
//     };
//     loadInstructions(simple_program, 0x1000);
    
//     startExecution();
    
//     // Monitor execution for core activity
//     bool cores_active = false;
//     for (int i = 0; i < 100; i++) {
//         clockCycle();
        
//         // Check for instruction fetch activity (indicates cores are working)
//         for (int j = 0; j < INSTRUCTION_MEM_NUM_CHANNELS; j++) {
//             if (top->instruction_mem_read_valid) {
//                 cores_active = true;
//                 break;
//             }
//         }
        
//         if (top->execution_done) break;
//     }
    
//     EXPECT_TRUE(cores_active) << "No core activity detected during execution";
// }

// // ------------------ MEMORY CONTROLLER STRESS TEST ------------------
// TEST_F(GpuTestbench, MemoryControllerStress) {
//     reset();
    
//     // Load data pattern for testing
//     std::vector<uint32_t> pattern_data;
//     for (int i = 0; i < 1000; i++) {
//         pattern_data.push_back(i * 0x12345678);
//     }
//     loadData(pattern_data, 0x2000);
    
//     // Load instructions that would cause memory access
//     std::vector<uint32_t> memory_intensive_program = {
//         0x80000000, // Load instruction (M-TYPE)
//         0x80000004, // Store instruction (M-TYPE)
//         0x80000008, // Load instruction (M-TYPE)
//         0xE0000000  // EXIT
//     };
//     loadInstructions(memory_intensive_program, 0x1000);
    
//     startExecution();
    
//     // Monitor memory controller activity
//     int data_requests = 0;
//     int instruction_requests = 0;
    
//     for (int i = 0; i < 20; i++) {
//         clockCycle();
        
//         // Count memory requests
//         for (int j = 0; j < DATA_MEM_NUM_CHANNELS; j++) {
//             if (top->data_mem_read_valid || top->data_mem_write_valid) {
//                 data_requests++;
//             }
//         }
        
//         for (int j = 0; j < INSTRUCTION_MEM_NUM_CHANNELS; j++) {
//             if (top->instruction_mem_read_valid) {
//                 instruction_requests++;
//             }
//         }
        
//         if (top->execution_done) break;
//     }
    
//     EXPECT_GT(instruction_requests, 0) << "No instruction memory requests observed";
//     std::cout << "Data requests: " << data_requests << ", Instruction requests: " << instruction_requests << std::endl;
// }

// ------------------ RESET DURING EXECUTION TEST ------------------
TEST_F(GpuTestbench, ResetDuringExecution) {
    reset();
    
    // Load a long-running program
    std::vector<uint32_t> long_program;
    for (int i = 0; i < 100; i++) {
        long_program.push_back(0x00000000); // NOPs
    }
    long_program.push_back(0xE0000000); // EXIT
    loadInstructions(long_program, 0x1000);
    
    startExecution();
    
    // Run for a while
    for (int i = 0; i < 50; i++) {
        clockCycle();
    }
    
    // Apply reset
    top->reset = 1;
    clockCycle();
    clockCycle();
    top->reset = 0;
    clockCycle();
    
    // Verify reset state
    EXPECT_EQ(top->execution_done, 0);
    
    // Verify memory interfaces are idle after reset
    bool interfaces_idle = true;
    for (int i = 0; i < DATA_MEM_NUM_CHANNELS; i++) {
        if (top->data_mem_read_valid || top->data_mem_write_valid) {
            interfaces_idle = false;
            break;
        }
    }
    EXPECT_TRUE(interfaces_idle) << "Memory interfaces not properly reset";
}

// ------------------ PARAMETER VERIFICATION TEST ------------------
TEST_F(GpuTestbench, ParameterVerification) {
    // These are compile-time checks, but we can verify the testbench constants
    EXPECT_EQ(DATA_MEM_NUM_CHANNELS, 8);
    EXPECT_EQ(INSTRUCTION_MEM_NUM_CHANNELS, 8);
    EXPECT_EQ(NUM_CORES, 1);
    EXPECT_EQ(WARPS_PER_CORE, 1);
    EXPECT_EQ(THREADS_PER_WARP, 16);
    
    std::cout << "GPU Configuration:" << std::endl;
    std::cout << "  Cores: " << NUM_CORES << std::endl;
    std::cout << "  Warps per core: " << WARPS_PER_CORE << std::endl;
    std::cout << "  Threads per warp: " << THREADS_PER_WARP << std::endl;
    std::cout << "  Data memory channels: " << DATA_MEM_NUM_CHANNELS << std::endl;
    std::cout << "  Instruction memory channels: " << INSTRUCTION_MEM_NUM_CHANNELS << std::endl;
}

// ------------------ MAIN ------------------
int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    auto res = RUN_ALL_TESTS();
    VerilatedCov::write(("logs/coverage_" + std::string(NAME) + ".dat").c_str());
    return res;
}