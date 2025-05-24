#include "sync_testbench.h"
#include <verilated_cov.h>

#define NAME "regfile"

class RegfileTestbench : public SyncTestbench {
protected:
    void initializeInputs() override {
        top->clk = 1;
        top->AD1 = 0;
        top->AD2 = 0;
        top->AD3 = 0;
        top->WE3 = 0;
        top->WD3 = 0;
        top->thread_read = 0;
        top->thread_write = 0;
        top->bIdx = 0;
    }
};

// testcase not passing 
TEST_F(RegfileTestbench, ThreadLocalRegisterWriteReadTest) {
    int value = 0xCAFEBABE;

    for (int tid = 0; tid < 4; ++tid) {
        for (int reg = 1; reg < 28; ++reg) {
            top->AD3 = reg;
            top->WD3 = value + tid + reg;
            top->WE3 = 1;
            top->thread_write = tid;

            runSimulation(1);

            top->AD1 = reg;
            top->AD2 = reg;
            top->thread_read = tid;

            runSimulation(1);
            printf("TID=%d REG=%d -> Writing %d\n", tid, reg, value + tid + reg);
            printf("RD1=%d RD2=%d\n", top->RD1, top->RD2);
            EXPECT_EQ(top->RD1, value + tid + reg);
            EXPECT_EQ(top->RD2, value + tid + reg);
        }
    }
}

TEST_F(RegfileTestbench, SpecialRegistersReadTest) {
    top->bIdx = 0x12345678;
    top->thread_read = 0;

    runSimulation(1);

    top->AD1 = 28; // tIdx
    top->AD2 = 29; // bIdx

    runSimulation(1);
    EXPECT_EQ(top->RD1, 0);             // tIdx
    EXPECT_EQ(top->RD2, 0x12345678);    // bIdx

    top->AD1 = 30; // bDim
    top->AD2 = 31; // lId

    runSimulation(1);
    EXPECT_EQ(top->RD1, 1);             // bDim
    EXPECT_EQ(top->RD2, 0);             // lId
}

TEST_F(RegfileTestbench, AsyncReadCheck) {
    int value = 0xFEEDFACE;

    top->AD3 = 10;
    top->WD3 = value;
    top->WE3 = 1;
    top->thread_write = 0;

    runSimulation(1);

    top->AD1 = 10;
    top->AD2 = 10;
    top->thread_read = 0;

    top->eval(); // combinational async read

    EXPECT_EQ(top->RD1, value);
    EXPECT_EQ(top->RD2, value);
    EXPECT_EQ(top->a0, value); // a0 should mirror x10
}

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    auto res = RUN_ALL_TESTS();
    return res;
}
