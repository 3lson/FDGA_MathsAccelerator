//Completed

#include "sync_testbench.h"

#define NAME            "top-pdf"



class CpuTestbench : public SyncTestbench
{
protected:
    void initializeInputs() override
    {
        top->clk = 1;
        top->rst = 0;

        system("./compile.sh --input asm/pdf.s --output ../rtl/program.hex");

    }
};



TEST_F(CpuTestbench, Runprogrammetest)
{
    int max_cycles = 10000;

    for (int i = 0; i < max_cycles; ++i)
    {
        runSimulation(1);
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