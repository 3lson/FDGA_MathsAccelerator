/*
Assembler for custom ISA:

# Includes
- R-type, I-type, M-type, X-type

# TODO
- Control Flow and other MISC
- Make it cleaner is header file definition mapping that way later
- Did not define NOPs yet
*/

#include <iostream>
#include <sstream>
#include <fstream> 
#include <unordered_map>
#include <vector>
#include <bitset>
#include <iomanip>
#include <regex>
#include <algorithm>

using namespace std;

// Register alias to ID mapping
unordered_map<string, int> reg_map = {
    {"x0", 0}, {"x1", 1}, {"x2", 2}, {"x3", 3}, {"x4", 4}, {"x5", 5},
    {"x6", 6}, {"x7", 7}, {"x8", 8}, {"x9", 9}, {"x10", 10}, {"x11", 11},
    {"x12", 12}, {"x13", 13}, {"x14", 14}, {"x15", 15},
    {"x16", 16}, {"x17", 17}, {"x18", 18}, {"x19", 19}, {"x20", 20},
    {"x21", 21}, {"x22", 22}, {"x23", 23}, {"x24", 24}, {"x25", 25},
    {"x26", 26}, {"x27", 27}, {"x28", 28}, {"x29", 29}, {"x30", 30}, {"x31", 31}
};

// FUNCT5 values for R and I types
unordered_map<string, int> funct5_map = {
    {"ADD", 0x00}, {"SUB", 0x01}, {"MUL", 0x02}, {"MAC", 0x03},
    {"DIV", 0x04}, {"REM", 0x05}, {"SLL", 0x0A}, {"SRL", 0x0B},
    {"SQRT", 0x0C}, {"SLT", 0x1C}, {"SEQ", 0x1D}, {"NOP", 0x1F},
    {"ADDI", 0x00}, {"MULI", 0x02}, {"MACI", 0x03}, {"DIVI", 0x04},
    {"CLAMP", 0x06}, {"VLOADLEN", 0x08}, {"VSETDIM", 0x09}
};

// FUNCT2 for X-type
unordered_map<string, int> funct2_x = {
    {"DIST_VEC", 0x0}, {"ASSIGN_CLUSTER", 0x1},
    {"UPDATE_CENTROID", 0x2}, {"FINALIZE_CENTROID", 0x3}
};

int encode_rtype(const string& mnemonic, int rd, int rs1, int rs2) {
    int opcode = 0b000;
    int funct5 = funct5_map[mnemonic];
    return (opcode << 29) | (funct5 << 10) | (rs2 << 15) | (rs1 << 5) | rd;
}

int encode_itype(const string& mnemonic, int rd, int rs1, int imm) {
    int opcode = 0b001;
    int funct5 = funct5_map[mnemonic];
    int imm14 = imm & 0x3FFF;
    return (opcode << 29) | (imm14 << 15) | (funct5 << 10) | (rs1 << 5) | rd;
}

int encode_mem(const string& mnemonic, int rd_or_rs2, int rs1) {
    int opcode = 0b100;
    int funct3 = (mnemonic == "lw") ? 0b000 :
                 (mnemonic == "sw") ? 0b001 : 0;
    if (funct3 == 0b000) // LOAD
        return (opcode << 29) | (funct3 << 25) | (rs1 << 10) | (rd_or_rs2 << 5);
    else // STORE
        return (opcode << 29) | (funct3 << 25) | (rd_or_rs2 << 20) | (rs1 << 10);
}

int encode_streamload(const string& mnemonic, int vreg, int rs1, int rs2) {
    int opcode = 0b100;
    int funct3 = (mnemonic == "STREAMLOAD") ? 0b010 : 0b011;
    return (opcode << 29) | (funct3 << 25) | (vreg << 20) | (rs1 << 15) | (rs2 << 10);
}

int encode_xtype(const string& mnemonic, int ptr1, int ptr2) {
    int opcode = 0b101;
    int funct2 = funct2_x[mnemonic];
    return (opcode << 29) | (funct2 << 27) | (ptr1 << 15) | (ptr2 << 3);
}

int parse_register(const string& reg) {
    string trimmed = regex_replace(reg, regex(R"([\s,])"), "");
    if (reg_map.count(trimmed)) return reg_map[trimmed];
    if (trimmed[0] == 'x') return stoi(trimmed.substr(1));
    if (trimmed[0] == 'v') return stoi(trimmed.substr(1)); // for vector registers
    throw invalid_argument("Unknown register: " + reg);
}

