#include "Vfloating_alu.h"
#include "verilated.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstring>
#include <cmath>
#include <bitset>

// Convert 32-bit float to raw bits
uint32_t floatToBits(float f) {
    uint32_t bits;
    std::memcpy(&bits, &f, sizeof(float));
    return bits;
}

// Convert raw bits to float
float bitsToFloat(uint32_t bits) {
    float f;
    std::memcpy(&f, &bits, sizeof(uint32_t));
    return f;
}

// Print float as both float and hex
void printFloat32(uint32_t bits) {
    std::cout << std::fixed << std::setprecision(6)
              << bitsToFloat(bits)
              << " (0x" << std::hex << std::setw(8) << std::setfill('0') << bits << std::dec << ")";
}

const char* opSymbol(uint8_t alu_op) {
    switch (alu_op) {
        case 1: return "+";
        case 2: return "-";
        case 3: return "*";
        case 4: return "/";
        case 5: return "|·|";
        case 6: return "==";
        case 7: return "!=";
        case 8: return "<";
        default: return "?";
    }
}

struct TestCase {
    float op1;
    float op2;
    uint8_t alu_op;
    const char* desc;
};

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);

    Vfloating_alu* alu = new Vfloating_alu;

    std::vector<TestCase> tests = {
        // --- FADD (1) ---
        {1.0f, 1.0f, 1, "1.0 + 1.0 = 2.0"},
        {2.0f, 1.0f, 1, "2.0 + 1.0 = 3.0"},
        {1.0f, -1.0f, 1, "1.0 + (-1.0) = 0.0"},

        // --- FSUB (2) ---
        {1.0f, 1.0f, 2, "1.0 - 1.0 = 0.0"},
        {2.0f, 1.0f, 2, "2.0 - 1.0 = 1.0"},
        {1.0f, 2.0f, 2, "1.0 - 2.0 = -1.0"},
        {-1.0f, -1.0f, 2, "-1.0 - (-1.0) = 0.0"},

        // --- FMUL (3) ---
        {1.0f, 1.0f, 3, "1.0 * 1.0 = 1.0"},
        {2.0f, 1.0f, 3, "2.0 * 1.0 = 2.0"},
        {-2.0f, 2.0f, 3, "-2.0 * 2.0 = -4.0"},
        {1.0f, 0.0f, 3, "1.0 * 0.0 = 0.0"},

        // --- FDIV (4) ---
        {2.0f, 1.0f, 4, "2.0 / 1.0 = 2.0"},
        {1.0f, 2.0f, 4, "1.0 / 2.0 = 0.5"},
        {-2.0f, 2.0f, 4, "-2.0 / 2.0 = -1.0"},
        {1.0f, 1e-10f, 4, "1.0 / very small → very large"},

        // --- FABS (5) ---
        {-1.0f, 0.0f, 5, "|-1.0| = 1.0"},
        {1.0f, 0.0f, 5, "|1.0| = 1.0"},
        {0.0f, 0.0f, 5, "|0.0| = 0.0"},

        // --- FEQ (6) ---
        {1.0f, 1.0f, 6, "1.0 == 1.0 → 1"},
        {1.0f, 2.0f, 6, "1.0 == 2.0 → 0"},
        {-1.0f, -1.0f, 6, "-1.0 == -1.0 → 1"},

        // --- FNE (7) ---
        {1.0f, 2.0f, 7, "1.0 != 2.0 → 1"},
        {1.0f, 1.0f, 7, "1.0 != 1.0 → 0"},

        // --- FLT (8) ---
        {1.0f, 2.0f, 8, "1.0 < 2.0 → 1"},
        {2.0f, 1.0f, 8, "2.0 < 1.0 → 0"},
        {-1.0f, 1.0f, 8, "-1.0 < 1.0 → 1"},
        {2.0f, 2.0f, 8, "2.0 < 2.0 → 0"},
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        auto& test = tests[i];
        alu->alu_op = test.alu_op;
        alu->op1 = floatToBits(test.op1);
        alu->op2 = floatToBits(test.op2);
        alu->eval();

        std::cout << "Test " << std::setw(2) << i + 1 << ": ";
        printFloat32(floatToBits(test.op1));
        std::cout << " " << opSymbol(test.alu_op) << " ";
        printFloat32(floatToBits(test.op2));
        std::cout << " = ";
        printFloat32(alu->result);
        std::cout << "   cmp: " << (alu->cmp ? "1" : "0");
        std::cout << "   // " << test.desc << "\n";
    }

    delete alu;
    return 0;
}
