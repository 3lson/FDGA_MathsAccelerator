#include "sync_testbench.h"
#include "vbuddy.cpp"
#include <iostream>
#include <cstdlib>

#define NAME            "top-pdf"

class CpuTestbench : public SyncTestbench
{
protected:
    void initializeInputs() override
    {
        top->clk = 1;
        top->rst = 0;

        // We compile the program here, so the whole thing can use it.
        system("./compile.sh --input asm/pdf.s --output ../rtl/program.hex");
    }
};


TEST_F(CpuTestbench, InitialStateTest)
{
    // Initialise VBuddy
    //-------------------------------------------------------------------------
    if (vbdOpen() != 1)
    {
        SUCCEED();
    }
    vbdHeader("PDF plotting");
    //-------------------------------------------------------------------------
    
    int plot = 0;

    for (int i = 0; i < 1'000'000; ++i)
    {
        runSimulation(1);

        if (plot == false && top->a0 != 0)
        {
            plot = 1;
        }
        if (plot && (int)top->a0 >= 0)
        {
            vbdPlot(top->a0, 0, 255);
            plot++;
        }
        if (plot > 256)
        {
            break;
        }
    }

    SUCCEED();
}

int main(int argc, char **argv)
{
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    auto res = RUN_ALL_TESTS();
    
    return res;
}
