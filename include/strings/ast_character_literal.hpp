#pragma once

#include "ast_node.hpp"
#include "../operations/ast_operand.hpp"

namespace ast{

class CharacterLiteral : public Operand
{
private:
    char character_;

public:
    CharacterLiteral(char character) : character_(character) {}

    Type GetType(Context &context) const override;
    bool isPointerOp(Context &context) const override;

    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
    void Print(std::ostream &stream) const override;
};

}//namespace ast