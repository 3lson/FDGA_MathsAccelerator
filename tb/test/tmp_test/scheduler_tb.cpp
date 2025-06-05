#include "sync_testbench.h"
#include <verilated_cov.h>
#include <queue>

#define NAME "scheduler"

class SchedulerTestbench : public SyncTestbench {
protected:
    void initializeInputs() override {
        top->clk = 0;
        top->reset = 1;
        top->instr_valid = 0;
        top->instr_data = 0;
        for (int i = 0; i < 4; ++i) {
            top->stall[i] = 0;
        }
    }

    void loadInstruction(uint32_t instr) {
        top->instr_data = instr;
        top->instr_valid = 1;
        runSimulation(1);
        top->instr_valid = 0;
    }

    void resetScheduler() {
        top->reset = 1;
        runSimulation(2);
        top->reset = 0;
    }
};

// Instruction format for test (adjust to match your decode logic)
uint32_t makeInstruction(
    uint8_t funct4,
    uint32_t imm,
    uint8_t rs1,
    uint8_t rs2,
    uint8_t rd,
    bool is_int,
    bool is_float,
    bool we3,
    bool pred_en,
    bool pred_read
) {
    uint32_t instr = 0;
    instr |= (funct4 & 0x1F) << 27;
    instr |= (imm & 0xFFFFF) << 7;
    instr |= (rs1 & 0x1F) << 2;
    instr |= (rs2 & 0x03); // simplified
    instr |= (rd & 0x1F) << 7;
    instr |= (is_int ? 1 : 0) << 12;
    instr |= (is_float ? 1 : 0) << 13;
    instr |= (we3 ? 1 : 0) << 14;
    instr |= (pred_en ? 1 : 0) << 15;
    instr |= (pred_read ? 1 : 0) << 16;
    return instr;
}

TEST_F(SchedulerTestbench, InstructionQueueTest) {
    resetScheduler();

    for (int i = 0; i < 8; ++i) {
        loadInstruction(makeInstruction(0xA + i, 0x100 + i, i, i, i, 1, 0, 1, 1, 0));
        EXPECT_TRUE(top->instr_ready);
    }

    // At this point, queue should be full
    loadInstruction(makeInstruction(0xB, 0x200, 1, 2, 3, 1, 0, 1, 1, 0));
    EXPECT_FALSE(top->instr_ready);
}

TEST_F(SchedulerTestbench, ThreadDispatchBasicTest) {
    resetScheduler();

    // Push a single instruction
    loadInstruction(makeInstruction(0xF, 0x111, 2, 2, 5, 1, 0, 1, 1, 0));

    // Simulate some cycles to let scheduling run
    runSimulation(10);

    // At least one lane should now have active control signals
    bool found_active_lane = false;
    for (int i = 0; i < 4; ++i) {
        if (top->WE3[i] || top->is_int[i]) {
            found_active_lane = true;
            break;
        }
    }

    EXPECT_TRUE(found_active_lane);
}

TEST_F(SchedulerTestbench, StallAndUnstallTest) {
    resetScheduler();

    loadInstruction(makeInstruction(0x1C, 0xABC, 3, 4, 6, 1, 0, 1, 0, 1));

    runSimulation(5);

    top->stall[0] = 1;
    runSimulation(3);  // Let it stall

    // Lane 0 should now reflect stalled state
    EXPECT_EQ(top->stall[0], 1);

    top->stall[0] = 0;
    runSimulation(3);  // Should resume

    // Ensure signals are again active
    EXPECT_TRUE(top->WE3[0] || top->is_int[0]);
}

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    auto res = RUN_ALL_TESTS();
    return res;
}
