#include "sync_testbench.h"
#include <verilated_cov.h>

#define NAME            "cache"

class CacheTestbench : public SyncTestbench
{
protected:
    void initializeInputs() override
    {
        top->clk = 1;
        top->reset = 0;
        top->AddrMode = 0;
        top->A = 0;
        top->WD = 0;
    }
};

// Test case to check cache hit functionality
TEST_F(CacheTestbench, CacheHitTest)
{
    // Reset the cache before the test
    top->reset = 1;
    runSimulation(1);
    top->reset = 0;
    runSimulation(1);

    int address = 0x00000020;
    int writeData = 0xDEADBEEF;

    // Write data
    top->AddrMode = 7; // SW (store word)
    top->A = address;
    top->WD = writeData;
    runSimulation(2);

    // Delay between write and read
    runSimulation(1);

    // Read data
    top->AddrMode = 2; // LW (load word)
    top->A = address;
    runSimulation(2);

    std::cout << "CacheHitTest - Hit: " << (int)top->hit << ", Out: 0x" << std::hex << top->out << std::dec << std::endl;

    EXPECT_EQ(top->hit, 1);
    EXPECT_EQ(top->out, writeData);
}

// Test case to check cache miss and fetching from main memory
TEST_F(CacheTestbench, CacheMissTest)
{
    top->reset = 1;
    runSimulation(1);
    top->reset = 0;
    runSimulation(1);

    const int NUM_ACCESSES = 5;
    int base_address = 0x30000000;
    
    for (int i = 0; i < NUM_ACCESSES; i++) {
        int address = base_address + (i * 0x1000);
        
        top->AddrMode = 2; // LW (load word)
        top->A = address;
        runSimulation(1); // Run for only 1 cycle

        std::cout << "CacheMissTest - Access " << i 
                  << " - Hit: " << (int)top->hit 
                  << ", Address: 0x" << std::hex << address << std::dec << std::endl;

        if (top->hit == 0) {
            EXPECT_EQ(top->hit, 0);
            return; // Exit the test as soon as a miss is detected
        }

        runSimulation(1); // Run for one more cycle to complete the access
    }

    FAIL() << "No cache misses observed after " << NUM_ACCESSES << " accesses";
}

// Test case for byte-specific writes and reads
TEST_F(CacheTestbench, ByteWriteReadTest)
{
    int address = 0x00000040;
    int byteData = 0xAB;

    top->AddrMode = 5; // SB (store byte)
    top->A = address;
    top->WD = byteData;
    runSimulation(2);

    top->AddrMode = 0; // LB (load byte)
    top->A = address;
    runSimulation(2);

    std::cout << "Hit: " << (int)top->hit << ", Out: 0x" << std::hex << (top->out & 0xFF) << std::dec << std::endl;

    EXPECT_EQ(top->hit, 1);
    EXPECT_EQ(top->out & 0xFF, byteData);
}

TEST_F(CacheTestbench, CacheHitAndMissTest)
{
    top->reset = 1;
    runSimulation(1);
    top->reset = 0;
    runSimulation(1);

    // First access - should be a miss
    int address1 = 0x30000000;
    top->AddrMode = 2; // LW (load word)
    top->A = address1;
    runSimulation(1);
    
    std::cout << "Access 1 - Hit: " << (int)top->hit 
              << ", Address: 0x" << std::hex << address1 << std::dec << std::endl;
    
    EXPECT_EQ(top->hit, 0); // Expect a miss

    // Second access to the same address - should be a hit
    runSimulation(1);
    top->A = address1;
    runSimulation(1);
    
    std::cout << "Access 2 - Hit: " << (int)top->hit 
              << ", Address: 0x" << std::hex << address1 << std::dec << std::endl;
    
    EXPECT_EQ(top->hit, 1); // Expect a hit

    // Third access to a different address - should be a miss
    int address2 = 0x30001000;
    top->A = address2;
    runSimulation(1);
    
    std::cout << "Access 3 - Hit: " << (int)top->hit 
              << ", Address: 0x" << std::hex << address2 << std::dec << std::endl;
    
    EXPECT_EQ(top->hit, 0); // Expect a miss
}


TEST_F(CacheTestbench, PerformanceCountersTest)
{

    int address1 = 0x00000050;
    int address2 = 0x00000060;
    int data1 = 0x12345678;

    EXPECT_EQ(top->total_accesses, 0);
    EXPECT_EQ(top->total_hits, 0);
    EXPECT_EQ(top->total_misses, 0);

    // First access (miss)
    top->AddrMode = 2; // LW (load word)
    top->A = address1;
    runSimulation(1); // Access should miss

    // Check after first access
    std::cout << "After first access - Accesses: " << top->total_accesses 
              << ", Hits: " << top->total_hits 
              << ", Misses: " << top->total_misses << std::endl;

    // Second access to the same address (hit)
    top->AddrMode = 2; // LW (load word)
    top->A = address1;
    runSimulation(1); // Access should hit

    // Check after second access
    std::cout << "After second access - Accesses: " << top->total_accesses 
              << ", Hits: " << top->total_hits 
              << ", Misses: " << top->total_misses << std::endl;

    // Third access to a different address (miss)
    top->AddrMode = 2; // LW (load word)
    top->A = address2;
    runSimulation(1); // Access should miss

    // Final check
    std::cout << "Final state - Accesses: " << top->total_accesses 
              << ", Hits: " << top->total_hits 
              << ", Misses: " << top->total_misses << std::endl;

    EXPECT_EQ(top->total_accesses, 3); // Expecting total accesses to be counted
    EXPECT_EQ(top->total_hits, 1);      // Expecting one hit
    EXPECT_EQ(top->total_misses, 2);    // Expecting two misses
}


int main(int argc, char **argv)
{
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    auto res = RUN_ALL_TESTS();
    // VerilatedCov::write(
    //     ("logs/coverage_" + std::string(NAME) + ".dat").c_str()
    // );

    return res;
}
