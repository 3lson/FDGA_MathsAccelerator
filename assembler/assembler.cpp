#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <bitset>
#include <vector>
#include <iomanip>
#include <filesystem>

using namespace std;

unordered_map<string, int> registerMap;
unordered_map<string, pair<int, int>> rTypeFunctMap = {
    {"add", {0b000, 0b0000}}, {"sub", {0b000, 0b0001}}, {"mul", {0b000, 0b0010}},
    {"div", {0b000, 0b0011}}, {"slt", {0b000, 0b0100}}, {"seq", {0b000, 0b0101}},
    {"min", {0b000, 0b0110}}, {"abs", {0b000, 0b0111}},
};

unordered_map<string, pair<int, int>> iTypeFunctMap = {
    {"addi", {0b001, 0b0000}}, {"muli", {0b001, 0b0001}}, {"divi", {0b001, 0b0100}},
};

unordered_map<string, int> cTypeFunctMap = {
    {"jump", 0b000}, {"branch", 0b001}, {"call", 0b010},
    {"ret", 0b011}, {"sync", 0b110}, {"exit", 0b111},
};

void initRegisterMap() {
    for (int i = 0; i < 32; ++i)
        registerMap["x" + to_string(i)] = i;
}

vector<string> tokenize(const string& line) {
    string cleaned;
    for (char c : line)
        cleaned += (c == ',' ? ' ' : c);
    istringstream iss(cleaned);
    vector<string> tokens;
    string tok;
    while (iss >> tok) tokens.push_back(tok);
    return tokens;
}

int getImm(const string& s) {
    int val = stoi(s);
    if (val < 0) val = (1 << 14) + val;
    return val;
}

string toHex(uint32_t value) {
    stringstream ss;
    ss << hex << setw(8) << setfill('0') << value;
    return ss.str();
}

uint32_t encodeRType(string op, const vector<string>& args) {
    int pred = 0;  // optional
    int opcode = 0b000;
    int rd = registerMap[args[0]];
    int rs1 = registerMap[args[1]];
    int rs2 = registerMap[args[2]];
    int funct4 = rTypeFunctMap[op].second;
    return (opcode << 29) | (pred << 28) | (0 << 19) | (rs2 << 14) | (funct4 << 10) | (rs1 << 5) | rd;
}

uint32_t encodeIType(string op, const vector<string>& args) {
    int pred = 0;
    int opcode = 0b001;
    int rd = registerMap[args[0]];
    int rs1 = registerMap[args[1]];
    int imm = getImm(args[2]) & 0x3FFF;  // 14 bits
    int funct4 = iTypeFunctMap[op].second;
    return (opcode << 29) | (pred << 28) | (imm << 14) | (funct4 << 10) | (rs1 << 5) | rd;
}

uint32_t encodeLoad(const vector<string>& args) {
    int opcode = 0b100;
    int imm = 0;  // assuming (rs1)
    int rs1 = registerMap[args[1].substr(1, args[1].length() - 2)];
    int rd = registerMap[args[0]];
    return (opcode << 29) | (imm << 14) | (0b0000 << 10) | (rs1 << 5) | rd;
}

uint32_t encodeStore(const vector<string>& args) {
    int opcode = 0b100;
    int rs2 = registerMap[args[0]];
    int rs1 = registerMap[args[2].substr(1, args[2].length() - 2)];
    int imm = 0;
    return (opcode << 29) | ((imm >> 5) << 19) | (rs2 << 14) | (0b0001 << 10) | (rs1 << 5) | (imm & 0x1F);
}

uint32_t encodeControl(string op, const vector<string>& args) {
    int opcode = 0b111;
    int funct3 = cTypeFunctMap[op];
    if (op == "ret") return (opcode << 29) | (0 << 13) | (funct3 << 10) | (1 << 5);
    return (opcode << 29) | (0 << 13) | (funct3 << 10);
}

int main() {
    initRegisterMap();
    ifstream input("assembler/program.asm");
    ofstream output("rtl/program.hex");
    system("mkdir -p rtl");

    string line;
    while (getline(input, line)) {
        if (line.empty() || line[0] == '#') continue;
        vector<string> tokens = tokenize(line);
        string op = tokens[0];
        uint32_t instr = 0;

        if (rTypeFunctMap.count(op)) {
            instr = encodeRType(op, {tokens[1], tokens[2], tokens[3]});
        } else if (iTypeFunctMap.count(op)) {
            instr = encodeIType(op, {tokens[1], tokens[2], tokens[3]});
        } else if (op == "lw") {
            instr = encodeLoad({tokens[1], tokens[2]});
        } else if (op == "sw") {
            instr = encodeStore({tokens[1], tokens[2], tokens[3]});
        } else if (cTypeFunctMap.count(op)) {
            instr = encodeControl(op, tokens);
        } else {
            cerr << "Unknown instruction: " << op << endl;
            continue;
        }

        output << hex << setw(2) << setfill('0') << ((instr >> 0) & 0xFF) << endl;
        output << hex << setw(2) << setfill('0') << ((instr >> 8) & 0xFF) << endl;
        output << hex << setw(2) << setfill('0') << ((instr >> 16) & 0xFF) << endl;
        output << hex << setw(2) << setfill('0') << ((instr >> 24) & 0xFF) << endl;

    }

    input.close();
    output.close();
    return 0;
}
