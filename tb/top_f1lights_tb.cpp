#include "sync_testbench.h"

#define NAME            "topf1lights"


class CpuTestbench : public SyncTestbench
{
protected:
    void initializeInputs() override
    {
        top->clk = 1;
        top->rst = 0;

        // We compile the program here, so the whole thing can use it.
        system("./compile.sh --input asm/f1_lights.s");
    }
};



TEST_F(CpuTestbench, LoopTest)
{
    int max_cycles = 1000;

    for (int i = 0; i < max_cycles; ++i)
    {
        runSimulation(); // Evaluate the model

        // Checking for subroutine execution (e.g., Result reaching 0xff)
        if (top->Result == 0xff) // Subroutine's final value
        {
            // Check if loop continues by resetting Result and observing it in subsequent cycles
            top->Result = 0;
        }
        else if (top->Result != 0)
        {
            SUCCEED(); // Indicates the loop has iterated at least once
            return;
        }

        top->clk = !top->clk; // Toggle the clock
    }

    FAIL() << "The iloop did not demonstrate expected behavior within " << max_cycles << " cycles.";
}


TEST_F(CpuTestbench, SubroutineFinalValueTest)
{
    int max_cycles = 1000; // Define a maximum number of cycles to simulate

    for (int i = 0; i < max_cycles; ++i)
    {
        runSimulation(); // Evaluate the model

        if (top->Result == 0xff) // Check if Result has the correct final value
        {
            SUCCEED();
            return;
        }

        top->clk = !top->clk; // Toggle the clock to simulate the next cycle
    }

    FAIL() << "The register Result did not reach the expected value within " << max_cycles << " cycles.";
}


int main(int argc, char **argv)
{
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    auto res = RUN_ALL_TESTS();
    VerilatedCov::write(
        ("logs/coverage_" + std::string(NAME) + ".dat").c_str()
    );

    return res;
}