void assemble_line(const string& line) {
    istringstream iss(line);
    string mnemonic;
    iss >> mnemonic;

    if (mnemonic.empty()) return;

    try {
        if (mnemonic == "NOP") {
            int instr = (0b000 << 29) | (0x1F << 10);
            cout << hex << setw(8) << setfill('0') << instr << " // " << line << endl;
            return;
        } else if (funct5_map.count(mnemonic)) {
            string rd_str, rs1_str, rs2_or_imm_str;
            getline(iss, rd_str, ',');
            getline(iss, rs1_str, ',');
            getline(iss, rs2_or_imm_str);

            rd_str = regex_replace(rd_str, regex(R"(\s)"), "");
            rs1_str = regex_replace(rs1_str, regex(R"(\s)"), "");
            rs2_or_imm_str = regex_replace(rs2_or_imm_str, regex(R"(\s)"), "");

            int rd = parse_register(rd_str);
            int rs1 = parse_register(rs1_str);
            int instr;

            if (rs2_or_imm_str.find('x') == string::npos && rs2_or_imm_str.find('v') == string::npos) {
                instr = encode_itype(mnemonic, rd, rs1, stoi(rs2_or_imm_str));
            } else {
                int rs2 = parse_register(rs2_or_imm_str);
                instr = encode_rtype(mnemonic, rd, rs1, rs2);
            }

            cout << hex << setw(8) << setfill('0') << instr << " // " << line << endl;

        } else if (mnemonic == "lw" || mnemonic == "sw") {
            string reg_str, addr_str;
            getline(iss, reg_str, ',');
            getline(iss, addr_str, ')');
            size_t lparen = addr_str.find('(');
            string offset = addr_str.substr(0, lparen);
            string base = addr_str.substr(lparen + 1);

            reg_str = regex_replace(reg_str, regex(R"(\s)"), "");
            base = regex_replace(base, regex(R"(\s)"), "");

            int reg = parse_register(reg_str);
            int base_reg = parse_register(base);

            cout << hex << setw(8) << setfill('0') << encode_mem(mnemonic, reg, base_reg) << " // " << line << endl;

        } else if (mnemonic == "STREAMLOAD" || mnemonic == "STOREVEC") {
            string vreg_str, r1_str, r2_str;
            getline(iss, vreg_str, ',');
            getline(iss, r1_str, ',');
            getline(iss, r2_str);

            vreg_str = regex_replace(vreg_str, regex(R"(\s)"), "");
            r1_str = regex_replace(r1_str, regex(R"(\s)"), "");
            r2_str = regex_replace(r2_str, regex(R"(\s)"), "");

            int vreg = parse_register(vreg_str);
            int rs1 = parse_register(r1_str);
            int rs2 = parse_register(r2_str);

            cout << hex << setw(8) << setfill('0') << encode_streamload(mnemonic, vreg, rs1, rs2) << " // " << line << endl;

        } else if (funct2_x.count(mnemonic)) {
            string p1_str, p2_str;
            getline(iss, p1_str, ',');
            getline(iss, p2_str);

            p1_str = regex_replace(p1_str, regex(R"(\s)"), "");
            p2_str = regex_replace(p2_str, regex(R"(\s)"), "");

            int ptr1 = parse_register(p1_str);
            int ptr2 = parse_register(p2_str);

            cout << hex << setw(8) << setfill('0') << encode_xtype(mnemonic, ptr1, ptr2) << " // " << line << endl;

        } else {
            cerr << "Unknown mnemonic: " << mnemonic << endl;
        }
    } catch (const exception& e) {
        cerr << "Error parsing line: '" << line << "'\n  -> " << e.what() << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input.asm>" << endl;
        return 1;
    }

    ifstream infile(argv[1]);
    if (!infile.is_open()) {
        cerr << "Error: Cannot open file " << argv[1] << endl;
        return 1;
    }

    // Open output file in rtl folder (relative path from assembler folder)
    ofstream outfile("../rtl/instr_mem.mem");
    if (!outfile.is_open()) {
        cerr << "Error: Cannot open output file ../rtl/instr_mem.mem" << endl;
        return 1;
    }

    string line;

    while (getline(infile, line)) {
        if (line.empty() || line[0] == '#') continue;

        // reroute output print into mem file
        streambuf* coutbuf = cout.rdbuf();
        cout.rdbuf(outfile.rdbuf()); 
        assemble_line(line);

        cout.rdbuf(coutbuf); 
    }

    infile.close();
    outfile.close();

    return 0;
}