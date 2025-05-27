#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <bitset>
#include <vector>
#include <iomanip>
#include <filesystem>
#include <map>

using namespace std;

// The assembler works by breaking down the instr sw ra, 268(sp)
// args[0] = sw, args[1] = ra, args[2] = 268(sp)


unordered_map<string, int> registerMap;
unordered_map<string, pair<int, int>> rTypeFunctMap = {
    {"add", {0b000, 0b0000}}, {"sub", {0b000, 0b0001}}, {"mul", {0b000, 0b0010}},
    {"div", {0b000, 0b0011}}, {"slt", {0b000, 0b0100}}, {"sgt", {0b000, 0b0101}},
    {"seq", {0b000, 0b0110}}, {"snez", {0b000, 0b0111}}, {"min", {0b000, 0b1000}},
    {"abs", {0b000, 0b1001}}
};

unordered_map<string, pair<int, int>> iTypeFunctMap = {
    {"addi", {0b001, 0b0000}}, {"muli", {0b001, 0b0001}}, {"slli", {0b001, 0b0010}},
    {"divi", {0b001, 0b0100}}
};

unordered_map<string, pair<int, int>> fTypeFunctMap = {
    {"fadd.s", {0b010, 0b0000}}, {"fsub.s", {0b010, 0b0001}}, {"fmul.s", {0b010, 0b0010}},
    {"fdiv.s", {0b010, 0b0011}}, {"fslt.s", {0b010, 0b0100}}, {"fneg.s", {0b010, 0b0101}},
    {"feq.s",  {0b010, 0b0110}}, {"fmin.s", {0b010, 0b0111}}, {"fabs.s", {0b010, 0b1000}},
    {"fcvt.s.w", {0b010, 0b1001}}
};

unordered_map<string, int> cTypeFunctMap = {
    {"j", 0b000}, {"beqz", 0b001}, {"call", 0b010},
    {"ret", 0b011}, {"sync", 0b110}, {"exit", 0b111}
};

map<string, int> labelMap;

void initRegisterMap() {
    for (int i = 0; i < 32; ++i)
        registerMap["x" + to_string(i)] = i;

    registerMap["zero"] = 0; registerMap["ra"] = 1; registerMap["sp"] = 2;
    registerMap["gp"] = 3; registerMap["tp"] = 4; registerMap["t0"] = 5;
    registerMap["t1"] = 6; registerMap["t2"] = 7; registerMap["s0"] = 8;
    registerMap["fp"] = 8; registerMap["s1"] = 9; registerMap["a0"] = 10;
    registerMap["a1"] = 11; registerMap["a2"] = 12; registerMap["a3"] = 13;
    registerMap["a4"] = 14; registerMap["a5"] = 15; registerMap["a6"] = 16;
    registerMap["a7"] = 17; registerMap["s2"] = 18; registerMap["s3"] = 19;
    registerMap["s4"] = 20; registerMap["s5"] = 21; registerMap["s6"] = 22;
    registerMap["s7"] = 23; registerMap["s8"] = 24; registerMap["s9"] = 25;
    registerMap["s10"] = 26; registerMap["s11"] = 27; registerMap["t3"] = 28;
    registerMap["t4"] = 29; registerMap["t5"] = 30; registerMap["t6"] = 31;

    //This is fine since we have no function arguments
    // Floating-point register aliases (mapped to same as t0-t6)
    registerMap["ft0"] = 5;  
    registerMap["ft1"] = 6;  
    registerMap["ft2"] = 7;  
    registerMap["ft7"] = 17; 
    registerMap["fa2"] = 12;
    registerMap["fa3"]  = 13;
    registerMap["fa4"] = 14;
}

int resolveSymbol(const string& expr, const map<string, int>& symbolTable) {
    if (expr.rfind("%hi(", 0) == 0 && expr.back() == ')') {
        string sym = expr.substr(4, expr.size() - 5);
        return symbolTable.at(sym) >> 12;
    } else if (expr.rfind("%lo(", 0) == 0 && expr.back() == ')') {
        string sym = expr.substr(4, expr.size() - 5);
        return symbolTable.at(sym) & 0xFFF;
    } else {
        return stoi(expr);
    }
}

vector<string> tokenize(const string& line) {
    string cleaned;
    for (char c : line) cleaned += (c == ',' ? ' ' : c);
    istringstream iss(cleaned);
    vector<string> tokens;
    string tok;
    while (iss >> tok) tokens.push_back(tok);
    return tokens;
}

int getImm(const string& s) {
    try {
        return resolveSymbol(s, labelMap);
    } catch (...) {
        return 0;
    }
}

uint32_t encodeRType(string op, const vector<string>& args) {
    int opcode = 0b000, pred = 0;
    int rd = registerMap[args[0]], rs1 = registerMap[args[1]], rs2 = registerMap[args[2]];
    int funct4 = rTypeFunctMap[op].second;
    return (opcode << 29) | (pred << 28) | (0 << 19) | (rs2 << 14) | (funct4 << 10) | (rs1 << 5) | rd;
}

uint32_t encodeIType(string op, const vector<string>& args) {
    int opcode = 0b001, pred = 0;
    int rd = registerMap[args[0]], rs1 = registerMap[args[1]];
    int imm = getImm(args[2]) & 0x3FFF;
    int funct4 = iTypeFunctMap[op].second;
    return (opcode << 29) | (pred << 28) | (imm << 14) | (funct4 << 10) | (rs1 << 5) | rd;
}

