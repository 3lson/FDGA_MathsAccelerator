#include "base_testbench.h"
#include <cstdlib>
#include <iomanip> 

#define NAME "instr_mem"

class InstrMemTestbench : public BaseTestbench
{
protected:
    void initializeInputs() override
    {
        top->addr = 0;
    }
};

TEST_F(InstrMemTestbench, PrintAllInstructions)
{   
    // Compile the asm file into hex
    //system("./compile.sh --input asm/counter.s");
    
    bool found_nonzero = false;

    // Assuming max 256 instructions (adjust as needed)
    const int max_instructions = 10;

    for (int i = 0; i < max_instructions; ++i)
    {
        top->addr = i * 4; // 4 bytes per instruction
        top->eval();

        uint32_t instr = top->instr;

        std::cout << "Address 0x" << std::setw(4) << std::setfill('0') << std::hex << (i * 4)
                  << ": 0x" << std::setw(8) << std::setfill('0') << instr << std::endl;

        if (instr != 0)
            found_nonzero = true;
    }

    // At least one instruction must be non-zero to pass
    EXPECT_TRUE(found_nonzero);
}

int main(int argc, char **argv)
{
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");

    int result = RUN_ALL_TESTS();

    VerilatedCov::write(("logs/coverage_" + std::string(NAME) + ".dat").c_str());

    return result;
}
