#include "../../include/custom/ast_builtin_operand.hpp"
#include "../../include/context/ast_context.hpp"
#include "../../include/context/ast_context_kernel.hpp"
#include <iostream>
#include <string>


namespace ast {

BuiltInOperand::BuiltInOperand(const std::string& name, int reg_id)
    : name_(name), reg_id_(reg_id) {}

Type BuiltInOperand::GetType(Context& context) const {
    (void)context;
    return Type::_INT;
}

void BuiltInOperand::EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const {
    (void)context;

    std::string reg_prefix;

    if(context.get_instruction_state() == Kernel::_SCALAR){
        reg_prefix = "s";
    }
    else{
        reg_prefix = "v";
    }


    std::string reg = reg_prefix + std::to_string(reg_id_);
    if (dest_reg != reg) {
        stream << asm_prefix.at(context.get_instruction_state()) <<"addi " << dest_reg << ", " << reg << ", 0" << std::endl;
    }
}

void BuiltInOperand::Print(std::ostream& stream) const {
    stream << name_;
}

bool BuiltInOperand::isPointerOp(Context& context) const {
    (void)context;
    return false;
}

}