uint32_t encodeFType(string op, const vector<string>& args) {
    int opcode = 0b010, pred = 0;
    int rd = registerMap[args[0]], rs1 = registerMap[args[1]];
    int rs2 = args.size() > 2 ? registerMap[args[2]] : 0;
    int funct4 = fTypeFunctMap[op].second;
    return (opcode << 29) | (pred << 28) | (0 << 19) | (rs2 << 14) | (funct4 << 10) | (rs1 << 5) | rd;
}

uint32_t encodeLoad(string op, const vector<string>& args) {
    int opcode = 0b100, funct4 = 0b0000;
    int rd = registerMap[args[0]];
    int paren = args[1].find('(');
    string immStr = args[1].substr(0, paren);
    string rs1Str = args[1].substr(paren + 1, args[1].size() - paren - 2);
    int imm = resolveSymbol(immStr, labelMap);
    int rs1 = registerMap[rs1Str];
    return (opcode << 29) | ((imm & 0x7FFF) << 14) | (funct4 << 10) | (rs1 << 5) | rd;
}

uint32_t encodeStore(const vector<string>& args) {
    int opcode = 0b100, funct4 = 0b0001;
    int rs2 = registerMap[args[1]];
    int paren = args[2].find('(');
    string immStr = args[2].substr(0, paren);
    string rs1Str = args[2].substr(paren + 1, args[2].size() - paren - 2);
    int imm = resolveSymbol(immStr, labelMap);
    int rs1 = registerMap[rs1Str];
    uint32_t uimm = static_cast<uint32_t>(imm) & 0x7FFF;
    uint32_t imm_hi = (uimm >> 5) & 0x3FF;
    uint32_t imm_lo = uimm & 0x1F;
    return (opcode << 29) | (imm_hi << 19) | (rs2 << 14) | (funct4 << 10) | (rs1 << 5) | imm_lo;
}

uint32_t encodeControl(string op, const vector<string>& args, int pc) {
    int opcode = 0b111;
    int funct3 = cTypeFunctMap[op];

    if (op == "ret")
        return (opcode << 29) | (funct3 << 10) | (1 << 5);

    if (op == "j" || op == "beqz") {
        string label = args[2];
        int target = labelMap[label];
        std::cout << label << std::endl;
        std::cout << target << std::endl;
        int offset = (target - (pc + 4)) / 4;
        std::cout << offset << std::endl;
        offset &= 0x3FFFF;
        std::cout << offset << 13 << std::endl;
        return (opcode << 29) | (offset << 13) | (funct3 << 10) | (registerMap[args[1]] << 5);
    }

    return (opcode << 29) | (funct3 << 10);
}



uint32_t encodeLUI(const vector<string>& args) {
    int opcode = 0b011;
    int rd = registerMap[args[0]];
    int upimm = resolveSymbol(args[1], labelMap);
    return (opcode << 29) | (upimm << 9) | rd;
}

uint32_t encodePseudoLI(const vector<string>& args) {
    return encodeIType("addi", {args[1], "zero", args[2]});
}

int main() {
    initRegisterMap();
    ifstream input("assembler/program.asm");
    ofstream output("assembler/rtl/program.hex");
    vector<pair<int, string>> instructions;
    string line;
    int pc = 0;

    while (getline(input, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto colon = line.find(':');
        if (colon != string::npos) {
            string label = line.substr(0, colon);
            labelMap[label] = pc;
            line = line.substr(colon + 1);
            // If the label is followed by .word, it still takes up 4 bytes
            if (line.find(".word") != string::npos) {
                pc += 4;
                continue;
            }
        }
        if (line.find(".word") != string::npos) {
            pc += 4;
            continue;
        }
        
        if (line.find(".align") != string::npos) {
            continue;
        }
        if (!line.empty()) {
            instructions.emplace_back(pc, line);
            pc += 4;
        }
    }

    for (auto& [pc, line] : instructions) {
        vector<string> tokens = tokenize(line);
        if (tokens.empty()) continue;
        string op = tokens[0];
        uint32_t instr = 0;
        if (rTypeFunctMap.count(op)) instr = encodeRType(op, {tokens[1], tokens[2], tokens[3]});
        else if (iTypeFunctMap.count(op)) instr = encodeIType(op, {tokens[1], tokens[2], tokens[3]});
        else if (fTypeFunctMap.count(op)) instr = encodeFType(op, tokens);
        else if (op == "lw" || op == "flw") instr = encodeLoad(op, {tokens[1], tokens[2]});
        else if (op == "sw" || op == "fsw") instr = encodeStore(tokens);
        else if (cTypeFunctMap.count(op)) instr = encodeControl(op, tokens, pc);
        else if (op == "lui") instr = encodeLUI(tokens);
        else if (op == "li") instr = encodePseudoLI(tokens);
        else {
            cerr << "Unknown instruction: " << op << endl;
            continue;
        }

        // write 4 bytes, LSB first
        for (int i = 0; i < 4; ++i) {
            uint8_t byte = (instr >> (i * 8)) & 0xFF;   // pick out byte i
            output << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(byte) << '\n';
    }
    }

    input.close();
    output.close();
    return 0;
}
