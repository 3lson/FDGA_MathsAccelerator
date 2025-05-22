#pragma once

#include "ast_node.hpp"
#include "../operations/ast_operand.hpp"


namespace ast{

class StringLiteral : public Operand
{
private:
    std::string value_;

public:
    StringLiteral(std::string *value)
    {
        value_ = *value;
        delete value;
    }

    Type GetType(Context &context) const override;
    bool isPointerOp(Context &context) const override;
    std::string GetString() const;

    void SaveValue(Context &context, Global &global) const;

    void EmitRISC(std::ostream &stream, Context &context, std::string dest_reg) const override;
    void Print(std::ostream &stream) const override;
};

}//namespace ast