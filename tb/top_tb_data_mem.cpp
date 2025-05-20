#include "testbench.h"
#include <cstdlib>
#define CYCLES 10000
unsigned int ticks = 0;

class CpuTestbench : public Testbench
{
protected:
    void initializeInputs() override
    {
        top->clk = 1;
        top->rst = 0;
    }
};

TEST_F(CpuTestbench, InitialStateTest)
{
    top->eval();
    EXPECT_EQ(top->clk, 1);
    EXPECT_EQ(top->rst, 0);
    EXPECT_EQ(top->a0, 0);
}

TEST_F(CpuTestbench, LoadStoreInstructionsTest) 
{
    system("./compile.sh asm/little_endian.s");  // Compile the test assembly code
    bool test_complete = false;

    for (int i = 0; i < CYCLES; i++) 
    {
        runSimulation(1);

        // Test initial register loading (indirect via a0) 
        EXPECT_EQ(top->a0, 0x12345678) << "Failed to load initial value for x10 into memory";

        
        // Test store word results (indirect via a0)
        // Assume a0 is loaded with the value read from memory after store operations
        runSimulation(1);
        EXPECT_EQ(top->a0, 0x12345678) << "Failed to store x10 to memory correctly";
            
        // Read second memory location (indirect verification)
        runSimulation(1);
        EXPECT_EQ(top->a0, 0xFF886543) << "Failed to store x11 to memory correctly";

        
        // Test load instructions (indirect via a0)
        EXPECT_EQ(top->a0, 0x00000078) << "lb test failed";
        runSimulation(1);
        EXPECT_EQ(top->a0, 0xFFFFFF88) << "lh test failed - sign extension issue";
        runSimulation(1);
        EXPECT_EQ(top->a0, 0x12345678) << "lw test failed";
        runSimulation(1);
        EXPECT_EQ(top->a0, 0x00000088) << "lbu test failed - unsigned load issue";
        runSimulation(1);
        EXPECT_EQ(top->a0, 0x0000FF88) << "lhu test failed - unsigned half load issue";
        
        // Test store byte/halfword and subsequent loads
        EXPECT_EQ(top->a0, 0x00000078) << "sb/lbu test failed at offset 8";
        runSimulation(1);
        EXPECT_EQ(top->a0, 0x00005678) << "sh/lhu test failed at offset 12";
        test_complete = true;
        
    }
    
    EXPECT_TRUE(test_complete) << "Test did not complete within expected cycle count";
}
