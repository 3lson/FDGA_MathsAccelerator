#include "sync_testbench.h"
#include <cstdlib>

#define NAME            "top-instr"


class CpuTestbench : public SyncTestbench
{
protected:
    void initializeInputs() override
    {
        top->clk = 1;
        top->rst = 0;
    }
};

TEST_F(CpuTestbench, cache_read_test)
{
    system("./compile.sh --input asm/010-cache_read.s");
    
    runSimulation(15);

    int hits = top->total_hits;
    int misses = top->total_misses;

    std::cout << "After write - Hits: " << hits << ", Misses: " << misses << std::endl;
    
    EXPECT_EQ((int)top->a0, 0);
}

TEST_F(CpuTestbench, cache_temporal_locality)
{
    system("./compile.sh --input asm/011-cache_temporal_locality.s");
    
    runSimulation(50);

    int hits = top->total_hits;
    int misses = top->total_misses;

    std::cout << "After write - Hits: " << hits << ", Misses: " << misses << std::endl;
    
    EXPECT_EQ((int)top->a0, 0);
}

TEST_F(CpuTestbench, cache_overwrite_byte)
{
    system("./compile.sh --input asm/012-overwrite_byte.s");
    
    runSimulation(100);

    int hits = top->total_hits;
    int misses = top->total_misses;

    std::cout << "After write - Hits: " << hits << ", Misses: " << misses << std::endl;
    
    EXPECT_EQ((int)top->a0, 0x0403FFFF);
}

TEST_F(CpuTestbench, cache_coherence)
{
    system("./compile.sh --input asm/013-coherence.s");
    
    runSimulation(50);

    int hits = top->total_hits;
    int misses = top->total_misses;

    std::cout << "After write - Hits: " << hits << ", Misses: " << misses << std::endl;
    
    EXPECT_EQ((int)top->a0, 0x2);
}


int main(int argc, char **argv)
{
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    auto res = RUN_ALL_TESTS();

    return res;
}