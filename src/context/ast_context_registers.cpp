#include "../../include/context/ast_context.hpp"

namespace ast{

void ScalarRegisterFile::initialiseRegisters(){
    register_file = {
        {0, Register("zero", Type::_INT, false)},
        {1, Register("ra", Type::_INT, false)},
        {2, Register("sp", Type::_INT, false)},
        {3, Register("gp", Type::_INT, false)},
        {4, Register("tp", Type::_INT, false)},
        {5, Register("s0", Type::_INT, false)}, //stack header
        {6, Register("s1", Type::_INT, false)}, //execution mask 
        {7, Register("s2", Type::_INT, true)},
        {8, Register("s3", Type::_INT, true)}, //?? this was set false could be important
        {9, Register("s4", Type::_INT, true)},
        {10, Register("s5", Type::_INT, true)},
        {11, Register("s6", Type::_INT, true)},
        {12, Register("s7", Type::_INT, true)},
        {13, Register("s8", Type::_INT, true)},
        {14, Register("s9", Type::_INT, true)},
        {15, Register("s10", Type::_INT, true)},
        {16, Register("s11", Type::_INT, true)},
        {17, Register("s12", Type::_INT, true)},
        {18, Register("s13", Type::_INT, true)},
        {19, Register("s14", Type::_INT, true)},
        {20, Register("s15", Type::_INT, true)},
        {21, Register("s16", Type::_INT, true)},
        {22, Register("s17", Type::_INT, true)},
        {23, Register("s18", Type::_INT, true)},
        {24, Register("s19", Type::_INT, true)},
        {25, Register("s20", Type::_INT, true)},
        {26, Register("s21", Type::_INT, true)},
        {27, Register("s22", Type::_INT, true)},

        {28, Register("s23", Type::_INT, true)}, 
        {29, Register("s24", Type::_INT, true)}, // "reserved for multi-threading"
        {30, Register("s25", Type::_INT, false)}, // PCvalue 
        {31, Register("s26", Type::_INT, false)}, // warpId

        {32, Register("fs0", Type::_FLOAT, false)}, // zeroreg 
        {33, Register("fs1", Type::_FLOAT, true)},
        {34, Register("fs2", Type::_FLOAT, true)},
                                                   
        {35, Register("fs3", Type::_FLOAT, true)}, 
        {36, Register("fs4", Type::_FLOAT, true)}, 
        {37, Register("fs5", Type::_FLOAT, true)}, 
        {38, Register("fs6", Type::_FLOAT, true)}, 

        {39, Register("fs7", Type::_FLOAT, true)},
        {40, Register("fs8", Type::_FLOAT, true)},
        {41, Register("fs9", Type::_FLOAT, true)},
        {42, Register("fs10", Type::_FLOAT, true)},
        {43, Register("fs11", Type::_FLOAT, true)},
        {44, Register("fs12", Type::_FLOAT, true)},
        {45, Register("fs13", Type::_FLOAT, true)},
        {46, Register("fs14", Type::_FLOAT, true)},
        {47, Register("fs15", Type::_FLOAT, true)},
        {48, Register("fs16", Type::_FLOAT, true)},
        {49, Register("fs17", Type::_FLOAT, true)},
        {50, Register("fs18", Type::_FLOAT, true)},
        {51, Register("fs19", Type::_FLOAT, true)},
        {52, Register("fs20", Type::_FLOAT, true)},
        {53, Register("fs21", Type::_FLOAT, true)},
        {54, Register("fs22", Type::_FLOAT, true)},
        {55, Register("fs23", Type::_FLOAT, true)},
        {56, Register("fs24", Type::_FLOAT, true)},
        {57, Register("fs25", Type::_FLOAT, true)},
        {58, Register("fs26", Type::_FLOAT, true)},
        {59, Register("fs27", Type::_FLOAT, true)},
        {60, Register("fs28", Type::_FLOAT, true)},
        {61, Register("fs29", Type::_FLOAT, true)},
        {62, Register("fs30", Type::_FLOAT, true)},
        {63, Register("fs31", Type::_FLOAT, false)}
    };
}

void VectorRegisterFile::initialiseRegisters(){
    register_file = {
        {0, Register("zero", Type::_INT, false)},
        {1, Register("ra", Type::_INT, false)},
        {2, Register("sp", Type::_INT, false)}, //data stack pointer
        {3, Register("gp", Type::_INT, false)}, //thread offset
        {4, Register("tp", Type::_INT, false)}, 
        {5, Register("v0", Type::_INT, false)}, //stack header
        {6, Register("v1", Type::_INT, true)},  
        {7, Register("v2", Type::_INT, true)},
        {8, Register("v3", Type::_INT, true)}, 
        {9, Register("v4", Type::_INT, true)},
        {10, Register("v5", Type::_INT, true)},
        {11, Register("v6", Type::_INT, true)},
        {12, Register("v7", Type::_INT, true)},
        {13, Register("v8", Type::_INT, true)},
        {14, Register("v9", Type::_INT, true)},
        {15, Register("v10", Type::_INT, true)},
        {16, Register("v11", Type::_INT, true)},
        {17, Register("v12", Type::_INT, true)},
        {18, Register("v13", Type::_INT, true)},
        {19, Register("v14", Type::_INT, true)},
        {20, Register("v15", Type::_INT, true)},
        {21, Register("v16", Type::_INT, true)},
        {22, Register("v17", Type::_INT, true)},
        {23, Register("v18", Type::_INT, true)},
        {24, Register("v19", Type::_INT, true)},
        {25, Register("v20", Type::_INT, true)},
        {26, Register("v21", Type::_INT, true)},
        {27, Register("v22", Type::_INT, true)},
        {28, Register("v23", Type::_INT, true)}, 
        {29, Register("v24", Type::_INT, true)},
        {30, Register("v25", Type::_INT, true)}, 
        {31, Register("v26", Type::_INT, false)}, // global thread id

        {32, Register("fv0", Type::_FLOAT, false)}, // zeroreg 
        {33, Register("fv1", Type::_FLOAT, true)},
        {34, Register("fv2", Type::_FLOAT, true)},
        {35, Register("fv3", Type::_FLOAT, true)}, 
        {36, Register("fv4", Type::_FLOAT, true)}, 
        {37, Register("fv5", Type::_FLOAT, true)},
        {38, Register("fv6", Type::_FLOAT, true)},
        {39, Register("fv7", Type::_FLOAT, true)},
        {40, Register("fv8", Type::_FLOAT, true)},
        {41, Register("fv9", Type::_FLOAT, true)},
        {42, Register("fv10", Type::_FLOAT, true)},
        {43, Register("fv11", Type::_FLOAT, true)},
        {44, Register("fv12", Type::_FLOAT, true)},
        {45, Register("fv13", Type::_FLOAT, true)},
        {46, Register("fv14", Type::_FLOAT, true)},
        {47, Register("fv15", Type::_FLOAT, true)},
        {48, Register("fv16", Type::_FLOAT, true)},
        {49, Register("fv17", Type::_FLOAT, true)},
        {50, Register("fv18", Type::_FLOAT, true)},
        {51, Register("fv19", Type::_FLOAT, true)},
        {52, Register("fv20", Type::_FLOAT, true)},
        {53, Register("fv21", Type::_FLOAT, true)},
        {54, Register("fv22", Type::_FLOAT, true)},
        {55, Register("fv23", Type::_FLOAT, true)},
        {56, Register("fv24", Type::_FLOAT, true)},
        {57, Register("fv25", Type::_FLOAT, true)},
        {58, Register("fv26", Type::_FLOAT, true)},
        {59, Register("fv27", Type::_FLOAT, true)},
        {60, Register("fv28", Type::_FLOAT, true)},
        {61, Register("fv29", Type::_FLOAT, true)},
        {62, Register("fv30", Type::_FLOAT, true)},
        {63, Register("fv31", Type::_FLOAT, false)}
    };
}

// Get register method
std::string RegisterFile::get_register(Type type){
    int start_register_file = 0;
    int end_register_file = 0;

    switch(type){
        case Type::_INT:
        case Type::_CHAR:
        case Type::_SHORT:
        case Type::_UNSIGNED_INT:
        case Type::_LONG:
            start_register_file = 5;
            end_register_file = 31;
            break;
        case Type::_FLOAT:
        case Type::_DOUBLE:
            start_register_file = 32;
            end_register_file = 63;
            break;
        default:
            throw std::runtime_error("RegisterFile::get_register: Invalid variable type");
    }
    for (int i = start_register_file; i<=end_register_file; i++){
        if (register_file[i].isAvailable()){
            std::cout << "Allocating register: " << register_file[i].getName() << std::endl;
            allocate_register(register_file[i].getName(), type);
            return get_register_name(i);
        }
    }
    throw std::runtime_error("No available register found!");
}

// Deallocate Register
void RegisterFile::deallocate_register(const std::string &reg_name){
    if(register_name_to_int.find(reg_name) != register_name_to_int.end()){
        int reg_num = register_name_to_int[reg_name];
        std::cout << "Deallocating register: " << reg_name << std::endl;
        register_file[reg_num].setAvailable(true);
    }
}

// Get register name from number
std::string RegisterFile::get_register_name(int reg_number) const {
    if(register_file.find(reg_number) != register_file.end()){
        return register_file.at(reg_number).getName();
    }
    throw std::runtime_error("Invalid register number!");
}

// Set register type
void RegisterFile::set_register_type(const std::string &reg_name, Type type){
    if (register_name_to_int.find(reg_name)!= register_name_to_int.end()){
        int reg_num = register_name_to_int[reg_name];
        register_file[reg_num].setType(type);
    }
}

int RegisterFile::get_register_id(const std::string& reg_name) const {
    auto it = register_name_to_int.find(reg_name);
    if (it != register_name_to_int.end()) {
        return it->second;
    }
    throw std::runtime_error("Register name not found: " + reg_name);
}


void RegisterFile::allocate_register(std::string reg_name, Type type)
{
    int reg = register_name_to_int[reg_name];
    register_file[reg].setAvailable(false);
    register_file[reg].setType(type);
}

void Context::add_reg_to_set(std::string reg_name){
    int reg = reg_manager->get_register_id(reg_name);
    allocated_registers.top().insert(reg);
}

void Context::remove_reg_from_set(std::string reg_name){
    int reg = reg_manager->get_register_id(reg_name);
    allocated_registers.top().erase(reg);
}

//For function calls

void Context::push_registers(std::ostream &stream)
{
    for (int reg : allocated_registers.top())
    {
        int offset = get_stack_offset();
        Type type = reg_manager->get_register_by_id(reg).getType();
        stream << store_instr(type) << " " << reg_manager->get_register_name(reg) << ", " << offset << "(s0)" << std::endl;
        allocated_register_offsets[reg] = offset;
        increase_stack_offset(types_size.at(type));

        reg_manager->get_register_by_id(reg).setAvailable(true);
    }
}

void Context::pop_registers(std::ostream &stream)
{
    for (int reg : allocated_registers.top())
    {
        Type type = reg_manager->get_register_by_id(reg).getType();
        stream << load_instr(type) << " " << reg_manager->get_register_name(reg) << ", " << allocated_register_offsets[reg] << "(s0)" << std::endl;
        increase_stack_offset(-types_size.at(type));
        allocated_register_offsets.erase(reg);

        reg_manager->get_register_by_id(reg).setAvailable(false);
        reg_manager->get_register_by_id(reg).setType(type);
    }
}

}//namespace ast
