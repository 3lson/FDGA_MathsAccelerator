#include "../../include/custom/ast_builtin_operand.hpp"
#include "../../include/context/ast_context.hpp"
#include <iostream>

namespace ast {

BuiltInOperand::BuiltInOperand(const std::string& name, int reg_id)
    : name_(name), reg_id_(reg_id) {}

Type BuiltInOperand::GetType(Context& context) const {
    (void)context;
    return Type::_INT;
}

void BuiltInOperand::EmitElsonV(std::ostream& stream, Context& context, std::string dest_reg) const {
    (void)context;
    std::string reg = "x" + std::to_string(reg_id_);
    if (dest_reg != reg) {
        stream << "addi " << dest_reg << ", " << reg << ", 0" << std::endl;
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
