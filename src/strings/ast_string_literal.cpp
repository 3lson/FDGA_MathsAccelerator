#include "../../include/strings/ast_string_literal.hpp"

namespace ast{

Type StringLiteral::GetType(Context &context) const
{
    (void)context;
    return Type::_INT;
}

bool StringLiteral::isPointerOp(Context &context) const
{
    (void)context;
    return true;
}

void StringLiteral::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const
{
    int index = context.define_string(value_);
    stream << "lui " << dest_reg << ", %hi(string_" << index << ")" << std::endl;
    stream << "addi " << dest_reg << ", " << dest_reg << ", %lo(string_" << index << ")" << std::endl;
}

void StringLiteral::Print(std::ostream &stream) const
{
    stream << value_;
}

std::string StringLiteral::GetString() const
{
    return value_;
}

void StringLiteral::SaveValue(Context &context, Global &global) const
{
    int index = context.define_string(value_);
    std::string label = "string_" + std::to_string(index);

    global.push_label(label);
}

}//namespace ast