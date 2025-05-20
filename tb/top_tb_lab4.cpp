#include "testbench.h"
#include <cstdlib>

#define CYCLES 10000

unsigned int ticks = 0;

class CpuTestbench : public Testbench
{
protected:
    void initializeInputs() override
    {
        system("./compile.sh --input asm/data_mem_test.s --output ../rtl/program.hex");
        top->clk = 1;
        top->rst = 0;
    }
};

TEST_F(CpuTestbench, InitialStateTest)
{
    top->eval();
    
    EXPECT_EQ(top->clk, 1);
    EXPECT_EQ(top->rst, 0);
    EXPECT_EQ(top->Result, 0);
}

/*  Archived test for later
TEST_F(CpuTestbench, ResetStateTest)
{
    // Can be random number
    runSimulation(60);

    top->rst = 1;
    runSimulation(1);

    EXPECT_EQ(top->Result, 0);
}
*/


TEST_F(CpuTestbench, CounterGetsTo1)
{
    for (int i = 0; i < MAX_SIM_CYCLES; ++i)
    {
        runSimulation(1);
        
        if (top->Result == 1)
        {
            SUCCEED();
            return;
        }
        else if (top->Result > 1)
        {
            FAIL() << "Result exceeded 1 at sim cycle " << i;
            return;
        }
        else
        {
            /* Do nothing */
        }
    }
    
    FAIL() << "Result did not reach 1 within the maximum number of sim cycles";
}


TEST_F(CpuTestbench, CounterGetsTo254)
{
    for (int i = 0; i < MAX_SIM_CYCLES; ++i)
    {
        runSimulation(1);
        
        if (top->Result == 254)
        {
            SUCCEED();
            return;
        }
    }
    
    FAIL() << "Result did not reach 254 within the maximum number of sim cycles";
}


TEST_F(CpuTestbench, CounterResetsAfter254)
{
    int cycles_at_254;
    bool reached_254;

    for (int i = 0; i < MAX_SIM_CYCLES; ++i)
    {
        runSimulation(1);
        
        if (top->Result == 254)
        {
            cycles_at_254++;
            reached_254 = true;
        }
        else if (top->Result > 254)
        {
            runSimulation(10);
            FAIL() << "Result exceeded 254 at simulation cycle" << i;
            return;
        }
        else if (top->Result == 0)
        {
            SUCCEED();
            return;
        }
        else
        {
            /* Do nothing */
        }
    }
    
    FAIL() << "Result did not reset after 254";
}

TEST_F(CpuTestbench, BaseProgramTest)
{
    bool success = false;

    for (int i = 0; i < CYCLES; i++)
    {
        runSimulation(1);
        if (top->Result == 254)
        {
            SUCCEED();
            success = true;
            break;
        }
    }
    if (!success)
    {
        FAIL() << "Counter did not reach 254";
    }
}

// Note this is how we are going to test your CPU. Do not worry about this for
// now, as it requires a lot more instructions to function
// TEST_F(CpuTestbench, Return5Test)
// {
//     system("./compile.sh c/return_5.c");
//     runSimulation(100);
//     EXPECT_EQ(top->Result, 5);
// }

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    auto res = RUN_ALL_TESTS();
    return res;
}
