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
    {"addi", {0b001, 0b0000}}, {"muli", {0b001, 0b0010}}, {"slli", {0b001, 0b1010}},
    {"divi", {0b001, 0b0011}}
};

unordered_map<string, pair<int, int>> fTypeFunctMap = {
    {"fadd.s", {0b010, 0b0000}}, {"fsub.s", {0b010, 0b0001}}, {"fmul.s", {0b010, 0b0010}},
    {"fdiv.s", {0b010, 0b0011}}, {"flt.s", {0b010, 0b0100}}, {"fneg.s", {0b010, 0b0101}},
    {"feq.s",  {0b010, 0b0110}}, {"fmin.s", {0b010, 0b0111}}, {"fabs.s", {0b010, 0b1000}},
    {"fcvt.w.s", {0b010, 0b1001}}, {"fcvt.s.w", {0b010, 0b1010}}
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
        auto it = symbolTable.find(sym);
        if (it == symbolTable.end()) {
            cerr << "Undefined symbol in %hi(): " << sym << endl;
            return 0;
        }
        return (it->second >> 12) & 0xFFFFF;  // Get upper 20 bits
    } else if (expr.rfind("%lo(", 0) == 0 && expr.back() == ')') {
        string sym = expr.substr(4, expr.size() - 5);
        auto it = symbolTable.find(sym);
        if (it == symbolTable.end()) {
            cerr << "Undefined symbol in %lo(): " << sym << endl;
            return 0;
        }
        return it->second & 0xFFF;  // Get lower 12 bits
    } else if (expr.find_first_not_of("0123456789-") == string::npos) {
        return stoi(expr);
    } else {
        // Try to look up as a plain symbol
        auto it = symbolTable.find(expr);
        if (it != symbolTable.end()) {
            return it->second;
        }
        cerr << "Undefined symbol: " << expr << endl;
        return 0;
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
    int rd = registerMap[args[1]];
    int rs1 = registerMap[args[2]];
    int rs2 = 0;

    // For binary ops like fadd.s, fsub.s, fmul.s, fdiv.s, etc.
    if (args.size() > 2 && (
        op == "fadd.s" || op == "fsub.s" || op == "fmul.s" ||
        op == "fdiv.s" || op == "flt.s" || op == "feq.s" || op == "fmin.s")) {
        rs2 = registerMap[args[3]];
    }

    int funct4 = fTypeFunctMap[op].second;

    return (opcode << 29) | (pred << 28) | (0 << 19) |
           (rs2 << 14) | (funct4 << 10) | (rs1 << 5) | rd;
}

uint32_t encodeLoad(string op, const vector<string>& args) {
    int opcode = 0b100, funct4 = 0b0000;
    int rd = registerMap[args[0]];

    string fullExpr = args[1];
    size_t paren = fullExpr.rfind('(');  // use last '('
    if (paren == string::npos || fullExpr.back() != ')') {
        cerr << "Invalid load format: " << fullExpr << endl;
        return 0;
    }

    string immStr = fullExpr.substr(0, paren);  // %lo(.LC0)
    string rs1Str = fullExpr.substr(paren + 1, fullExpr.size() - paren - 2);  // t0

    int imm = resolveSymbol(immStr, labelMap);
    int rs1 = registerMap[rs1Str];

    return (opcode << 29) | ((imm & 0x7FFF) << 14) | (funct4 << 10) | (rs1 << 5) | rd;
}


uint32_t encodeStore(const vector<string>& args) {
    int opcode = 0b100, funct4 = 0b0001;
    int rs2 = registerMap[args[0]];
    int paren = args[1].find('(');
    string immStr = args[1].substr(0, paren);
    string rs1Str = args[1].substr(paren + 1, args[1].size() - paren - 2);
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

    if (op == "ret") {
        return (opcode << 29) | (funct3 << 10) | (1 << 5);
    }

    if (op == "j") {
        string label = args[1];
        int target = labelMap[label];
        //std::cout << "Jump label" << label << std::endl;
        //std::cout << "Target" << target << std::endl;
        int32_t offset = (target - (pc + 4)) / 4;
        //std::cout << "Offset" << offset << std::endl;
        int32_t imm = offset & 0x3FFFFFF;  
        uint32_t imm_hi = (imm >> 10) & 0xFFFF; 
        uint32_t imm_lo = imm & 0x3FF;          
    
        return (opcode << 29) | (imm_hi << 13) | (funct3 << 10) | imm_lo;
    }

    if (op == "beqz") {
        string rs1_str = args[1];
        string label = args[2];
        int rs1 = registerMap[rs1_str];
        int rs2 = 0;  // hardcoded to x0
        int target = labelMap[label];
        std::cout << "Label: " << label << std::endl;
        std::cout << "Target: " << target << std::endl;    
        int32_t offset = (target - (pc + 4)) / 4;
        std::cout << "Offset: " << offset << std::endl;
    
        // Sign-extend and mask offset to 18 bits
        int32_t imm = offset & 0x3FFFF;  // 18-bit signed immediate
    
        uint32_t imm_17_8 = (imm >> 8) & 0x3FF;   // bits [17:8] -> [28:19]
        uint32_t imm_7    = (imm >> 7) & 0x1;     // bit [7]     -> [13]
        uint32_t imm_6_2  = imm & 0x1F;           // bits [6:2]  -> [4:0]
    
        return (opcode << 29)
             | (imm_17_8 << 19)
             | (rs2 << 14)
             | (imm_7 << 13)
             | (funct3 << 10)
             | (rs1 << 5)
             | imm_6_2;
    }
    

    // Fallback (sync, exit, etc)
    return (opcode << 29) | (funct3 << 10);
}


