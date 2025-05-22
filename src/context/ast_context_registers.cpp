#include "../../include/context/ast_context.hpp"

namespace ast{
// Constructor
ContextRegister::ContextRegister(){
    register_file = {
        {0, Register("zero", Type::_INT, false)},
        {1, Register("ra", Type::_INT, false)},
        {2, Register("sp", Type::_INT, false)},
        {3, Register("gp", Type::_INT, false)},
        {4, Register("tp", Type::_INT, false)},
        {5, Register("t0", Type::_INT, true)},
        {6, Register("t1", Type::_INT, true)},
        {7, Register("t2", Type::_INT, true)},
        {8, Register("s0", Type::_INT, false)},
        {9, Register("s1", Type::_INT, false)},
        {10, Register("a0", Type::_INT, false)},
        {11, Register("a1", Type::_INT, false)},
        {12, Register("a2", Type::_INT, false)},
        {13, Register("a3", Type::_INT, false)},
        {14, Register("a4", Type::_INT, false)},
        {15, Register("a5", Type::_INT, false)},
        {16, Register("a6", Type::_INT, false)},
        {17, Register("a7", Type::_INT, false)},
        {18, Register("s2", Type::_INT, true)},
        {19, Register("s3", Type::_INT, true)},
        {20, Register("s4", Type::_INT, true)},
        {21, Register("s5", Type::_INT, true)},
        {22, Register("s6", Type::_INT, true)},
        {23, Register("s7", Type::_INT, true)},
        {24, Register("s8", Type::_INT, true)},
        {25, Register("s9", Type::_INT, true)},
        {26, Register("s10", Type::_INT, true)},
        {27, Register("s11", Type::_INT, true)},
        {28, Register("t3", Type::_INT, true)},
        {29, Register("t4", Type::_INT, true)},
        {30, Register("t5", Type::_INT, true)},
        {31, Register("t6", Type::_INT, true)},
        {32, Register("ft0", Type::_FLOAT, true)},
        {33, Register("ft1", Type::_FLOAT, true)},
        {34, Register("ft2", Type::_FLOAT, true)},
        {35, Register("ft3", Type::_FLOAT, true)},
        {36, Register("ft4", Type::_FLOAT, true)},
        {37, Register("ft5", Type::_FLOAT, true)},
        {38, Register("ft6", Type::_FLOAT, true)},
        {39, Register("ft7", Type::_FLOAT, true)},
        {40, Register("fs0", Type::_FLOAT, false)},
        {41, Register("fs1", Type::_FLOAT, false)},
        {42, Register("fa0", Type::_FLOAT, false)},
        {43, Register("fa1", Type::_FLOAT, false)},
        {44, Register("fa2", Type::_FLOAT, true)},
        {45, Register("fa3", Type::_FLOAT, true)},
        {46, Register("fa4", Type::_FLOAT, true)},
        {47, Register("fa5", Type::_FLOAT, true)},
        {48, Register("fa6", Type::_FLOAT, true)},
        {49, Register("fa7", Type::_FLOAT, true)},
        {50, Register("fs2", Type::_FLOAT, true)},
        {51, Register("fs3", Type::_FLOAT, true)},
        {52, Register("fs4", Type::_FLOAT, true)},
        {53, Register("fs5", Type::_FLOAT, true)},
        {54, Register("fs6", Type::_FLOAT, true)},
        {55, Register("fs7", Type::_FLOAT, true)},
        {56, Register("fs8", Type::_FLOAT, true)},
        {57, Register("fs9", Type::_FLOAT, true)},
        {58, Register("fs10", Type::_FLOAT, true)},
        {59, Register("fs11", Type::_FLOAT, true)},
        {60, Register("ft8", Type::_FLOAT, true)},
        {61, Register("ft9", Type::_FLOAT, true)},
        {62, Register("ft10", Type::_FLOAT, true)},
        {63, Register("ft11", Type::_FLOAT, true)}
    };

    // Mapping register to unique id reference for quick look-ups
    for (const auto& [key, reg] : register_file){
        register_name_to_int[reg.getName()] = key;
    }
}

// Destructor
ContextRegister::~ContextRegister() {}

// Get register method
std::string ContextRegister::get_register(Type type){
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
            throw std::runtime_error("ContextRegister::get_register: Invalid variable type");
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
void ContextRegister::deallocate_register(const std::string &reg_name){
    if(register_name_to_int.find(reg_name) != register_name_to_int.end()){
        int reg_num = register_name_to_int[reg_name];
        std::cout << "Deallocating register: " << reg_name << std::endl;
        register_file[reg_num].setAvailable(true);
    }
}

// Get register name from number
std::string ContextRegister::get_register_name(int reg_number) const {
    if(register_file.find(reg_number) != register_file.end()){
        return register_file.at(reg_number).getName();
    }
    throw std::runtime_error("Invalid register number!");
}

// Set register type
void ContextRegister::set_register_type(const std::string &reg_name, Type type){
    if (register_name_to_int.find(reg_name)!= register_name_to_int.end()){
        int reg_num = register_name_to_int[reg_name];
        register_file[reg_num].setType(type);
    }
}

int ContextRegister::get_register_id(const std::string& reg_name) const {
    auto it = register_name_to_int.find(reg_name);
    if (it != register_name_to_int.end()) {
        return it->second;
    }
    throw std::runtime_error("Register name not found: " + reg_name);
}


void ContextRegister::allocate_register(std::string reg_name, Type type)
{
    int reg = register_name_to_int[reg_name];
    register_file[reg].setAvailable(false);
    register_file[reg].setType(type);
}

void Context::add_reg_to_set(std::string reg_name){
    int reg = reg_manager.get_register_id(reg_name);
    allocated_registers.top().insert(reg);
}

void Context::remove_reg_from_set(std::string reg_name){
    int reg = reg_manager.get_register_id(reg_name);
    allocated_registers.top().erase(reg);
}

//For function calls

void Context::push_registers(std::ostream &stream)
{
    for (int reg : allocated_registers.top())
    {
        int offset = get_stack_offset();
        Type type = reg_manager.get_register_by_id(reg).getType();
        stream << store_instr(type) << " " << reg_manager.get_register_name(reg) << ", " << offset << "(s0)" << std::endl;
        allocated_register_offsets[reg] = offset;
        increase_stack_offset(types_size.at(type));

        reg_manager.get_register_by_id(reg).setAvailable(true);
    }
}

void Context::pop_registers(std::ostream &stream)
{
    for (int reg : allocated_registers.top())
    {
        Type type = reg_manager.get_register_by_id(reg).getType();
        stream << load_instr(type) << " " << reg_manager.get_register_name(reg) << ", " << allocated_register_offsets[reg] << "(s0)" << std::endl;
        increase_stack_offset(-types_size.at(type));
        allocated_register_offsets.erase(reg);

        reg_manager.get_register_by_id(reg).setAvailable(false);
        reg_manager.get_register_by_id(reg).setType(type);
    }
}

}//namespace ast
