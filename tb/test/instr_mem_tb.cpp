#include "base_testbench.h"
#include <cstdlib>
#include <iomanip> // Add this include for std::setw and std::setfill

#define NAME            "instr_mem"


class InstrMemTestbench : public BaseTestbench
{
protected:
    void initializeInputs() override
    {
        top->addr = 0;
    }
};


TEST_F(InstrMemTestbench, InstructionExistsTest)
{   
    //system("./compile.sh --input asm/counter.s");
    top->eval();
   
    std::cout << "top->instr (hex): " 
            << std::setw(8) << std::setfill('0')
            << std::hex << top->instr 
            << std::endl;
            
    EXPECT_NE(top->instr, 0);
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