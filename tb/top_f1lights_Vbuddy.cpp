//Completed

#include "sync_testbench.h"

#define NAME            "top-f1lights"
#include "vbuddy.cpp"


class CpuTestbench : public SyncTestbench
{
protected:
    void initializeInputs() override
    {
        top->clk = 1;
        top->rst = 0;

        system("./compile.sh --input asm/f1_lights.s --output ../rtl/program.hex");

    }
};



TEST_F(CpuTestbench, RunvBuddy)
{
    int max_cycles = 10000;

    // Initialise VBuddy
    //-------------------------------------------------------------------------
    if (vbdOpen() != 1)
    {
        SUCCEED();
    }
    vbdHeader("F1-Lights");
    //-------------------------------------------------------------------------

    for (int i = 0; i < max_cycles; ++i)
    {
        // Mask to get 8 bits
        vbdBar(top->a0 & 0xFF);
        runSimulation();
        sleep(1);
    }

    SUCCEED();
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