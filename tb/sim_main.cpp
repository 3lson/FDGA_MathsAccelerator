#include "Vscalar_cpu.h"
#include "verilated.h"
#include <iostream>

vluint64_t main_time = 0;
double sc_time_stamp() { return main_time; }

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    Vscalar_cpu* top = new Vscalar_cpu;

    // Initialize inputs
    top->clk = 0;
    top->rst = 1;

    // Reset for some cycles (hold reset for 4 full clock cycles)
    for (int i = 0; i < 8; ++i) { // toggle clk 8 times = 4 full cycles
        top->clk = !top->clk;
        top->eval();
        main_time++;
    }
    top->rst = 0;

    // Run simulation for 20 full clock cycles
    for (int cycle = 0; cycle < 20; ++cycle) {
        // Rising edge
        top->clk = 1;
        top->eval();
        main_time++;

        // Print values at rising edge
        std::cout << "Cycle " << cycle
                  << " PC=0x" << std::hex << top->pc
                  << " Instr=0x" << std::hex << top->instruction
                  << " Opcode=0x" << std::hex << (int)top->opcode
                  << " RD=" << std::dec << (int)top->rd
                  << " RS1=" << (int)top->rs1
                  << " RS2=" << (int)top->rs2
                  << " IMM=0x" << std::hex << top->imm
                  << std::endl;

        // Falling edge
        top->clk = 0;
        top->eval();
        main_time++;
    }

    top->final();
    delete top;
    return 0;
}
