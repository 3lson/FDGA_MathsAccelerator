#include "../../include/strings/ast_character_literal.hpp"

namespace ast{

Type CharacterLiteral::GetType(Context &context) const
{
    (void)context;
    return Type::_CHAR;
}

bool CharacterLiteral::isPointerOp(Context &context) const
{
    (void)context;
    return false;
}

void CharacterLiteral::EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const
{
    (void)context;
    stream << "li " << dest_reg << ", " << int(character_) << std::endl;
}

void CharacterLiteral::Print(std::ostream &stream) const
{
    stream << character_;
}

} //namespace ast