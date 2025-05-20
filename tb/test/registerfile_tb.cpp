#include "sync_testbench.h"
#include <verilated_cov.h>

#define NAME "regfile"

class RegfileTestbench : public SyncTestbench {
protected:
    void initializeInputs() override {
        top->clk = 1;
        top->rst = 0;
        top->AD1 = 0;
        top->AD2 = 0;
        top->AD3 = 0;
        top->WE3 = 0;
        top->WD3 = 0;
    }
};

TEST_F(RegfileTestbench, RegZeroHardWiredTest) {
    int randomNumber = 0x12345678;

    // Attempt to write to x0
    top->AD3 = 0;
    top->WD3 = randomNumber;
    top->WE3 = 1;

    runSimulation(1);

    // Read x0
    top->AD1 = 0;
    top->AD2 = 0;

    runSimulation(1);

    // Ensure x0 remains zero
    EXPECT_EQ(top->RD1, 0);
    EXPECT_EQ(top->RD2, 0);
}

TEST_F(RegfileTestbench, AllRegsFunctionTest) {
    int randomNumber = 0x12345678;

    // Write to all registers except x0
    top->WE3 = 1;

    for (int i = 1; i < 32; ++i) {
        top->AD3 = i;
        top->WD3 = randomNumber;
        runSimulation(1);

        // Read back from both ports
        top->AD1 = i;
        top->AD2 = i;

        runSimulation(1);

        EXPECT_EQ(top->RD1, randomNumber);
        EXPECT_EQ(top->RD2, randomNumber);
    }
}

TEST_F(RegfileTestbench, AsyncReadTest) {
    int randomNumber = 0x12345678;

    // Write to register x10
    top->AD3 = 10;
    top->WD3 = randomNumber;
    top->WE3 = 1;

    runSimulation(1);

    // Asynchronous read of x10
    top->AD1 = 10;
    top->AD2 = 10;

    // Evaluate without clock cycle
    top->eval();

    EXPECT_EQ(top->RD1, randomNumber);
    EXPECT_EQ(top->RD2, randomNumber);
}

TEST_F(RegfileTestbench, ResetTest) {
    int randomNumber = 0x12345678;

    // Write to all registers except x0
    top->WE3 = 1;
    for (int i = 1; i < 32; ++i) {
        top->AD3 = i;
        top->WD3 = randomNumber;
        runSimulation(1);
    }

    // Assert reset
    top->rst = 1;
    runSimulation(1);
    top->rst = 0;

    // Verify all registers are reset to zero
    for (int i = 0; i < 32; ++i) {
        top->AD1 = i;
        top->AD2 = i;
        runSimulation(1);

        EXPECT_EQ(top->RD1, 0);
        EXPECT_EQ(top->RD2, 0);
    }
}

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    Verilated::mkdir("logs");
    auto res = RUN_ALL_TESTS();
    return res;
}
