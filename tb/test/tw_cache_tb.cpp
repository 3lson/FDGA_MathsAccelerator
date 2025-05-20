#include "sync_testbench.h"
#include <verilated_cov.h>
#include <iostream>

#define NAME "tw_cache"

class TwoWayCacheTestbench : public SyncTestbench {
protected:
    void initializeInputs() override {
        top->clk = 1;
        top->reset = 0;
        top->AddrMode = 0;
        top->A = 0;
        top->WD = 0;
    }
};

// Test for cache hit
TEST_F(TwoWayCacheTestbench, CacheHitTest) {
    top->reset = 1;
    runSimulation(1);
    top->reset = 0;
    runSimulation(1);

    int address = 0x20;
    int writeData = 0xABCD1234;

    // Write
    top->AddrMode = 7; // Store word
    top->A = address;
    top->WD = writeData;
    runSimulation(2);

    // Read
    top->AddrMode = 2; // Load word
    top->A = address;
    runSimulation(2);

    EXPECT_EQ(top->hit, 1);
    EXPECT_EQ(top->out, writeData);
}

TEST_F(TwoWayCacheTestbench, CacheMissAndLRUTest) {
    top->reset = 1;
    runSimulation(1);
    top->reset = 0;
    runSimulation(1);

    int addr1 = 0x10, addr2 = 0x20, addr3 = 0x30;
    int data1 = 0xDEADBEEF, data2 = 0xBAADF00D, data3 = 0xFEEDFACE;

    // Write to way 0
    top->AddrMode = 7;
    top->A = addr1;
    top->WD = data1;
    runSimulation(2);

    // Write to way 1
    top->AddrMode = 7;
    top->A = addr2;
    top->WD = data2;
    runSimulation(2);

    // Write triggers LRU replacement
    top->AddrMode = 7;
    top->A = addr3;
    top->WD = data3;
    runSimulation(2);

    // Read addr1 (miss, since it was evicted)
    top->AddrMode = 2;
    top->A = addr1;
    runSimulation(2);
    EXPECT_EQ(top->hit, 0) << "Expected miss for addr1";

    // Read addr2 (hit, still in way 1)
    top->AddrMode = 2;
    top->A = addr2;
    runSimulation(2);
    EXPECT_EQ(top->hit, 1) << "Expected hit for addr2";
    EXPECT_EQ(top->out, data2) << "Expected data2";

    // Read addr3 (hit, replaced way 0)
    top->AddrMode = 2;
    top->A = addr3;
    runSimulation(2);
    EXPECT_EQ(top->hit, 1) << "Expected hit for addr3";
    EXPECT_EQ(top->out, data3) << "Expected data3";
}

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    auto res = RUN_ALL_TESTS();
    return res;
}