uint32_t encodeLUI(const vector<string>& args) {
    int opcode = 0b011;
    int rd = registerMap[args[1]];
    int upimm = resolveSymbol(args[2], labelMap);
    return (opcode << 29) | ((upimm & 0xFFFFF) << 9) | rd;
}

uint32_t encodePseudoLI(const vector<string>& args) {
    return encodeIType("addi", {args[1], "zero", args[2]});
}

int main() {
    initRegisterMap();
    ifstream input("bin/output/algotests/for/for.s");
    ofstream output("rtl/program.hex");
    vector<pair<int, string>> instructions;
    vector<pair<int, uint32_t>> data;
    string line;
    int pc = 0;

    // First pass - collect labels and instructions
    while (getline(input, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line.find(".text") != string::npos ||
            line.find(".globl") != string::npos ||
            line.find(".type") != string::npos ||
            line.find(".section") != string::npos ||
            line.find(".size") != string::npos ||
            line.find(".data") != string::npos) {
            continue;
        }

        // Handle labels
        auto colon = line.find(':');
        if (colon != string::npos) {
            string label = line.substr(0, colon);
            //std::cout << "New label added: " << label << " with pc of " << pc << std::endl;
            labelMap[label] = pc;
            line = line.substr(colon + 1);
        }
        
        // Handle data directives
        if (line.find(".word") != string::npos) {
            stringstream ss(line.substr(line.find(".word") + 5));
            uint32_t value;
            ss >> value;
            data.emplace_back(pc, value);
            pc += 4;
            continue;
        }

        if (line.find(".zero") != string::npos) {
            stringstream ss(line.substr(line.find(".zero") + 5));
            int size;
            ss >> size;
            data.emplace_back(pc, 0);
            pc += size;
            continue;
        }
        
        if (line.find(".align") != string::npos) {
            // Handle alignment if needed
            continue;
        }
        
        if (!line.empty()) {
            instructions.emplace_back(pc, line);
            pc += 4;
        }
    }

    // Second pass - output instructions
    for (auto& [pc, line] : instructions) {
        vector<string> tokens = tokenize(line);
        if (tokens.empty()) continue;
        string op = tokens[0];
        uint32_t instr = 0;
        
        if (rTypeFunctMap.count(op)) instr = encodeRType(op, {tokens[1], tokens[2], tokens[3]});
        else if (iTypeFunctMap.count(op)) instr = encodeIType(op, {tokens[1], tokens[2], tokens[3]});
        else if (fTypeFunctMap.count(op)) instr = encodeFType(op, tokens);
        else if (op == "lw" || op == "flw") instr = encodeLoad(op, {tokens[1], tokens[2]});
        else if (op == "sw" || op == "fsw") instr = encodeStore({tokens[1], tokens[2]});
        else if (cTypeFunctMap.count(op)) instr = encodeControl(op, tokens, pc);
        else if (op == "lui") instr = encodeLUI(tokens);
        else if (op == "li") instr = encodePseudoLI(tokens);
        else {
            cerr << "Unknown instruction: " << op << endl;
            continue;
        }

        //Modelled into this form as this is how the instr_mem.sv reads the instrs from the hex file
        output << hex << setw(2) << setfill('0') << ((instr >> 0) & 0xFF) << endl;
        output << hex << setw(2) << setfill('0') << ((instr >> 8) & 0xFF) << endl;
        output << hex << setw(2) << setfill('0') << ((instr >> 16) & 0xFF) << endl;
        output << hex << setw(2) << setfill('0') << (instr >> 24) << endl;
    }

    // Output data section
    for (auto& [addr, value] : data) {
        output << hex << setw(2) << setfill('0') << ((value >> 0) & 0xFF) << endl;
        output << hex << setw(2) << setfill('0') << ((value >> 8) & 0xFF) << endl;
        output << hex << setw(2) << setfill('0') << ((value >> 16) & 0xFF) << endl;
        output << hex << setw(2) << setfill('0') << (value >> 24) << endl;
    }

    input.close();
    output.close();
    return 0;
}